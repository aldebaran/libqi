#! /usr/bin/env python

## Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.

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
import ctypes

medir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(medir, '..', 'lib', 'python2.7', 'site-packages'))
import shlib

qi_type = shlib.load_shlib('qitype', os.path.join(medir, '..'), False)

if qi_type is None:
  #Rerun in verbose mode to give more informations
  shlib.load_shlib('qitype', os.path.join(medir, '..'), True)
  raise Exception("Could not load libqitype shared module")

qi_type.signature_to_json.restype = ctypes.c_char_p

def signature_to_json(s):
  return handle.signature_to_json(s)

METHODS = 0
SIGNALS = 1
PROPERTIES = 2


# C++ -> signature mapping
CXX_SIG_MAP = {
  'unsigned int': 'L',
  'unsigned long': 'L',
  'unsigned short': 'W',
  'unsigned char': 'C',
  'int': 'l',
  'long': 'l',
  'short': 'w',
  'char': 'c',
  'int64_t': 'l',
  'uint64_t': 'L',
  'int32_t': 'i',
  'uint32_t': 'I',
  'int16_t': 'w',
  'uint16_t': 'W',
  'int8_t': 'c',
  'uint8_t': 'C',
  'char*': 's',
  'string': 's',
  'void':  'v',
  'GenericValue': 'm',
}

# signature to C++ type
SIG_CXX_MAP = {
    'c'    : 'signed char',
    'C'    : 'unsigned char',
    'w'    : 'short',
    'W'    : 'unsigned short',
    'i'    : 'qi::int32_t',
    'I'    : 'qi::uint32_t',
    'l'    : 'qi::int64_t',
    'L'    : 'qi::uint64_t',
    'f'    : 'float',
    'd'    : 'double',
    's'    : 'std::string',
    'm'    : 'qi::GenericValue',
    'v'    : 'void',
    'b'    : 'bool',
    'X'    : 'void*',
    '['    : 'std::vector',
    '{'    : 'std::map'
}

CONTAINERS_SIG = '{[('

# annotated name -> cxx type
ANNOTATION_CXX_MAP = {
}

def signature_to_json(s):
  jsig = qi_type.signature_to_json(s)
  import json
  sig = json.loads(jsig)
  return sig

def signature_to_cxxtype(s):
  """ Take a signature and return a C++ type that matches.
  """
  if not isinstance(s, basestring): #common pitfall, this works with unicode strings too
    raise Exception("Expected string, got " + str(s) + " which is " + str(type(s)))
  sig = signature_to_json(str(s)) # no unicode!
  if not len(sig) or not len(sig[0]):
    raise Exception("Invalid signature: " + s +", gave " + str(sig))
  return signature_to_cxxtype_(sig[0])

def function_signature_to_cxxtypes(s):
  """ Take a function signature and return
      An array of C++ types
  """
  if not isinstance(s, basestring): #common pitfall, this works with unicode strings too
    raise Exception("Expected string, got " + str(s) + " which is " + str(type(s)))
  jsig = qi_type.signature_to_json(s)
  import json
  sig = json.loads(jsig)
  if len(sig)!=1 or not len(sig[0]):
    raise Exception("Invalid signature: " + s)
  sig = sig[0] #unwrap toplevel array
  return map(signature_to_cxxtype_, sig[1])

def signature_to_cxxtype_(s):
  """ Return the C++ type to use for parsed signature s
  """
  (t, children, annotation) = s
  if t == '(':
    sname = annotation.split(',')[0]
    if sname in ANNOTATION_CXX_MAP:
      return ANNOTATION_CXX_MAP[sname]
    elif len(children) == 2:
      return 'std::pair<' + signature_to_cxxtype_(children[0]) + ',' + signature_to_cxxtype_(children[1]) + ' >'
    else:
      raise Exception("Unhandled tuple typpe " + str(s))
  elif t in '{[':
    res = SIG_CXX_MAP[t] + '<'
    res += ','.join(map(signature_to_cxxtype_, children))
    res += ' >'
    return res
  elif t == 'o':
    if len(annotation):
      return annotation + 'ProxyPtr'
    return 'qi::ObjectPtr' #FIXME specialized proxy from annotation
  elif len(annotation): #FIXME some flag to select default or annotation?
    return annotation
  else:
    return SIG_CXX_MAP[t]

