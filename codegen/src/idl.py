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
    'dynamic': 'qi::GenericValue',
    'string' : 'std::string',
    'int64'  : 'qi::int64_t',
}

def idltype_to_cxxtype(t):
  """ Return the C++ type to use for idl type t
  """
  t = t.replace('{', 'std::map<').replace('}', ' >')
  t = t.replace('[', 'std::vector<').replace(']', ' >')
  for e in REV_MAP:
    t = t.replace(e, REV_MAP[e])
  return t

def parse_toplevel_comma(txt):
  """ Split given string on top-level commas (not within <>)
  """
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
  """ Split a C++ type into basic components.
  Extracts template parameters.
  A type is extracted as a string, or (template-name, template-args, trailing-stuff)
  args is a list containing strings or (template-name, template-args, trailing)
  """
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
  return results

def cxx_parsed_to_sig(p):
  """ Convert a C++ type parsed by cxx_type_parse into the corresponding
      IDL signature.
  """
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
  """ Convert the string representation of a C++ type into the corresponding
  IDL signature
  """
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
  """ Invoke doxygen on given source files or directories
  :param files: A list of file or directory to scan
  :result: the temporary directory where doxygen output is
  """
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
  """ Convert doxygen output to internal RAW representation
  """
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

    result[cls] = (methods, signals) #force tuple will add signals, doc
  return result

def raw_to_idl(dstruct):
  """ Convert RAW to IDL XML format
  """
  root = etree.Element('IDL')
  for cls in dstruct:
    e = etree.SubElement(root, 'class', name=cls)
    for method_name in dstruct[cls][0]:
      m = etree.SubElement(e, 'method', name=method_name)
      method_raw = dstruct[cls][0][method_name]
      etree.SubElement(m, 'return', type= method_raw[0])
      for a in method_raw[1]:
        etree.SubElement(m, 'argument', type=a)
    for signal in dstruct[cls][1]:
      s = etree.SubElement(e, 'signal', name=signal[0])
      for a in signal[1]:
        etree.SubElement(s, 'argument', type=a)
  return root

def raw_to_text(dstruct):
  """ Convert RAW to human-readable text
  """
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
  """ Convert IDL XML to internal RAW representation
  """
  result = dict()
  for cls in root.findall("class"):
    methods = dict()
    for m in cls.findall("method"):
      r = m.find("return").get("type")
      args = [a.get("type") for a in m.findall("argument")]
      methods[m.get('name')] = (r, args)
    signals = []
    for s in cls.findall("signal"):
      n = s.get('name')
      args = [a.get("type") for a in s.findall("argument")]
      signals.append((n, args))
    result[cls.get("name")] =  (methods, signals)
  return result

def raw_to_proxy(class_name, data, return_future):
  """ Generate C++ proxy code from RAW
  """
  skeleton = """
#ifndef @GARD@
#define @GARD@


#include <vector>
#include <string>
#include <map>

#include <qi/types.hpp>
#include <qitype/signal.hpp>
#include <qitype/genericobject.hpp>

static void signal_bridge(bool enable, qi::SignalBase::Link* link, qi::GenericObject* obj,
  qi::SignalBase* sig, const char* sigName);

class @className@Proxy
{
public:
  @className@Proxy(qi::ObjectPtr obj)
  : _obj(obj)
@constructorInitList@
  {
@constructor@
  }
  qi::ObjectPtr asObject() { return _obj;}
@publicDecl@
  private:
    qi::ObjectPtr _obj;
@privateDecl@
};

typedef boost::shared_ptr<@className@Proxy> @className@ProxyPtr;

static qi::GenericValuePtr signal_bounce(const std::vector<qi::GenericValuePtr>& args,
 qi::SignalBase* target)
{
  target->trigger(args);
}

static void signal_bridge(bool enable, qi::SignalBase::Link* link, qi::GenericObject* obj,
  qi::SignalBase* sig, const char* sigName)
{
  std::string signature = sigName + ("::" + sig->signature());
  if (enable)
    *link = obj->xConnect(signature, qi::SignalSubscriber(qi::makeDynamicGenericFunction(
      boost::bind(&signal_bounce, _1, sig))));
  else
    obj->disconnect(*link);
}

#endif //@GARD@
"""
  #generate methods
  (methods, signals) = (data[0], data[1])
  methodImpls = ""
  for method_name in methods:
    method = methods[method_name]
    iret = method[0]
    cret = idltype_to_cxxtype(iret)
    if (return_future):
      cret = 'qi::FutureSync<' + cret + ' >'
    iargs = method[1]
    cargs = map(idltype_to_cxxtype, iargs)
    argNames = ", ".join(map(lambda x: 'p' + str(x), range(len(cargs))))
    if argNames:
      argNames = ', ' + argNames # comma used in call
    typedArgs = map(lambda x: cargs[x] + ' p' + str(x), range(len(cargs)))
    typedArgs = ','.join(typedArgs)
    #NOTE: should we return the future?
    methodImpls += '  ' + cret + " " + method_name + "(" + typedArgs + ") {\n    "
    if (cret != "void" or return_future):
      methodImpls += "return "
    methodImpls += '_obj->call<' + cret + ' >' + '("' + method_name + '"' + argNames + ");\n  }\n"
  signalDecl = ""
  signalDecl2 = ""
  ctor = ""
  # Make  a Signal field for each signal, bridge it to backend in ctor
  for sig in signals:
    signalDecl += '  qi::Signal<void(' + ','.join(map(idltype_to_cxxtype, sig[1])) +')> ' + sig[0] + ';\n'
    signalDecl2 += '  qi::SignalBase::Link _link_' + sig[0] + ';\n'
    ctor += '  , {0}(boost::bind(&signal_bridge, _1, &_link_{0}, _obj.get(), &{0}, "{0}"))\n'.format(sig[0])
  result = skeleton
  replace = {
      'GARD': '_' + class_name.upper() + '_PROXY_HPP_',
      'className': class_name,
      'publicDecl': methodImpls + signalDecl,
      'privateDecl': signalDecl2,
      'constructor': '',
      'constructorInitList': ctor,
  }
  for k in replace:
    result = result.replace('@' + k + '@', replace[k])
  return result

