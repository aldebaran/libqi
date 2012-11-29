#! /usr/bin/env python


from xml.etree import ElementTree as etree
import sys
import argparse
import tempfile
import os
import subprocess
import shutil
import re

TYPE_MAP = {
  'unsigned int': 'uint',
  'unsigned long': 'uint64',
  'unsigned short': 'ushort',
  'unsigned char': 'uchar',
  'long': 'int64',
  'char*': 'string',
  'qi::GenericValue': 'dynamic',
}

REV_MAP = {
    'uint' : 'unsigned int',
    'uint64' : 'unsigned long',
    'dynamic': 'qi::GenericValue'
}

def idltype_to_cxxtype(t):
  t = t.replace('{', 'std::map<').replace('}', ' >')
  t = t.replace('[', 'std::vector<').replace(']', ' >')
  for e in REV_MAP:
    t = t.replace(e, REV_MAP[e])
  return t

def parse_toplevel_comma(txt):
  components = []
  level = 0
  p = 0
  while p < len(txt):
    if txt[p] == '>':
      level = level - 1
    if txt[p] == '<':
      level = level + 1
    if txt[p] == ',' and level == 0:
      components.append(txt[0: p])
      txt = txt[p+1:]
      p = 0
    else:
      p = p+1
  components.append(txt)
  return components

def cxx_type_parse(txt):
  components = parse_toplevel_comma(txt)
  results = []
  for t in components:
    substart = t.find('<')
    if substart != -1:
      #find matching
      count=1
      p = substart + 1
      while p < len(t) and count:
        if t[p] in '>}]':
          count = count - 1
        if t[p] in '<{[':
          count = count + 1
        p = p+1
      if count:
        print ("Parse error in " + t)
      elem = t[substart+1:p-1]
      subres = cxx_type_parse(elem)
      after = t[p+1:]
      results.append((t[0:substart], subres, after))
    else:
      results.append(t)
  print(results)
  return results

def cxx_parsed_to_sig(p):
  if (type(p) == list):
    return ','.join(map(cxx_parsed_to_sig, p))
  if (type(p) == tuple):
    if re.search('vector$', p[0]):
      return p[0][0:-6] + "[" + cxx_parsed_to_sig(p[1]) + "]" + cxx_parsed_to_sig(p[2])
    elif re.search('map$', p[0]):
      return p[0][0:-3] + "{" + cxx_parsed_to_sig(p[1]) + "}" + cxx_parsed_to_sig(p[2])
    else: # unknown template
      return p[0] + "<" + p[1] + ">" + p[2]
  else:
    return p


def cxx_type_to_signature(t):
  # Drop const and ref.
  # Drop namespace std (assume any class named vector is...a vector)
  t = t.replace('const ', '').replace("&", '').replace('std::', '')
  # Drop all spaces that do not separate identifiers
  t = re.sub(r"\s([^a-zA-Z])", r"\1", t)
  t = re.sub(r"([^a-zA-Z])\s", r"\1", t)
  t = t.strip()
  #Known type conversion
  for e in TYPE_MAP:
    t = re.sub(e, TYPE_MAP[e], t)
  #Container handling
  #For correct result in presence of containers of containers,
  #we need to parse the type almost fully
  #Huge hack, we do not realy parse 'a,b' in template
  parsed = cxx_type_parse(t)
  sig = cxx_parsed_to_sig(parsed)
  return sig

def run_doxygen(files):
  tmp_dir = tempfile.mkdtemp()
  # Create Doxyfile in there
  doxyfile_path = os.path.join(tmp_dir, "Doxyfile")
  doxy = open(doxyfile_path, "w")
  doxy.write("GENERATE_XML=YES\n"
    + "GENERATE_HTML=NO\n"
    + "GENERATE_LATEX=NO\n"
    + "QUIET=YES\n"
    + "WARN_IF_UNDOCUMENTED   = NO\n"
    + "INPUT= " + " ".join(files) + "\n"
    + "OUTPUT_DIRECTORY= " + tmp_dir + "\n")
  doxy.close()
  # Invoke doxygen
  subprocess.call(["doxygen", doxyfile_path])
  return tmp_dir

def doxyxml_to_raw(doxy_dir):
  # Parse the index to get all class names (and their functions)
  index_tree = etree.parse(os.path.join(doxy_dir, "xml", "index.xml"))
  class_index = dict()
  result = dict()
  for cls in index_tree.findall(".//compound[@kind='class']"):
    class_index[cls.find("name").text] = (cls.get('refid'), [f.find("name").text for f in cls.findall("member[@kind='function']")])
  for cls in class_index:
    class_id = class_index[cls][0]
    ctree = etree.parse(os.path.join(doxy_dir, "xml", class_id + ".xml"))
    class_root = ctree.find(".//compounddef[@id='" + class_id + "']")
    methods = dict()
    # Parse methods
    for m in class_root.findall("sectiondef[@kind='public-func']/memberdef[@kind='function']"):
      method_name = m.find("name").text
      rettype_raw = m.find("type").text
      rettype = cxx_type_to_signature(rettype_raw)
      arg_nodes = m.findall("param")
      argstype_raw = []
      if arg_nodes is not None:
        argstype_raw = [a.find('type').text for a in arg_nodes]
      argstype = map(cxx_type_to_signature, argstype_raw)
      methods[method_name] = (rettype, argstype)
    signals = []
    # Parse signals
    for s in class_root.findall("sectiondef[@kind='public-attrib']/memberdef[@kind='variable']"):
      name = s.find("name").text
      t = s.find("type").text
      # Normalize spacing to ease matching below
      t = re.sub(r"\s([^a-zA-Z])", r"\1", t)
      t = re.sub(r"([^a-zA-Z])\s", r"\1", t)
      t = t.strip()
      match = re.match(r"(qi::)?Signal<[^(]+\((.*)\)>", t)
      if match:
        t = match.expand(r"\2")
        sig = cxx_type_to_signature(t)
        sig = parse_toplevel_comma(sig)
        signals.append((name, sig))
        print(sig)

    result[cls] = (methods, signals) #force tuple will add signals, doc
  return result

