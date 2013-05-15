#! /usr/bin/env python

## Copyright (c) 2012 Aldebaran Robotics. All rights reserved.

""" Code parsing and generator tool.

    Representations used:
    - IDL: XML file describing the interface
    - RAW: Internal representation of the IDL
    raw: (methods, signals, properties, annotations)
    methods: [method]
    signals: [signal]
    properties: [property]
    method: (name, [argtype], rettype, annotations)
    signal: (name, [argtype], annotations)
    property: [name, type, annotations]

    Code parser:
    - Invoke Doxygen and parse its XML output to produce an IDL file.

    Code generators from RAW to C++:
    - interface: An interface that service must implement.
    - proxy: Specialized proxy: implement a modified version of interface with callType.
    - service skeleton
    - service bouncer: Implementation of the interface that bounces to an
      existing class
    - service and type registration: Code that fills an ObjectTypeBuilder and
      registers it.

     All instances are handled through a shared ptr. Typedefs are
     generated with 'Ptr' suffix.
     Naming: for a user service Foo not inheriting from any interface
     - IFoo : interface
     - FooService: implementation of IFoo bouncing to Foo
     - FooProxy: proxy
"""

from xml.etree import ElementTree as etree
import sys
import argparse
import tempfile
import os
import subprocess
import shutil
import re

METHODS = 0
SIGNALS = 1
PROPERTIES = 2


""" IDL type system:
    PRIMITIVES:
      - u?(char, short, long, int)
      - string
      - dynamic
    CONTAINERS:
      - [value_type] : list
      - {key_type, value_type}: map
"""
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
    'pair'   : 'std::pair'
}

# signature to IDL type
SIGNATURE_MAP = {
    'c'    : 'char',
    'C'    : 'uchar',
    'w'    : 'short',
    'W'    : 'ushort',
    'i'    : 'int',
    'I'    : 'uint',
    'l'    : 'long',
    'L'    : 'ulong',
    'f'    : 'float',
    'd'    : 'double',
    's'    : 'string',
    'm'    : 'dynamic',
    'v'    : 'void',
    'b'    : 'bool',
    'X'    : 'dynamic'
}

# signature of a tuple to known matching structure
KNOWN_STRUCT_MAP = {
}

def idltype_to_cxxtype(t):
  """ Return the C++ type to use for idl type t
  """
  # FIXME we are working at text level, this sucks
  if t in KNOWN_STRUCT_MAP:
    return KNOWN_STRUCT_MAP[t]
  t = t.replace('{', 'std::map<').replace('}', ' >')
  t = t.replace('[', 'std::vector<').replace(']', ' >')
  #t = t.replace('(', 'boost::mpl::vector<').replace(')', '>')
  for e in REV_MAP:
    # Do not replace partial symbols!
    t = re.sub("(^|[^a-zA-Z])"+e+"([^a-zA-Z]|$)", "\\1" + REV_MAP[e] + "\\2", t)
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
      return p[0] + "<" + cxx_parsed_to_sig(p[1]) + ">" + p[2]
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

ANNOTATIONS = ['fast', 'threadSafe']
def run_doxygen(files):
  """ Invoke doxygen on given source files or directories
  :param files: A list of file or directory to scan
  :result: the temporary directory where doxygen output is
  """
  tmp_dir = tempfile.mkdtemp()
  # Create Doxyfile in there
  doxyfile_path = os.path.join(tmp_dir, "Doxyfile")
  doxy = open(doxyfile_path, "w")
  doxy.write("""
GENERATE_XML=YES
GENERATE_HTML=NO
GENERATE_LATEX=NO
QUIET=YES
WARN_IF_UNDOCUMENTED   = NO
""" +
    "INPUT= " + " ".join(files) + "\n" +
    "OUTPUT_DIRECTORY= " + tmp_dir + "\n"
    )
  for a in ANNOTATIONS:
    doxy.write('ALIASES += %s=___%s___\n' % (a, a))
  doxy.close()
  # Invoke doxygen
  subprocess.call(["doxygen", doxyfile_path])
  return tmp_dir