PAIR_CLOSING_MAP = {
  '[': ']',
  '{': '}',
  '(': ')'
}
def json_to_signature(js):
  """ Reconstruct signature from JSON representation
  """
  (t, children, annotation) = js
  res = t + ''.join(map(json_to_signature, children)) + PAIR_CLOSING_MAP.get(t, '')
  if len(annotation):
    res += '<' + annotation + '>'
  return res

#mimou
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
    res = map(cxx_parsed_to_sig, p)
    return ''.join(res)
  if (type(p) == tuple):
    if re.search('vector$', p[0]):
      return "[" + cxx_parsed_to_sig(p[1]) + "]"
    elif re.search('map$', p[0]):
      return "{" + cxx_parsed_to_sig(p[1]) + "}"
    elif re.search('pair$', p[0]):
      return "(" + cxx_parsed_to_sig(p[1]) + ")"
    elif re.search('Future$', p[0]):
      return cxx_parsed_to_sig(p[1])
    else: # unknown template
      return 'X' + '<' + ','.join(p) + '>'
  elif p in CXX_SIG_MAP:
    return CXX_SIG_MAP[p]
  #still no match, try without namespace
  pend = p.split('::')[-1]
  if pend in CXX_SIG_MAP:
    return CXX_SIG_MAP[pend]
  if re.search('Ptr$', p):
    p = p.replace('ProxyPtr', '').replace('Ptr', '')
    return 'o<' + p + '>' #This is no duck
  print("Unhandled type " + p)
  return 'X<' + p + '>'


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
  parsed = cxx_type_parse(t)
  sig = cxx_parsed_to_sig(parsed)
  print(t + " => " + sig)
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
      match = re.match(r"(qi::)?SignalF<[^(]+\((.*)\)>", t)
      if not match:
        match = re.match(r"(qi::)?Signal<(.*)>", t)
      if match:
        t = match.expand(r"\2")
        sig = "(" + cxx_type_to_signature(t) + ")"
        sig = signature_to_json(sig)
        sig = sig[0][1] # unwrap toplevel array and method tuple
        sig = map(json_to_signature, sig)
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
  cret = signature_to_cxxtype(iret)
  iargs = method[1]
  cargs = map(signature_to_cxxtype, iargs)
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

def raw_to_interface(class_name, data, include, namespaces):
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
#include <qitype/genericobject.hpp>
@include@

@OPEN_NAMESPACE@

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
  QI_REGISTER_CHILD_OBJECT(@INAME@, name, @FIELDS@, ##__VA_ARGS__)

QI_REGISTER_OBJECT(@INAME@, @FIELDS@)

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

@CLOSE_NAMESPACE@
QI_TYPE_NOT_CLONABLE(@NAMESPACES@@INAME@);

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
    signature = ','.join(map(signature_to_cxxtype, sig[1]))
    signals_decl += '    qi::Signal<%s> & %s;\n' % (signature, name)
    ctor_decl.append('qi::Signal<%s> & %s' % (signature, name))
    ctor_init.append('%s(%s)' % (name, name))
    ctor_init0.append('%s(*new qi::Signal<%s>)' % (name, signature))
    dtor += '    delete &%s;\n' % (name)
    getters += '    qi::Signal<%s> & _interface_%s() { return %s;}\n' % (signature, name, name)
    fields.append('_interface_' + name)
  for prop in properties:
    name = prop[0]
    signature = signature_to_cxxtype(prop[1])
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

  open_namespace = ""
  close_namespace = ""
  ns_full = ""
  if namespaces:
    for n in namespaces:
      open_namespace += "namespace " + n + "\n{\n"
      close_namespace = "} // !" + n + "\n" + close_namespace
      ns_full = n + "::"

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
   '@include@': ''.join(['#include <' + x + '>\n' for x in include]),
   '@OPEN_NAMESPACE@': open_namespace,
   '@CLOSE_NAMESPACE@': close_namespace,
   '@NAMESPACES@': ns_full
   }
  for k in replace:
    skeleton = skeleton.replace(k, replace[k])
  skeleton = "/*" + raw_to_text({class_name: data}) + "*/" + skeleton
  return skeleton