def raw_to_idl(dstruct):
  root = etree.Element('IDL')
  print(dstruct)
  for cls in dstruct:
    e = etree.SubElement(root, 'class', name=cls)
    for method_name in dstruct[cls][0]:
      m = etree.SubElement(e, 'method', name=method_name)
      method_raw = dstruct[cls][0][method_name]
      etree.SubElement(m, 'return', type= method_raw[0])
      for a in method_raw[1]:
        etree.SubElement(m, 'argument', type=a)
  return root

def raw_to_text(dstruct):
  result = ""
  for cls in dstruct:
    result += "class " + cls +"\n  methods\n"
    for method_name in dstruct[cls][0]:
      method_raw = dstruct[cls][0][method_name]
      result += "    " + method_raw[0] + " " + method_name +"(" + ",".join(method_raw[1]) + ")\n"
    result += "  signals\n"
    for signal in dstruct[cls][1]:
      result += "    " + signal[0] + '(' + ','.join(signal[1]) + ')\n'
  return result

def idl_to_raw(root):
  result = dict()
  for cls in root.findall("class"):
    methods = dict()
    for m in cls.findall("method"):
      r = m.find("return").get("type")
      args = [a.get("type") for a in m.find("argument")]
      methods[m.get(name)] = (r, args)
    result[cls.get("name")] =  (methods,)
  return result

def raw_to_proxy(class_name, data, return_future):
  skeleton = """
#ifndef @GARD@
#define @GARD@


#include <vector>
#include <string>
#include <map>

#include <qitype/genericobject.hpp>
class @className@Proxy
{
public:
  @className@Proxy(qi::ObjectPtr obj)
  : _obj(obj)
  {}
  qi::ObjectPtr asObject() { return _obj;}
@methods@
  private:
    qi::ObjectPtr _obj;
};

typedef boost::shared_ptr<@className@Proxy> @className@ProxyPtr;
#endif //@GARD@
"""
  #generate methods
  methods = data[0]
  methodImpls = ""
  for method_name in methods:
    method = methods[method_name]
    iret = method[0]
    cret = idltype_to_cxxtype(iret)
    if (return_future):
      cret = 'qi::FutureSync<' + cret + ' >'
    iargs = method[1]
    cargs = map(idltype_to_cxxtype, iargs)
    argNames = 'p' + ", p".join(map(str, range(len(cargs))))
    typedArgs = map(lambda x: cargs[x] + ' ' + argNames[x], range(len(cargs)))
    typedArgs = ''.join(typedArgs)
    #NOTE: should we return the future?
    methodImpls += '  ' + cret + " " + method_name + "(" + typedArgs + ") {\n    "
    if (cret != "void" or return_future):
      methodImpls += "return "
    methodImpls += '_obj->call<' + cret + ' >' + '("' + method_name + '", ' + argNames + ");\n  }\n"
  result = skeleton
  replace = {
      'GARD': '_' + class_name.upper() + '_PROXY_HPP_',
      'className': class_name,
      'methods': methodImpls
  }
  for k in replace:
    result = result.replace('@' + k + '@', replace[k])
  return result

def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument("--output-file","-o", help="output file (stdout)")
  parser.add_argument("--output-mode","-m", default="idl", choices=["txt", "idl", "proxy", "proxyFuture"], help="output mode (stdout)")
  parser.add_argument("input", nargs='+', help="input file(s)")
  pargs = parser.parse_args(args)
  doxy_dir = run_doxygen(pargs.input)
  raw = doxyxml_to_raw(doxy_dir)
  shutil.rmtree(doxy_dir)
  out = sys.stdout
  if pargs.output_file and pargs.output_file != "-" :
    out = open(pargs.output_file, "w")
  if pargs.output_mode == "txt":
    res = raw_to_text(raw)
  elif pargs.output_mode == "idl":
    res = etree.tostring(raw_to_idl(raw))
  elif pargs.output_mode == "proxy":
    res = raw_to_proxy(raw.keys()[0], raw[raw.keys()[0]], False)
  elif pargs.output_mode == "proxyFuture":
    res = raw_to_proxy(raw.keys()[0], raw[raw.keys()[0]], True)
  out.write(res)

main(sys.argv)