def xml_extract_text(node):
  """ Return all text content from node and its children
  """
  result = ''
  if node.text:
    result = node.text
  for i in node:
    result += xml_extract_text(i)
  if node.tail:  # can be none
    result += node.tail
  return result.replace("\n", "").strip()

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
    methods = []
    # parse annotations
    rawAn = etree.tostring(class_root.find("briefdescription"), 'us-ascii', 'text')
    rawAn += etree.tostring(class_root.find("detaileddescription"), 'us-ascii', 'text')
    class_annotations = []
    for a in ANNOTATIONS:
      if '___' + a + '___' in rawAn:
        class_annotations.append(a)
    # Parse methods
    for m in class_root.findall("sectiondef[@kind='public-func']/memberdef[@kind='function']"):
      method_name = m.find("name").text
      rettype_raw = xml_extract_text(m.find("type"))
      if not rettype_raw:
        continue # constructor
      rettype = cxx_type_to_signature(rettype_raw)
      arg_nodes = m.findall("param")
      argstype_raw = []
      if arg_nodes is not None:
        argstype_raw = [xml_extract_text(a.find('type')) for a in arg_nodes]
      argstype = map(cxx_type_to_signature, argstype_raw)
      # Look for annotation
      raw_an = etree.tostring(m.find("briefdescription"), 'us-ascii', 'text')
      raw_an += etree.tostring(m.find("detaileddescription"), 'us-ascii', 'text')
      an = []
      for a in ANNOTATIONS:
        if '___' + a + '___' in raw_an:
          an.append(a)
      methods.append((method_name, argstype, rettype, ' '.join(an)))
    signals = []
    properties = []
    # Parse signals and properties
    for s in class_root.findall("sectiondef[@kind='public-attrib']/memberdef[@kind='variable']"):
      name = s.find("name").text
      t = xml_extract_text(s.find("type"))
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
      match = re.match(r"(qi::)?Property<(.*)>", t)
      if match:
        t = match.expand(r"\2")
        sig = cxx_type_to_signature(t)
        sig = parse_toplevel_comma(sig)[0]
        properties.append((name, sig))

    result[cls] = (methods, signals, properties, ' '.join(class_annotations))
  return result

def rawtype_to_boxinterface_argtype(arg):
  if arg=='string':
    return 3 #string
  elif arg in ['uint', 'int', 'ushort', 'short', 'int64', 'uint64']:
    return 2 #int
  else:
    return 0 #dynamic
def raw_to_boxinterface(class_name, data):
  """ Convert RAW to boxInterface choregraphe XML format
  """
  root = etree.Element('BoxInterface', name=class_name, tooltip=data[3])
  (methods, signals, properties, an) = data
  for method in methods:
    #advertise only nullary or unary methods
    (method_name, args, ret, an) = method
    if len(args) > 1:
      continue
    argtype = 1
    if len(args) == 1:
      argtype = rawtype_to_boxinterface_argtype(args[0])
    e = etree.SubElement(root, 'Input', name=method_name, type=str(argtype),
      type_size="0", nature="1", inner="0", tooltype=an)
  for signal in signals:
    (name, args, an) = signal
    if len(args) >1:
      continue
    argtype = 1
    if len(args) == 1:
      argtype = rawtype_to_boxinterface_argtype(args[0])
    e = etree.SubElement(root, 'Output', name=name, type=str(argtype),
      type_size="0", nature="2", inner="0", tooltip=an)
  for prop in properties:
    # prop for now are in signals, so were registered as output.
    # Register them as input
    e = etree.SubElement(root, 'Input', name=prop[0],
      type=str(rawtype_to_boxinterface_argtype(prop[1])),
      type_size="0",
      tooltip="")
  return etree.tostring(root) + "\n"

def raw_to_idl(dstruct):
  """ Convert RAW to IDL XML format
  """
  root = etree.Element('IDL')
  for cls in dstruct:

    (methods, signals, properties, an) = dstruct[cls]
    e = etree.SubElement(root, 'class', name=cls, annotations=','.join(an))
    for method in methods:
      (method_name, args, ret, an) = method
      m = etree.SubElement(e, 'method', name=method_name, annotations=','.join(an))
      etree.SubElement(m, 'return', type=ret)
      for a in args:
        etree.SubElement(m, 'argument', type=a)
    for signal in signals:
      s = etree.SubElement(e, 'signal', name=signal[0])
      for a in signal[1]:
        etree.SubElement(s, 'argument', type=a)
    for prop in properties:
      s = etree.SubElement(e, 'property', name=prop[0], type=prop[1])
  return root

def raw_to_text(dstruct):
  """ Convert RAW to human-readable text
  """
  result = ""
  for cls in dstruct:
    result += "class " + cls +"// " + dstruct[cls][3] + "\n  methods\n"
    for method in dstruct[cls][METHODS]:
      (method_name, args, ret, an) = method
      result += "    " + ret + " " + method_name +"(" + ",".join(args) + ") // " + an +"\n"
    result += "  signals\n"
    for signal in dstruct[cls][SIGNALS]:
      result += "    " + signal[0] + '(' + ','.join(signal[1]) + ')\n'
    result += "  properties\n"
    for prop in dstruct[cls][PROPERTIES]:
      result += "    " + prop[0] + '(' + prop[1] + ')\n'
  return result