def raw_to_cxx_typebuild(class_name, data, registerToFactory):
  """ Generate a c++ file that registers the class to type system.
  """
  template = """
#include <qitype/objecttypebuilder.hpp>
#include <qitype/objectfactory.hpp>

qi::ObjectTypeBuilder<TYPE> builder;

qi::ObjectPtr makeOne(const std::string&)
{
  return builder.object(new TYPE());
}

static int init()
{
ADVERTISE
  builder.registerType();
REGISTER
  return 0;
}
static int _init_ = init();
  """
  advertise = ""
  (methods, signals) = (data[0], data[1])
  for method_name in methods:
    m = methods[method_name]
    advertise += '  builder.advertiseMethod("{0}", &{1}::{0});\n'.format(method_name, class_name)
  for s in signals:
    advertise += '  builder.advertiseEvent("{0}", &{1}::{0});\n'.format(s[0], class_name)
  register = ''
  if registerToFactory:
    register = '  qi::registerObjectFactory("{}", &makeOne);'.format(class_name)
  return template.replace('TYPE', class_name).replace('ADVERTISE', advertise).replace('REGISTER', register)

def raw_to_cxx_service_skeleton(class_name, data):
  """ Produce skeleton of C++ implementation of the service.
  """
  result = "#include <qitype/signal.hpp>\n\n"
  result += "class %s\n{\npublic:\n" % class_name
  (methods, signals) = (data[0], data[1])
  for method_name in methods:
    method = methods[method_name]
    args = ','.join(map(idltype_to_cxxtype, method[1]))
    result += '  %s %s(%s);\n' % (
      idltype_to_cxxtype(method[0]),
      method_name,
      args
    )
  for signal in signals:
    result += '  qi::Signal<void(%s)> %s;\n' % (
      ','.join(map(idltype_to_cxxtype, signal[1])),
      signal[0]
    )
  result += '};\n\n'
  for method_name in methods:
    method = methods[method_name]
    args = method[1]
    for i in range(len(args)):
      args[i] = idltype_to_cxxtype(args[i]) + ' p' + str(i)
    args = ','.join(args)
    result += '%s %s::%s(%s)\n{\n  // Implementation of %s\n}\n' % (
      idltype_to_cxxtype(method[0]),
      class_name,
      method_name,
      args,
      method_name
    )
  return result

def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument("--output-file","-o", help="output file (stdout)")
  parser.add_argument("--output-mode","-m", default="idl", choices=["txt", "idl", "proxy", "proxyFuture", "cxxtype", "cxxtyperegister", "cxxskel", "cxxservice"], help="output mode (stdout)")
  parser.add_argument("input", nargs='+', help="input file(s)")
  pargs = parser.parse_args(args)
  pargs.input = pargs.input[1:]
  if len(pargs.input) == 1 and pargs.input[0][-3:] == 'idl':
    xml = etree.ElementTree(file=pargs.input[0]).getroot()
    raw = idl_to_raw(xml)
  else:
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
  elif pargs.output_mode == "cxxtype":
    res = raw_to_cxx_typebuild(raw.keys()[0], raw[raw.keys()[0]], False)
  elif pargs.output_mode == "cxxtyperegister":
    res = raw_to_cxx_typebuild(raw.keys()[0], raw[raw.keys()[0]], True)
  elif pargs.output_mode == "cxxskel":
    res = raw_to_cxx_service_skeleton(raw.keys()[0], raw[raw.keys()[0]])
  elif pargs.output_mode == "cxxservice":
    res = raw_to_cxx_service_skeleton(raw.keys()[0], raw[raw.keys()[0]])
    res += raw_to_cxx_typebuild(raw.keys()[0], raw[raw.keys()[0]], True)
  out.write(res)

main(sys.argv)
