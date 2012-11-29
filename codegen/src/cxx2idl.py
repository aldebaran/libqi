#! /usr/bin/env python


from xml.etree import ElementTree as etree
import sys
import argparse
import tempfile
import os
import subprocess
import shutil

def cxx_type_to_signature(t):
  return t

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
    result[cls] = (methods,) #force tuple will add signals, doc
  return result

def raw_to_idl(dstruct):
  root = etree.Element('IDL')
  print(dstruct)
  for cls in dstruct:
    e = etree.SubElement(root, 'class', name=cls)
    print("--\n")
    print(dstruct[cls])
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
  return result

def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument("--output-file","-o", help="output file (stdout)")
  parser.add_argument("--output-mode","-m", default="idl", choices=["txt", "idl"], help="output mode (stdout)")
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
  out.write(res)

main(sys.argv)