def method_to_cxx(method):
  """ Take a method from RAW representation, and return
      (declarationret, declarationargs, args), for example
      ("int", "int p1, std::string p2", "p1, p2")
  """
  iret = method[2]
  cret = idltype_to_cxxtype(iret)
  iargs = method[1]
  cargs = map(idltype_to_cxxtype, iargs)
  typed_args = map(lambda x: cargs[x] + ' p' + str(x), range(len(cargs)))
  typed_args = ','.join(typed_args)
  arg_names = map(lambda x: 'p' + str(x), range(len(cargs)))
  arg_names = ','.join(arg_names)
  return (cret, typed_args, arg_names)

def idl_to_raw(root):
  """ Convert IDL XML to internal RAW representation
  """
  result = dict()
  for cls in root.findall("class"):
    methods = []
    for m in cls.findall("method"):
      r = m.find("return").get("type")
      args = [a.get("type") for a in m.findall("argument")]
      methods.append((m.get('name'), args, r, (m.get('annotations') or '')))
    signals = []
    for s in cls.findall("signal"):
      n = s.get('name')
      args = [a.get("type") for a in s.findall("argument")]
      signals.append((n, args))
    properties = []
    for p in cls.findall("property"):
      n = p.get('name')
      t = p.get('type')
      properties.append([n, t])
    result[cls.get("name")] =  (methods, signals, properties, (cls.get("annotations") or ''))
  return result

def raw_to_interface(class_name, data, include):
  """ Generate service interface class from RAW representation
  """
  skeleton = """
#ifndef @INAME@_INTERFACE_HPP
#define @INAME@_INTERFACE_HPP

#include <vector>
#include <string>
#include <map>

#include <qitype/signal.hpp>
#include <qitype/property.hpp>
@include@
class @INAME@
{
  public:
    @CTOR0DECL@
    @INAME@(@CTOR1ARGS@);
    virtual ~@INAME@();
@DECLS@

@GETTERS@
    bool _allocated; // are sigs/props allocated by us
};

// Register an object implementing this interface
#define QI_IMPLEMENT_@INAME@(name, ...) \
  QI_REGISTER_OBJECT(name, @FIELDS@, ##__VA_ARGS__)

@CTOR0IMPL@

inline @INAME@::@INAME@(@CTOR1ARGS@)
@CTOR1@
{}

inline @INAME@::~@INAME@()
{
  if (_allocated)
  {
@DELETE@
  }
}

typedef boost::shared_ptr<@INAME@> @INAME@Ptr;

QI_TYPE_NOT_CLONABLE(@INAME@);
#endif
"""
  (methods, signals, properties) = (data[METHODS], data[SIGNALS], data[PROPERTIES])
  methods_decl = ''
  getters = ''
  fields = []
  for method in methods:
    (cret, typed_args, arg_names) = method_to_cxx(method)
    method_name = method[0]
    methods_decl += '    virtual %s %s (%s) = 0;\n' % (cret, method_name, typed_args)
    fields.append(method_name)
  signals_decl = ''
  ctor_decl = []
  ctor_init = []   # all sig/prop in argument ctor
  ctor_init0 = []  # no argument ctor
  dtor = ''
  for sig in signals:
    name = sig[0]
    signature = ','.join(map(idltype_to_cxxtype, sig[1]))
    signals_decl += '    qi::Signal<void(%s)> & %s;\n' % (signature, name)
    ctor_decl.append('qi::Signal<void(%s)> & %s' % (signature, name))
    ctor_init.append('%s(%s)' % (name, name))
    ctor_init0.append('%s(*new qi::Signal<void (%s)>)' % (name, signature))
    dtor += '    delete &%s;\n' % (name)
    getters += '    qi::Signal<void(%s)> & _interface_%s() { return %s;}\n' % (signature, name, name)
    fields.append('_interface_' + name)
  for prop in properties:
    name = prop[0]
    signature = idltype_to_cxxtype(prop[1])
    signals_decl += '    qi::Property<%s> & %s;\n' % (signature, name)
    ctor_decl.append('qi::Property<%s> & %s' % (signature, name))
    ctor_init.append('%s(%s)' % (name, name))
    ctor_init0.append('%s(*new qi::Property<%s>)' % (name, signature))
    dtor += '    delete &%s;\n' % (name)
    getters += '    qi::Property<%s>& _interface_%s() { return %s;}\n' % (signature, name, name)
    fields.append('_interface_' + name)
  no_sigprop = (len(ctor_init) == 0)
  ctor_init.append('_allocated(false)')
  ctor_init0.append('_allocated(true)')
  ctor_decl = ','.join(ctor_decl)
  ctor_init =   ':    ' + '\n      ,'.join(ctor_init)
  ctor_init0 =  ':    ' + '\n      ,'.join(ctor_init0)
  if no_sigprop:
    ctor0impl = ''
    ctor0decl = ''
  else:
    getters = '   // Accessors required for proper bindings, since constructing a getter on a reference member is impossible\n' + getters
    ctor0impl = """
inline @INAME@::@INAME@()
%s
{}
""" % (ctor_init0)
    ctor0decl = '    %s();\n' % ('I'+class_name)
  replace = {
   '@CTOR0IMPL@': ctor0impl,
   '@CTOR0DECL@': ctor0decl,
   '@INAME@': 'I' + class_name,
   '@CTOR0@': ctor_init0,
   '@CTOR1@': ctor_init,
   '@CTOR1ARGS@': ctor_decl,
   '@DELETE@': dtor,
   '@DECLS@': methods_decl + signals_decl,
   '@GETTERS@': getters,
   '@FIELDS@': ','.join(fields),
   '@include@': ''.join(['#include <' + x + '>\n' for x in include])
   }
  for k in replace:
    skeleton = skeleton.replace(k, replace[k])
  skeleton = "/*" + raw_to_text({class_name: data}) + "*/" + skeleton
  return skeleton