def clean_extra_space(name):
  """ Clean extra spaces in type name, taking care of ">>"
  """
  return re.sub(r"([^>])\s>", r"\1>", name)

def raw_to_al_proxy(class_name, data):
  """ Generate an ALProxy for the given class
  """
  skeleton_header = """// Generated for @NAME@ version @VERSION@

#ifndef @UPNAME@PROXY_H_
#define @UPNAME@PROXY_H_

#include <alproxies/api.h>
#include <boost/shared_ptr.hpp>
#include <alvalue/alvalue.h>
#include <string>
#include <vector>

namespace AL
{
  class ALBroker;
  class ALProxy;
  class @PROXYNAME@;

  namespace detail {
    class @PROXYNAME@PostHandler
    {
    protected:
      @PROXYNAME@PostHandler(boost::shared_ptr<ALProxy> proxy);

    public:
      friend class AL::@PROXYNAME@;

@DECL6@    private:
      boost::shared_ptr<ALProxy> _proxy;
    };
  }

  @SUMMARY@
  /// \ingroup ALProxies
  class ALPROXIES_API @PROXYNAME@
  {
  private:
    boost::shared_ptr<ALProxy> _proxy;

  public:
    /// <summary>
    /// Default Constructor. If there is a broker in your process, which is always the case
    /// if you are in a module, then this will be found and used.
    /// </summary>
    @PROXYNAME@();

    /// <summary>
    /// Explicit Broker Constructor. If you have a smart pointer to a broker that you want
    /// to specify, then you can use this constructor. In most cases, the default constructor
    /// will do the work for you without passing a broker explicitly.
    /// </summary>
    /// <param name="broker">A smart pointer to your broker</param>
    @PROXYNAME@(boost::shared_ptr<ALBroker> broker);

    /// <summary>
    /// Explicit Proxy Constructor. Create a specialised proxy from a generic proxy.
    /// </summary>
    /// <param name="broker">A smart pointer to your broker</param>
    @PROXYNAME@(boost::shared_ptr<ALProxy> proxy);

    /// <summary>
    /// Remote Constructor. This constructor allows you to connect to a remote module by
    /// explicit IP address and port. This is useful if you are not within a process that
    /// has a broker, or want to connect to a remote instance of NAOqi such as another
    /// robot.
    /// </summary>
    /// <param name="ip">The IP address of the remote module you want to connect to</param>
    /// <param name="port">The port of the remote module, typically 9559</param>
    @PROXYNAME@(const std::string &ip, int port=9559);

    /// <summary>
    /// Gets the underlying generic proxy
    /// </summary>
    boost::shared_ptr<ALProxy> getGenericProxy();

@DECL4@
    detail::@PROXYNAME@PostHandler post;
  };

}
#endif // @UPNAME@PROXY_H_
"""
  skeleton_source = """// Generated for @NAME@ version @VERSION@

#include <alproxies/@DOWNNAME@proxy.h>
#include <alcommon/alproxy.h>
#include <alcommon/almodule.h>
#include <alcommon/albrokermanager.h>

namespace AL
{

  @PROXYNAME@::@PROXYNAME@()
    : _proxy(new AL::ALProxy(ALBrokerManager::getInstance()->getRandomBroker(), "@NAME@")),
      post(_proxy)
  {
  }

  @PROXYNAME@::@PROXYNAME@(boost::shared_ptr<ALProxy> pProxy)
    : _proxy(pProxy),
      post(_proxy)
  {
  }

  @PROXYNAME@::@PROXYNAME@(boost::shared_ptr<ALBroker> pBroker)
    : _proxy(new AL::ALProxy(pBroker, "@NAME@")),
      post(_proxy)
  {
  }

  @PROXYNAME@::@PROXYNAME@(const std::string &pIP, int pPort)
    : _proxy(new AL::ALProxy("@NAME@", pIP, pPort)),
      post(_proxy)
  {
  }

  boost::shared_ptr<ALProxy> @PROXYNAME@::getGenericProxy() {
    return _proxy;
  }

  // -----------------------------

@IMPL@
  // @PROXYNAME@PostHandler -----------------------------

namespace detail {
  @PROXYNAME@PostHandler::@PROXYNAME@PostHandler(boost::shared_ptr<ALProxy> proxy)
   : _proxy(proxy)
  {}

@POSTIMPL@}

}  // namespace AL
"""
  taskIdDoc = '/// <returns> brokerTaskID : The ID of the task assigned to it by the broker.</returns>\n'
  methods = data[0]
  summary = '/// <summary>' + '\n  ///'.join(data[3].split('\n')) + '</summary>'
  decl = ''
  declvoid = ''
  impl = ''
  postimpl = ''
  # Use ALValue instead of GenericValue for dynamic kind.
  SIG_CXX_MAP['m'] = 'AL::ALValue'
  SIG_CXX_MAP['i'] = 'int'
  # sort the methods
  methods.sort(key=lambda m: m[0])
  for method in methods:
    if method[0][0] == '_':
      continue
    (cret, typed_args, arg_names) = method_to_cxx(method)
    cargs = map(signature_to_cxxtype, method[1])
    #function_signature_to_cxxtypes(method[1])
    cargs = map(clean_extra_space, cargs)
    cret = clean_extra_space(cret)
    rawdoc = method[3].split('\n')
    arg_names = []
    #reformat doc and extract argument names
    retdoc = ''
    # handle multi-line stuff by delaying terminator
    doc = '/// <summary>\n/// ' + rawdoc[0]
    terminateLast = '\n/// </summary>\n'
    for arg in rawdoc[1:]:
      argl = arg.split(' ', 1)
      if argl[0] == 'param:':
        doc += terminateLast
        terminateLast = ' </param>\n'
        p = argl[1].split(' ', 1)
        doc += '/// <param name="'+p[0]+'"> ' + p[1]
        arg_names.append(p[0])
      elif argl[0] == 'return:':
        if cret != 'void': # workaround glitch in MetaMethodBuilder
          retdoc += '/// <returns> ' + argl[1] +' </returns>\n'
      else:
        doc += '\n/// ' + ' '.join(argl)
    doc += terminateLast
    # handle undocumented argsc
    i = len(arg_names) + 1
    while i <= len(cargs):
      doc += '/// <param name="arg' + str(i) + '"> arg </param>\n'
      arg_names.append('arg' + str(i))
      i = i+1
    # argument string
    argstring = ''
    argcall = ''
    for i in range(len(cargs)):
      if len(argstring):
        argstring += ', '
      argstring += 'const ' + cargs[i] +'& ' + arg_names[i]
      argcall += ', ' + arg_names[i]
    # construct full method decl
    call_kind = 'Void'
    if cret != 'void':
      call_kind='< ' + cret + ' >'
    decl += doc + retdoc + cret + ' ' + method[0] +'(' + argstring +');\n\n'
    impl += '  %s %s::%s(%s)\n  {\n    %s_proxy->call%s("%s"%s);\n  }\n\n' % (
      cret,
      class_name + 'Proxy',
      method[0],
      argstring,
      'return '*(cret != 'void'),
      call_kind,
      method[0],
      ' ' * (len(argcall)!=0) + argcall)

    if cret == 'void':
      declvoid += doc + taskIdDoc + 'int' + ' ' + method[0] +'(' + argstring +');\n\n'
      postimpl += '  int %s::%s(%s)\n  {\n    return _proxy->pCall("%s"%s);\n  }\n\n' % (
        class_name + "ProxyPostHandler",
        method[0],
        argstring,
        method[0],
        argcall
    )
  decl4 = '\n'.join(map(lambda x: '    ' * (len(x)!=0) + x, decl.split('\n')))
  decl6 = '\n'.join(map(lambda x: '      ' * (len(x)!=0) + x, declvoid.split('\n')))
  replace = {
    '@VERSION@': '0',
    '@NAME@': class_name,
    '@UPNAME@': class_name.upper(),
    '@DOWNNAME@': class_name.lower(),
    '@SUMMARY@': summary,
    '@PROXYNAME@': class_name + 'Proxy',
    '@DECL4@': decl4,
    '@DECL6@': decl6,
    '@IMPL@': impl,
    '@POSTIMPL@': postimpl,
  }
  for k in replace:
    skeleton_header = skeleton_header.replace(k, replace[k])
    skeleton_source = skeleton_source.replace(k, replace[k])
  return [skeleton_header, skeleton_source, '']