def raw_to_proxy(class_name, data, return_future, implement_interface, include):
  """ Generate C++ proxy code from RAW
  @param return_future have the declared functions return a Future
  @param implement_interface make the proxy inherit from an interface, that
         can be generated with raw_to_interface
  """
  skeleton = """
#ifndef @GARD@
#define @GARD@


#include <vector>
#include <string>
#include <map>

#include <qi/types.hpp>
#include <qitype/signal.hpp>
#include <qitype/property.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/proxysignal.hpp>
#include <qitype/proxyproperty.hpp>

@include@

class @className@Proxy: public ::qi::Proxy
{
public:
  @className@Proxy(qi::ObjectPtr obj)
  : qi::Proxy(obj)
@constructor_initList@
  {
@constructor@
  }
   public:
@publicDecl@

@privateDecl@
};

QI_TYPE_PROXY(@className@Proxy);
QI_REGISTER_PROXY(@className@Proxy);

#endif //@GARD@
"""
  forward_decls = "class @className@Proxy;\ntypedef boost::shared_ptr<@className@Proxy> @className@ProxyPtr;\n"
  forward_decls = forward_decls.replace('@className@', class_name)
  #generate methods
  (methods, signals, properties) = (data[METHODS], data[SIGNALS], data[PROPERTIES])
  method_impls = ""
  fwdecl = dict()
  for method in methods:
    (cret, typed_args, arg_names) = method_to_cxx(method)
    method_name = method[0]
    if cret.find('Ptr') == len(cret)-3 and cret.find('Proxy') == -1:
      fwdecl[cret[0:len(cret)-3]] = 1
      cret = cret.replace('Ptr', 'ProxyPtr')
    out_ret = cret
    if (return_future):
      out_ret = 'qi::FutureSync<' + cret + ' >'
    if arg_names:
      arg_names = ', ' + arg_names # comma used in call
    # Handle ObjectPtr in argument
    # If a function takes a FooProxyPtr, it actualy accepts AnyPtr with Any
    # a compatible object.
    # But we have no parent class to let C++ typecheck.
    # We could make this method a template, but let's rather take a
    # AutoGenericValuePtr.
    typed_args = ''
    argIdx = 0
    for arg in method[1]:
      if arg.find("Ptr") != -1:
        typed_args += '::qi::AutoGenericValuePtr p' + str(argIdx)
      else:
        typed_args += idltype_to_cxxtype(arg) + ' p' + str(argIdx)
      typed_args += ', '
      argIdx = argIdx + 1
    # Add a final optional argument 'MetaCallType calltype=auto'
    typed_args = typed_args + '::qi::MetaCallType callType = ::qi::MetaCallType_Auto'
    #NOTE: should we return the future?
    method_impls += '  ' + out_ret + " " + method_name + "(" + typed_args + ") {\n    "
    if (cret != "void" or return_future):
      method_impls += "return "
    method_impls += '_obj->call<' + cret + ' >' + '(callType,"' + method_name + '"' + arg_names + ");\n  }\n"
  signal_decl = ''
  ctor = ''
  # Make  a Signal field for each signal, bridge it to backend in ctor
  for sig in signals:
    signal_decl += '  qi::ProxySignal<void(' + ','.join(map(idltype_to_cxxtype, sig[1])) +')> ' + sig[0] + ';\n'
    ctor += '  , {0}(obj, "{0}")\n'.format(sig[0])
  for prop in properties:
    print(prop[1])
    signal_decl += '  qi::ProxyProperty<' + idltype_to_cxxtype(prop[1]) + '> ' + prop[0] + ';\n'
    ctor += '  , {0}(obj, "{0}")\n'.format(prop[0])
  result = skeleton
  replace = {
      'GARD': '_' + class_name.upper() + '_PROXY_HPP_',
      'className': class_name,
      'publicDecl': method_impls + signal_decl,
      'privateDecl': '',
      'constructor': '',
      'constructor_initList': ctor,
      'include': ''.join(['#include <' + x + '>\n' for x in include]),
  }
  for k in replace:
    result = result.replace('@' + k + '@', replace[k])
  for k in fwdecl.keys():
    forward_decls += 'class {0}; typedef boost::shared_ptr<{0}> {0}Ptr;\n'.format(k+'Proxy')
  return [forward_decls, result, '']

def raw_to_cxx_typebuild(class_name, data, use_interface, register_to_factory, include):
  """ Generate a c++ file that registers the class to type system.
  @param class_name name of the class to bind
  @param data raw IDL data
  @param use_interface true if the class inherits from generated interface
  @param register_to_factory: '', 'service' or 'factory'
  """
  template = """
#include <qitype/genericobject.hpp>
#include <qitype/objecttypebuilder.hpp>
#include <qitype/objectfactory.hpp>

@INCLUDE@
static int @TYPE@init()
{
 qi::ObjectTypeBuilder<@TYPE@> builder;
@ADVERTISE@
  builder.registerType();
  return 0;
}
static int _init_@TYPE@ = @TYPE@init();
@REGISTER@
"""

  if include:
    v = ''
    for i in include:
      v += '#include <' + i + '>\n'
    include = v
  else:
    include = ''
  advertise = ''
  (methods, signals, properties, annotations) = (data[0], data[1], data[2], data[3])
  if 'threadSafe' in annotations:
    advertise += '  builder.setThreadingModel(qi::ObjectThreadingModel_MultiThread);\n'

  for method in methods:
    method_name = method[0]
    annotations = method[3]
    thread_mode = 'qi::MetaCallType_Auto'
    if 'fast' in annotations:
      thread_mode = 'qi::MetaCallType_Fast'
    if 'threadSafe' in annotations:
      thread_mode = 'qi::MetaCallType_ThreadSafe'
    advertise += '  builder.advertiseMethod("{0}", &{1}::{0}, {2});\n'.format(method_name, class_name, thread_mode)
  for s in signals:
    name = s[0]
    field = name
    if use_interface:
      field = '_interface_' + name
    advertise += '  builder.advertise("%s", &%s::%s);\n' % (name, class_name, field);
  for s in properties:
    name = s[0]
    field = name
    if use_interface:
      field = '_interface_' + name
    advertise += '  builder.advertise("%s", &%s::%s);\n' % (name, class_name, field);
  register = ''
  if register_to_factory == 'service':
    register = 'QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR(%s);\n' % (class_name)
  elif register_to_factory == 'factory':
    register = 'QI_REGISTER_OBJECT_FACTORY_BUILDER(%s);\n' % (class_name)

  return template.replace('@TYPE@', class_name).replace('@ADVERTISE@', advertise).replace('@REGISTER@', register).replace('@INCLUDE@', include)

def raw_to_cxx_service_skeleton(class_name, data, implement_interface, include):
  """ Produce skeleton of C++ implementation of the service.
  """
  result = "#include <qitype/signal.hpp>\n#include <qitype/property.hpp>\n"
  result += ''.join(['#include <' + x + '>\n' for x in include])
  result += '\n'
  inherits = ''
  if implement_interface:
    inherits = ' : public I' + class_name
  result += "class %s %s \n{\npublic:\n" % (class_name, inherits)
  (methods, signals) = (data[0], data[1])
  for method in methods:
    method_name = method[0]
    args = ','.join(map(idltype_to_cxxtype, method[1]))
    result += '  %s %s(%s);\n' % (
      idltype_to_cxxtype(method[2]),
      method_name,
      args
    )
  iface_ctor = []
  for signal in signals:
    iface_ctor.append('%s' % (signal[0]))
    result += '  qi::Signal<void(%s)> %s;\n' % (
      ','.join(map(idltype_to_cxxtype, signal[1])),
      signal[0]
    )
  if implement_interface:
    result += '  %s() :%s(%s) {}\n' % (class_name, class_name, ','.join(iface_ctor))
  result += '};\n\n'
  for method in methods:
    method_name = method[0]
    args = method[1]
    for i in range(len(args)):
      args[i] = idltype_to_cxxtype(args[i]) + ' p' + str(i)
    args = ','.join(args)
    result += '%s %s::%s(%s)\n{\n  // Implementation of %s\n}\n' % (
      idltype_to_cxxtype(method[2]),
      class_name,
      method_name,
      args,
      method_name
    )
  return result