def raw_to_proxy(class_name, data, return_future, implement_interface, include, namespaces):
  """ Generate C++ proxy code from RAW
  @param return_future have the declared functions return a Future
  @param implement_interface make the proxy inherit from an interface, that
         can be generated with raw_to_interface
  """

  # Note: in interface mode, the generated class has no reason to ever be
  # used explicitly, so it is given a different name, and ClassProxy is set
  # to a typedef on IClass. That way other genertors can use to ClassProxyPtr

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

@open_namespace@

@forward_decls@

class @proxyName@: public ::qi::Proxy @if_inherit@
{
public:
  @proxyName@(qi::ObjectPtr obj)
  : qi::Proxy(obj)
@constructor_initList@
  {
@constructor@
  }
   public:
@publicDecl@

@privateDecl@
};

@QI_REGISTER_PROXY@
@close_namespace@

QI_TYPE_PROXY(@namepaces@@proxyName@);

#endif //@GARD@
"""

  if return_future and implement_interface:
    raise Exception("Cannot both return_future and implement_interface")
  if implement_interface:
    # In C++ "class foo;" and "typedef bar foo;" conflict, so use a preprocessor
    # define to inhibit standard fwdecl if the special interface version is used
    forward_decls = "class I@className@; typedef I@className@ @className@Proxy;typedef boost::shared_ptr<@className@Proxy> @className@ProxyPtr;\n#define FWD_@className@\n"
    proxy_name = class_name + "ProxyImpl"
    qi_register_proxy = "QI_REGISTER_PROXY_INTERFACE(@proxyName@, I@className@);"
  else:
    proxy_name = class_name + "Proxy"
    forward_decls = "class @className@Proxy;\ntypedef boost::shared_ptr<@className@Proxy> @className@ProxyPtr;\n"
    qi_register_proxy = "QI_REGISTER_PROXY(@proxyName@);"

  forward_decls = forward_decls.replace('@className@', class_name)
  #generate methods
  (methods, signals, properties) = (data[METHODS], data[SIGNALS], data[PROPERTIES])
  method_impls = ""
  fwdecl = dict()
  if implement_interface:
    call_begin = '(::qi::MetaCallType_Auto,'
    if_inherit = ', public I' + class_name
  else:
    call_begin = '(callType,'
    if_inherit = ''
  for method in methods:
    (cret, typed_args, arg_names) = method_to_cxx(method)
    method_name = method[0]
    if cret.find('Ptr') == len(cret)-3:
      tname = cret[0:-3]
      if tname.find('Proxy') == len(tname) - 5:
        tname = tname[0:-5]
      if tname.split('::')[-1] != 'Object':
        fwdecl[tname] = 1
        cret = tname + 'ProxyPtr'
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
      cxx_arg = signature_to_cxxtype(arg)
      if cxx_arg.find("Ptr") != -1:
        typed_args += '::qi::AutoGenericValuePtr p' + str(argIdx)
      else:
        typed_args += cxx_arg + ' p' + str(argIdx)
      typed_args += ', '
      argIdx = argIdx + 1
    # Add a final optional argument 'MetaCallType calltype=auto'
    if implement_interface:
      typed_args = typed_args[0:-2] #remove trailing comma
    else:
      typed_args = typed_args + '::qi::MetaCallType callType = ::qi::MetaCallType_Auto'
    #NOTE: should we return the future?
    method_impls += '  ' + out_ret + " " + method_name + "(" + typed_args + ") {\n    "
    if (cret != "void" or return_future):
      method_impls += "return "
    method_impls += '_obj->call<' + cret + ' >' + call_begin + '"' + method_name + '"' + arg_names + ");\n  }\n"
  signal_decl = ''
  ctor = ''
  # Make  a Signal field for each signal, bridge it to backend in ctor
  for sig in signals:
    signal_decl += '  qi::ProxySignal<void(' + ','.join(map(signature_to_cxxtype, sig[1])) +')> ' + sig[0] + ';\n'
    ctor += '  , {0}(obj, "{0}")\n'.format(sig[0])
  for prop in properties:
    signal_decl += '  qi::ProxyProperty<' + signature_to_cxxtype(prop[1]) + '> ' + prop[0] + ';\n'
    ctor += '  , {0}(obj, "{0}")\n'.format(prop[0])
  result = skeleton
  open_namespace = ""
  close_namespace = ""
  ns_full = ""
  if namespaces:
    for n in namespaces:
      open_namespace += "namespace " + n + "\n{\n"
      close_namespace = "} // !" + n + "\n" + close_namespace
      ns_full = n + "::"

  for k in fwdecl.keys():
    forward_decls += '#ifndef FWD_{1}\nclass {0}; typedef boost::shared_ptr<{0}> {0}Ptr;\n#endif\n'.format(k+'Proxy', k)
  replace = {
      'QI_REGISTER_PROXY': qi_register_proxy,
      'GARD': '_' + class_name.upper() + '_PROXY_HPP_',
      'open_namespace': open_namespace,
      'close_namespace': close_namespace,
      'className': class_name,
      'publicDecl': method_impls + signal_decl,
      'privateDecl': '',
      'constructor': '',
      'constructor_initList': ctor,
      'include': ''.join(['#include <' + x + '>\n' for x in include]),
      'forward_decls': forward_decls,
      'namepaces': ns_full,
      'if_inherit': if_inherit,
      'proxyName': proxy_name
  }
  for k in replace:
    qi_register_proxy = qi_register_proxy.replace('@' + k + '@', replace[k])
  replace['QI_REGISTER_PROXY'] = qi_register_proxy
  for k in replace:
    result = result.replace('@' + k + '@', replace[k])
  return ['', result, '']

def raw_to_cxx_typebuild(class_name, data, use_interface, register_to_factory, include, namespaces):
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

@OPEN_NAMESPACE@

static int @TYPE@init()
{
 qi::ObjectTypeBuilder<@TYPE@> builder;
@ADVERTISE@
  builder.registerType();
  return 0;
}
static int _init_@TYPE@ = @TYPE@init();
@REGISTER@
@CLOSE_NAMESPACE@
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

  open_namespace = ""
  close_namespace = ""
  if namespaces:
    for n in namespaces:
      open_namespace += "namespace " + n + "\n{\n"
      close_namespace = "} // !" + n + "\n" + close_namespace

  return template.replace('@TYPE@', class_name).replace('@ADVERTISE@', advertise).replace('@REGISTER@', register).replace('@INCLUDE@', include).replace('@OPEN_NAMESPACE@', open_namespace).replace('@CLOSE_NAMESPACE@', close_namespace)

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
    args = ','.join(map(signature_to_cxxtype, method[1]))
    result += '  %s %s(%s);\n' % (
      signature_to_cxxtype(method[2]),
      method_name,
      args
    )
  iface_ctor = []
  for signal in signals:
    iface_ctor.append('%s' % (signal[0]))
    result += '  qi::Signal<%s> %s;\n' % (
      ','.join(map(signature_to_cxxtype, signal[1])),
      signal[0]
    )
  if implement_interface:
    result += '  %s() :%s(%s) {}\n' % (class_name, class_name, ','.join(iface_ctor))
  result += '};\n\n'
  for method in methods:
    method_name = method[0]
    args = method[1]
    for i in range(len(args)):
      args[i] = signature_to_cxxtype(args[i]) + ' p' + str(i)
    args = ','.join(args)
    result += '%s %s::%s(%s)\n{\n  // Implementation of %s\n}\n' % (
      signature_to_cxxtype(method[2]),
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
  #print("woot %s %s" % (sig, ret))
  return ret

def runtime_to_raw(class_name, sd_url):
  abver = '.'.join(sys.version.split('.', 2)[0:2])
  me = os.path.dirname(os.path.abspath(__file__))
  lpath = me + '/../lib/python'+ abver + '/site-packages'
  sys.path.append(lpath)
  from qi import Session
  session = Session()
  session.connect(sd_url)
  obj = session.service(class_name)
  desc = obj.metaObject()
  #print(desc)
  methods = []
  for k in desc[0]:
    m = desc[0][k]
    #print(m) # ex: (0L, 'L', 'registerEvent', '(IIL)', 'doc', [argdoc], 'retdoc')
    if m[0] < 100:
      continue
    method_name = m[2]
    sig = m[3]
    # we must split the signature into its components
    sig = signature_to_json(sig)
    sig = sig[0][1] # unwrap toplevel array and method tuple
    sig = map(json_to_signature, sig)
    rettype = m[1]
    if method_name == 'metaObject': # HACK
      rettype = 'MetaObject'
    doc = m[4]
    # FIXME support argdoc/retdoc in RAW structure
    for argdoc in m[5]:
      doc += '\n' + 'param: ' + argdoc[0] + ' ' + argdoc[1]
    if len(m[6]):
      doc += '\nreturn: ' + m[6]
    methods.append((method_name, sig, rettype, doc))
  return {class_name : (methods, [], [], desc[3])}

def main(args):
  res = ''
  parser = argparse.ArgumentParser()
  parser.add_argument("--interface", "-i", help="Use interface mode", action='store_true')
  parser.add_argument("--output-file","-o", help="output file (stdout)")
  parser.add_argument("--output-mode","-m", default="txt", choices=["parse", "txt", "idl", "proxy", "proxyFuture", "cxxtype", "cxxtyperegisterfactory", "cxxtyperegisterservice", "cxxskel", "cxxservice", "cxxserviceregister", "cxxservicebouncer", "cxxservicebouncerregister", "interface", "boxinterface", "alproxy", "many"], help="output mode (stdout)")
  parser.add_argument("--include", "-I", default="", help="File to include in generated C++")
  parser.add_argument("--known-classes", "-k", default="", help="Comma-separated list of other handled classes")
  parser.add_argument("--classes", "-c", default="*", help="Comma-separated list of classes to select, optionally with per class ':operation'")
  parser.add_argument("--class-name", "-n", default="", help="C++ class name separated by include namespaces (ei: ns1::ns2::classname")
  parser.add_argument("--cxx-signature-mapping", default="", help="Extra C++->signature mapping(type=sig,type2=sig2)")
  parser.add_argument("input", nargs='+', help="input file(s)")

  pargs = parser.parse_args(args)
  pargs.input = pargs.input[1:]

  # Fill SIG_CXX_MAP with static stuff
  SIG_CXX_MAP['({I(Isss[(ss)]s)}{I(Is)}s)'] = 'qi::MetaObject'

  for m in pargs.cxx_signature_mapping.split(','):
    if len(m):
      m = m.split('=')
      CXX_SIG_MAP[m[0]] = m[1]
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
      namespaces_class_split = c.rsplit("::", 1) #[("ns1::ns1")+, "class:op1:op2"]
      if len(namespaces_class_split) > 1:
        namespaces = namespaces_class_split[0]
        class_and_op = namespaces_class_split[1]
      else:
        namespaces = ""
        class_and_op = namespaces_class_split[0]

      class_and_op_split = class_and_op.split(":", 1)
      cls = class_and_op_split[0]
      if len(class_and_op_split) > 1:
        class_op = class_and_op_split[1]
      else:
        class_op = ""

      if not c.strip():
        continue #be lenient on trailing ,
      c = ""
      if namespaces:
        c = namespaces + "::"
      c += cls
      if not c in raw:
        raise Exception("Requested class %s not found in %s" % (c, ','.join(raw.keys())))
      CXX_SIG_MAP[cls + 'Ptr'] = 'o<' + cls + '>'
      CXX_SIG_MAP[cls + 'ProxyPtr'] = 'o<' + cls + '>'
      ANNOTATION_CXX_MAP[cls] = cls + 'ProxyPtr'
      if pargs.class_name:
        newraw[pargs.class_name] = raw[c]
        if class_op:
          class_operation[pargs.class_name] = class_op
      else:
        newraw[c] = raw[c]
        if class_op:
          class_operation[c] = class_op
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
      namespaces_class_split = c.rsplit("::", 1) #[("ns1::ns1")+, "class:op1:op2"]
      if len(namespaces_class_split) > 1:
        namespaces = namespaces_class_split[0]
        name = namespaces_class_split[1]
      else:
        namespaces = ""
        name = namespaces_class_split[0]

      namespaces_list = list()
      if namespaces:
        namespaces_list = namespaces.split("::")
      if c in class_operation:
        op = class_operation[c].split(':')
        if len(op) > 1:
          name = op[1]
        op = op[0]
      functions = []
      args = []

      if op == "interface":
        functions = [raw_to_interface]
        args = [[pargs.include, namespaces_list]]
      elif op == "boxinterface":
        functions = [raw_to_boxinterface]
        args = [[]]
      elif op == "alproxy":
        functions = [raw_to_al_proxy]
        args = [[]]
      elif op == "proxy":
        functions = [raw_to_proxy]
        args = [[False, pargs.interface, pargs.include, namespaces_list]]
      elif op == "proxyFuture":
        functions = [raw_to_proxy]
        args = [[True, pargs.interface, pargs.include, namespaces_list]]
      elif op == "cxxtype":
        functions = [raw_to_cxx_typebuild]
        args = [[pargs.interface, '', pargs.include, namespaces_list]]
      elif op == "cxxtyperegisterfactory":
        functions = [raw_to_cxx_typebuild]
        args = [[pargs.interface, 'factory', pargs.include, namespaces_list]]
      elif op == "cxxtyperegisterservice":
        functions = [raw_to_cxx_typebuild]
        args = [[pargs.interface, 'service', pargs.include, namespaces_list]]
      elif op == "cxxskel":
        functions = [raw_to_cxx_service_skeleton]
        args = [[pargs.interface, pargs.include]]
      elif op == "cxxserviceregister":
        functions = [raw_to_cxx_service_skeleton, raw_to_cxx_typebuild]
        args = [[pargs.interface, pargs.include], [pargs.interface, 'service', pargs.include, namespaces_list]]
      elif op == "cxxservice":
        functions = [raw_to_cxx_service_skeleton, raw_to_cxx_typebuild]
        args = [[pargs.interface, pargs.include], [pargs.interface, '', pargs.include, namespaces_list]]
      elif op == "cxxservicebouncer":
        functions = [raw_to_cxx_service_bouncer, raw_to_cxx_typebuild]
        args = [['@Ptr', pargs.include], [pargs.interface, '', pargs.include, namespaces_list]]
      elif op == "cxxservicebouncerregister":
        functions = [raw_to_cxx_service_bouncer, raw_to_cxx_typebuild]
        args = [['@Ptr', pargs.include], [pargs.interface, 'service', pargs.include, namespaces_list]]
    #print("Executing %s functions on %s classes" % (len(functions), len(raw)))

      for i in range(len(functions)):
        cargs = [name, raw[c]] + args[i]
        tres = functions[i](*cargs)
        if type(tres) == type([]):
          res[0] += tres[0]
          res[1] += tres[1]
          res[2] += tres[2]
        else:
          res[1] += tres

      if split_output:
        out_name = pargs.output_file.replace("%s", c)
        out = open(out_name, "w")
        out.write(res[0] + res[1] + res[2])
        res = ["","",""]

  if not split_output:
    names = pargs.output_file.split(',')
    if len(names) > 1:
      for i in range(len(names)):
        out = open(names[i], "w")
        out.write(res[i])
    else:
      if type(res) == type([]):
        res = ''.join(res)
      # Set output stream to file or stdout
      out = sys.stdout
      if pargs.output_file and pargs.output_file != "-" :
        out = open(pargs.output_file, "w")
      out.write(res)

main(sys.argv)