def raw_to_cxx_service_bouncer(class_name, data, impl_name, include):
  """ Produce implementation of \p class_name interface bouncing to class
      \p impl_name.
  """
  skeleton = """

@impl@ make_@impl@();
class @name@Service: public I@name@
{
  public:
    @name@Service(@impl@ impl = make_@impl@())
    : I@name@(@signal_init@)
    , _impl(impl)
    {}
@code@
  private:
  @impl@ _impl;
};
QI_TYPE_NOT_CLONABLE(@name@Service);

"""
  converter_impl = """
I@name@Ptr interface_from_bound(@impl@ ptr)
{
  return I@name@Ptr((I@name@*)new @name@Service(ptr));
}

@name@ProxyPtr proxy_from_interface(I@name@Ptr ptr)
{
  // Get GenericObject from pointer
  qi::ObjectPtr obj = I@name@builder.object(ptr.get(),
    boost::bind(&release_ptr<I@name@Ptr>, ptr));
  return @name@ProxyPtr(new @name@Proxy(obj));
}
"""
  converter_decl = """
#include <vector>
#include <string>
#include <map>

#include <qi/types.hpp>
#include <qitype/signal.hpp>

#ifndef QI_GEN_SERVICE_BOUNCER_INCLUDE_
#define QI_GEN_SERVICE_BOUNCER_INCLUDE_
@include@

template<typename T>
inline void release_ptr(T ptr)
{
}
#endif

I@name@Ptr interface_from_bound(@impl@ ptr);
@name@ProxyPtr proxy_from_interface(I@name@Ptr ptr);
"""
  (methods, signals, properties) = (data[METHODS], data[SIGNALS], data[PROPERTIES])
  signal_init = []
  for s in signals:
    signal_init.append('impl->' + s[0])
  for p in properties:
    signal_init.append('impl->' + p[0])
  method_bounce = ''
  emit_interface = dict()
  for method in methods:
    (ret, typed_args, args) = method_to_cxx(method)
    method_name = method[0]
    # UGLY HACK to detect if the method returns an other class for which we
    # have a bouncer.
    # We need a double bounce:
    # User impl returns a FooPtr which does not implement IFoo
    # our sinature does not return a IFooPTR, but a FooProxyPtr
    # so:
    # FooPtr foo;
    # IFooPtr ifoo = interface_from_bound(foo); // generated by binder(us)
    # FooProxyPtr foop = proxy_from_interface(ifoo); // generated by proxy
    if method[2] in REV_MAP and method[2].find("Ptr") != -1: # FIXME make a more clever test
      #forward-declare converter functions we use
      fwd = "I@name@Ptr interface_from_bound(@name@Ptr ptr);\n@name@ProxyPtr proxy_from_interface(I@name@Ptr ptr);\n"
      fwd = fwd.replace('@name@', method[2].replace('Ptr', ''))
      converter_decl += fwd 
      emit_interface[method[2]] = 1
      method_bounce += '   %s %s(%s) { return qi_to_interface_%s(_impl->%s(%s));}\n' % (ret, method_name, typed_args, method[2], method_name, args)
    elif ret[0:11] == 'std::vector' and ret[12:-2] == 'I' + method[2][1:-1]:
      # Ugly vector detector case, not forgetting the inserted space
      method_bounce += '   %s %s(%s) { return qi_to_interface_v(_impl->%s(%s));}\n' % (ret, method_name, typed_args, method_name, args)
    else:
      method_bounce += '   %s %s(%s) { return _impl->%s(%s);}\n' % (ret, method_name, typed_args, method_name, args)
  include = ''.join(['#include <' + x + '>\n' for x in include])
  for name in emit_interface:
    proxy_name = name.replace("Ptr", "ProxyPtr")
    method_bounce = """
    static std::vector<%s> qi_to_interface_v(std::vector<%s> ptr) {
      std::vector<%s> res;
      res.resize(ptr.size());
      std::transform(ptr.begin(), ptr.end(), res.begin(), qi_to_interface_%s);
      return res;
   }
""" % (proxy_name, name, proxy_name, name) + method_bounce
    method_bounce = '   static %s qi_to_interface_%s(%s ptr) {return proxy_from_interface(interface_from_bound(ptr));}\n' % (
      proxy_name, name, name) + method_bounce
  res = skeleton.replace(
    '@name@', class_name).replace(
    '@impl@', impl_name.replace('@', class_name)).replace(
    '@code@', method_bounce).replace(
    '@signal_init@', ','.join(signal_init))
  converter_decl = converter_decl.replace('@name@', class_name).replace('@impl@', impl_name.replace('@', class_name)).replace(
    '@include@', include)
  converter_impl = converter_impl.replace('@name@', class_name).replace('@impl@', impl_name.replace('@', class_name))
  return [converter_decl, res, converter_impl]

def signature_to_idl(sig):
  # Add comma separator between tuple elements
  while True:
      next = re.sub('([a-zA-Z\]\}\)])([a-zA-Z\(\{\[])', "\\1,\\2", sig)
      if next == sig:
        break
      sig = next
  # Then convert each known element (one char) to the corresponding idl type
  tmp = ''
  for c in sig:
    if c in SIGNATURE_MAP:
      tmp += SIGNATURE_MAP[c]
    else:
      tmp += c
  return tmp

def signature_split(sig):
  ret = []
  enter = '({['
  leave = ')}]'
  p = 0
  plast = 0
  while p < len(sig):
    if sig[p] not in enter:
      ret.append(sig[p])
      p = p+1
      continue
    expect = leave[enter.find(sig[p])]
    plast = p
    while p < len(sig) and sig[p] != expect:
      p = p+1
    ret.append(sig[plast:p+1])
    p = p+1
  print("woot %s %s" % (sig, ret))
  return ret

def runtime_to_raw(class_name, sd_url):
  from qi import Session
  print("connecting session to " + sd_url)
  session = Session()
  session.connect(sd_url)
  print("Trying to fetch " + class_name)
  obj = session.service(class_name)
  print("Fetching metaobject")
  desc = obj.metaObject()
  print(desc)
  methods = []
  for k in desc[0]:
    m = desc[0][k]
    print(m) # ex: (0L, 'L', 'registerEvent', '(IIL)', 'doc', [argdoc], 'retdoc')
    method_name = m[2]
    sig = m[3][1:-1] #remove toplevel tuple
    sig = signature_split(sig)
    sig = map(signature_to_idl, sig)
    rettype = m[1]
    rettype = signature_to_idl(rettype)
    if method_name == 'metaObject': # HACK
      rettype = 'MetaObject'
    doc = m[4]
    # FIXME support argdoc/retdoc in RAW structure
    for argdoc in m[5]:
      doc += '\n' + argdoc[0] + ': ' + argdoc[1]
    if len(m[6]):
      doc += '\nreturn: ' + m[6]
    methods.append((method_name, sig, rettype, doc))
  return {class_name : (methods, [], [], '')}

def main(args):
  res = ''
  parser = argparse.ArgumentParser()
  parser.add_argument("--interface", "-i", help="Use interface mode", action='store_true')
  parser.add_argument("--output-file","-o", help="output file (stdout)")
  parser.add_argument("--output-mode","-m", default="txt", choices=["parse", "txt", "idl", "proxy", "proxyFuture", "cxxtype", "cxxtyperegisterfactory", "cxxtyperegisterservice", "cxxskel", "cxxservice", "cxxserviceregister", "cxxservicebouncer", "cxxservicebouncerregister", "interface", "boxinterface", "many"], help="output mode (stdout)")
  parser.add_argument("--include", "-I", default="", help="File to include in generated C++")
  parser.add_argument("--known-classes", "-k", default="", help="Comma-separated list of other handled classes")
  parser.add_argument("--classes", "-c", default="*", help="Comma-separated list of classes to select, optionally with per class ':operation'")
  parser.add_argument("input", nargs='+', help="input file(s)")

  pargs = parser.parse_args(args)
  pargs.input = pargs.input[1:]

  # Fill KNOWN_STRUCT_MAP with static stuff
  KNOWN_STRUCT_MAP[signature_to_idl('({I(Isss[(ss)]s)}{I(Is)}s)')] = 'qi::MetaObject'

  for c in pargs.known_classes.split(','):
    c = c.strip()
    if len(c):
      REV_MAP[c + 'Ptr'] = c + 'ProxyPtr'
  # Step one: get raw from either IDL, source files, or running service
  if len(pargs.input) == 1 and pargs.input[0][-3:] in ['idl', 'xml']:
    xml = etree.ElementTree(file=pargs.input[0]).getroot()
    raw = idl_to_raw(xml)
  elif len(pargs.input) == 1 and pargs.input[0].find('://') != -1:
    service = pargs.input[0].split('/')[-1]
    url = '/'.join(pargs.input[0].split('/')[0:-1])
    raw = runtime_to_raw(service, url)
  else:
    doxy_dir = run_doxygen(pargs.input)
    raw = doxyxml_to_raw(doxy_dir)
    #print("DOXYDIR " + doxy_dir)
    shutil.rmtree(doxy_dir)

  if not len(pargs.include):
    pargs.include = []
  else:
    pargs.include = filter(lambda x: len(x), pargs.include.split(','))

  # Filter out classes present in raw, fill class_operation
  class_operation = dict()
  if pargs.classes != '*':
    classes = pargs.classes.split(',')
    newraw = dict()
    for c in classes:
      if not c.strip():
        continue #be lenient on trailing ,
      cc = c.split(':', 1)
      if not cc[0] in raw:
        raise Exception("Requested class %s not found in %s" % (cc[0], ','.join(raw.keys())))
      REV_MAP[cc[0] + 'Ptr'] = cc[0] + 'ProxyPtr'
      newraw[cc[0]] = raw[cc[0]]
      if len(cc) > 1:
        class_operation[cc[0]] = cc[1]
    raw = newraw
  split_output = (pargs.output_file.find("%s") != -1)
  # Main switch on output mode
  if pargs.output_mode == "txt":
    res = raw_to_text(raw)
  elif pargs.output_mode == "idl":
    res = etree.tostring(raw_to_idl(raw))
  else: # Need to apply per-class function
    res = ['','','']
    for c in raw:
      op = pargs.output_mode
      name = c
      if c in class_operation:
        op = class_operation[c].split(':')
        if len(op) > 1:
          name = op[1]
        op = op[0]
      functions = []
      args = []
      if op == "interface":
        functions = [raw_to_interface]
        args = [[pargs.include]]
      elif op == "boxinterface":
        functions = [raw_to_boxinterface]
        args = [[]]
      elif op == "proxy":
        functions = [raw_to_proxy]
        args = [[False, pargs.interface, pargs.include]]
      elif op == "proxyFuture":
        functions = [raw_to_proxy]
        args = [[True, pargs.interface, pargs.include]]
      elif op == "cxxtype":
        functions = [raw_to_cxx_typebuild]
        args = [[pargs.interface, '', pargs.include]]
      elif op == "cxxtyperegisterfactory":
        functions = [raw_to_cxx_typebuild]
        args = [[pargs.interface, 'factory', pargs.include]]
      elif op == "cxxtyperegisterservice":
        functions = [raw_to_cxx_typebuild]
        args = [[pargs.interface, 'service', pargs.include]]
      elif op == "cxxskel":
        functions = [raw_to_cxx_service_skeleton]
        args = [[pargs.interface, pargs.include]]
      elif op == "cxxserviceregister":
        functions = [raw_to_cxx_service_skeleton, raw_to_cxx_typebuild]
        args = [[pargs.interface, pargs.include], [pargs.interface, 'service', pargs.include]]
      elif op == "cxxservice":
        functions = [raw_to_cxx_service_skeleton, raw_to_cxx_typebuild]
        args = [[pargs.interface, pargs.include], [pargs.interface, '', pargs.include]]
      elif op == "cxxservicebouncer":
        functions = [raw_to_cxx_service_bouncer, raw_to_cxx_typebuild]
        args = [['@Ptr', pargs.include], [pargs.interface, '', pargs.include]]
      elif op == "cxxservicebouncerregister":
        functions = [raw_to_cxx_service_bouncer, raw_to_cxx_typebuild]
        args = [['@Ptr', pargs.include], [pargs.interface, 'service', pargs.include]]
    #print("Executing %s functions on %s classes" % (len(functions), len(raw)))
      for i in range(len(functions)):
        cargs = [name, raw[c]] + args[i]
        tres = functions[i](*cargs)
        if type(tres) == type(''):
          res[1] += tres
        else:
          res[0] += tres[0]
          res[1] += tres[1]
          res[2] += tres[2]
      if split_output:
        out_name = pargs.output_file.replace("%s", c)
        out = open(out_name, "w")
        out.write(res[0] + res[1] + res[2])
        res = ["","",""]
    res = res[0] + res[1] + res[2]
  if not split_output:
    # Set output stream to file or stdout
    out = sys.stdout
    if pargs.output_file and pargs.output_file != "-" :
      out = open(pargs.output_file, "w")
    out.write(res)

main(sys.argv)
