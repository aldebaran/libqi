/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 *  See COPYING for the license
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>

#include <qiclang.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <clang-c/Index.h>

namespace qi {
  namespace clang {
class clstring
{
public:
  clstring() {}
  clstring(CXString s) :s(s) {}
  ~clstring()
  {
    if (s.data) // assuming s.data cannot be null for allocated string
      clang_disposeString(s);
  }
  void operator = (CXString newS)
  {
    if (s.data)
      clang_disposeString(s);
    s = newS;
  }
  const char* str() const { return clang_getCString(s);}
  operator std::string() const { const char* res = str(); return res?res:"";}
private:
  void operator = (const clstring& b) {}
  CXString s;
  clstring(const clstring& b) {}
};


std::ostream& operator << (std::ostream& o, const clstring& cl)
{
  return o << cl.str();
}

std::string Location::toString() const
{
  std::stringstream s;
  s << file <<":" << line <<":" << column;
  return s.str();
}
  
std::string NSNamed::toString() const
{
  std::string res;
  for (unsigned i=0; i<ns.size(); ++i)
    res += ns[i] + "::";
  res += name;
  return res;
}

std::string NSNamed::namespaces() const
{
  std::string res;
  for (unsigned i=0; i<ns.size(); ++i)
  {
    res += ns[i];
    if (i != ns.size()-1)
      res += "::";
  }
  return res;
}

std::string Type::toString() const
{
  std::string res = NSNamed::toString();
  if (!templateArguments.empty())
  {
    res += '<';
    for (unsigned j=0; j<templateArguments.size(); ++j)
    {
      if (j)
        res += ',';
      res += templateArguments[j].toString();
    }
    res += '>';
  }
  return res;
}


std::string Method::toString() const
{
  std::string res = NSNamed::toString();
  res += '(';
  for (unsigned i=0; i<arguments.size(); ++i)
  {
    res += arguments[i].toString();
    if (i!= arguments.size()-1)
      res += ',';
  }
  res += ") -> " + result.toString();
  return res;
}


void TranslationUnit::addClass(const Class& c)
{
  if (classByUsr.find(c.usr) != classByUsr.end())
    throw std::runtime_error("Class already present");
  Class * nc = new Class(c);
  classes.push_back(nc);
  classByUsr[nc->usr] = nc;
}

void Class::dump(std::ostream& os) const
{
  os << NSNamed::toString() << " {" << std::endl;
  for (unsigned i=0; i<methods.size(); ++i)
    os << methods[i].toString() << std::endl;
  for (unsigned i=0; i<constructors.size(); ++i)
    os << constructors[i].toString() << std::endl;
  for (unsigned i=0; i<fields.size(); ++i)
    os << fields[i].name << " -> " << fields[i].type.toString() << std::endl;
  os << '}' << std::endl;
}

void TranslationUnit::dump(std::ostream& os) const
{
  for (unsigned i=0; i<methods.size(); ++i)
    os << methods[i].toString() << std::endl;
  for (unsigned i=0; i<classes.size(); ++i)
  {
    classes[i]->dump(os);
    os << std::endl;
  }
}

Class::Class(const std::string& usr)
: usr(usr)
{
}

TranslationUnit::TranslationUnit()
{
}

TranslationUnit::~TranslationUnit()
{
  for (unsigned i=0; i<classes.size(); ++i)
    delete classes[i];
}

Location CXSourceLocation_to_Location(CXSourceLocation loc)
{
  Location res;
  CXFile f;
  clang_getSpellingLocation(loc, &f, &res.line, &res.column, &res.offset);
  res.file = clstring(clang_getFileName(f));
  return res;
}

void fillComment(CXCursor cursor, Comment& target)
{
  target.commentRaw = clstring(clang_Cursor_getRawCommentText(cursor));
  CXComment comment = clang_Cursor_getParsedComment(cursor);
  target.commentHtml = clstring(clang_FullComment_getAsHTML(comment));
  target.commentXml = clstring(clang_FullComment_getAsXML(comment));
}


Type CXType_to_Type(CXType t)
{
  Type res;
  t = clang_getCanonicalType(t);
  res.isRef = (t.kind == CXType_LValueReference || t.kind == CXType_RValueReference);
  if (res.isRef)
    t = clang_getPointeeType(t); // lucky for us, this works for references
  // Check const after dereference
  res.isConst = clang_isConstQualifiedType(t);
  std::string s = clstring(clang_getTypeSpelling(t));
  // spelling contains 'const', no API to get unconstified type
  if (res.isConst)
  {
    if (s.substr(0, 6) == "const ")
      s = s.substr(6);
    else if (s.substr(s.size()-5) == "const")
      s = s.substr(0, s.size()-5);
    else
      throw std::runtime_error("Expected const at beginnign or end of " + s);
  }
  size_t p = s.find('<');
  if (p != s.npos)
    s = s.substr(0, p);
  std::vector<std::string> comps;
  boost::split(comps, s, boost::algorithm::is_any_of(":"), boost::token_compress_on);
  res.name = comps.back();
  comps.pop_back();
  res.ns = comps;
  int nt = clang_Type_getNumTemplateArguments(t);
  for (int i=0; i<nt; ++i)
  {
    CXType targ = clang_Type_getTemplateArgument(t, i);
    res.templateArguments.push_back(CXType_to_Type(targ));
  }
  return res;
}


/// Get the namespaces in which the declaration is semanitacly situated.
std::vector<std::string> namespaces(CXCursor decl)
{
  std::vector<std::string> res;
  while (true)
  {
    decl = clang_getCursorSemanticParent(decl);
    if (decl.kind != CXCursor_Namespace)
      break;
    res.push_back(clstring(clang_getCursorSpelling(decl)).str());
  }
  // stop at kinds: unkwown, class, struct, template and translationunit (root)

  // reverse it
  unsigned sz = res.size();
  for (unsigned i=0; i < sz/2; ++i)
    std::swap(res[i], res[sz-1-i]);
  return res;
}

/// Return usr for semantic context of a declaration
std::string semanticContextUsr(CXCursor decl)
{
  decl = clang_getCursorSemanticParent(decl);
  return clstring(clang_getCursorUSR(decl)).str();
}

void CXType_to_Method(CXType t, Method& m)
{
  int nargs = clang_getNumArgTypes(t);
  for (int i=0; i<nargs; ++i)
  {
    CXType arg = clang_getArgType(t, i);
    Type at = CXType_to_Type(arg);
    m.arguments.push_back(at);
  }
  CXType res = clang_getResultType(t);
  Type rt = CXType_to_Type(res);
  m.result = rt;
}


// Unused generic AST visitor/dumper
CXChildVisitResult visitAll(CXCursor cursor, CXCursor parent, CXClientData d)
{
  static std::string indent;
  CXCursorKind kind = clang_getCursorKind(cursor);
  clstring spelling(clang_getCursorSpelling(cursor));
  clstring kindSpelling(clang_getCursorKindSpelling(kind));
  std::cerr << indent << '*' << kindSpelling << ' ' << spelling << std::endl;
  clang_bearDumpStuff(cursor);
  indent += "  ";
  clang_visitChildren(cursor, &visitAll, d);
  indent = indent.substr(2);

  CXCursor tref = clang_getCursorReferenced(cursor);
  if (clang_equalCursors(tref,cursor))
    std::cerr << indent << "ref self" << std::endl;
  else
  {
    std::cerr << indent << "ref "; // no endl
    visitAll(tref, parent, d);
  }

  CXType type = clang_getCursorType(cursor);
  type = clang_getCanonicalType(type);
  CXCursor declCursor = clang_getTypeDeclaration(type);
  if (
    !clang_Cursor_isNull(declCursor)
    && !clang_equalCursors(declCursor,cursor)
    && declCursor.kind != CXCursor_NoDeclFound
  )
  {
    std::cerr << indent << "decl ";
    visitAll(declCursor, parent, d);
  }
  
  return CXChildVisit_Continue;
}


int index_abort(CXClientData, void* r)
{
  return 0;
}

void index_diagnostic(CXClientData d,
                     CXDiagnosticSet diag, void *reserved)
{
  
}

CXIdxClientFile index_enteredMainFile(CXClientData client_data,
                                     CXFile mainFile, void *reserved)
{
  return 0;
}

CXIdxClientASTFile index_ppIncludedFile(CXClientData client_data,
                                        const CXIdxIncludedFileInfo *)
{
  return 0;
}

CXIdxClientASTFile index_importedASTFile(CXClientData d, const CXIdxImportedASTFileInfo *)
{
  return 0;
}


CXIdxClientContainer index_startedTranslationUnit(CXClientData client_data,
                                                 void *reserved)
{
  return 0;
}

void index_indexDeclaration(CXClientData d, const CXIdxDeclInfo * decl)
{
  TranslationUnit& tu = *(TranslationUnit*)d;
  const CXIdxEntityInfo* ent = decl->entityInfo;

  CXSourceLocation cxloc = clang_getCursorLocation(ent->cursor);
  Location loc = CXSourceLocation_to_Location(cxloc);
  if (tu.filter && !tu.filter(loc))
    return;
  Class* owner = 0;

  if (ent->templateKind == CXIdxEntity_Template)
    return; // This thing does not exist for us
  switch(ent->kind)
  {
  case CXIdxEntity_CXXNamespace:
    break;
  case CXIdxEntity_CXXClass: // catches templates?
  case CXIdxEntity_Struct:
    {
      std::string usr = clstring(clang_getCursorUSR(ent->cursor)).str();
      if (tu.classByUsr.find(usr)== tu.classByUsr.end())
      {
        Class c(usr);
        fillComment(ent->cursor, c);
        c.name = clstring(clang_getCursorSpelling(ent->cursor)).str();
        c.ns = namespaces(ent->cursor);
        tu.addClass(c);
      }
    }
    break;
  case CXIdxEntity_Enum:
    {
      Enum e;
      e.name = clstring(clang_getCursorSpelling(ent->cursor)).str();
      e.ns =  namespaces(ent->cursor);
      tu.enums.push_back(e);
    }
    break;
  case CXIdxEntity_EnumConstant:
    break;
  case CXIdxEntity_CXXConstructor:
    {
      CX_CXXAccessSpecifier ac = clang_getCXXAccessSpecifier(ent->cursor);
      if (ac != CX_CXXPublic)
        break;
      owner = tu.classByUsr[semanticContextUsr(ent->cursor)];
      if (!owner)
        break;
      Method m;
      fillComment(ent->cursor, m);
      m.name = owner->name;
      CXCursor methodCursor = ent->cursor;
      CXType ftype = clang_getCursorType(methodCursor);
      CXType_to_Method(ftype, m);
      if (owner)
        owner->constructors.push_back(m);
    }
    break;
  case CXIdxEntity_Field:
    owner = tu.classByUsr[semanticContextUsr(ent->cursor)];
    if (!owner)
      break;
    {
      Field f;
      fillComment(ent->cursor, f);
      CXType t = clang_getCursorType(ent->cursor);
      f.type = CXType_to_Type(t);
      f.name = clstring(clang_getCursorSpelling(decl->cursor)).str();
      owner->fields.push_back(f);
    }
    break;
  //case CXIdxEntity_Variable:
  //case CXIdxEntity_CXXStaticVariable:
  case CXIdxEntity_CXXInstanceMethod:
    owner = tu.classByUsr[semanticContextUsr(ent->cursor)];
    if (!owner)
      break; // we are not interested in this class
  case CXIdxEntity_Function:
  case CXIdxEntity_CXXStaticMethod:
    {
      CX_CXXAccessSpecifier ac = clang_getCXXAccessSpecifier(ent->cursor);
      if (ac != CX_CXXPublic && ac != CX_CXXInvalidAccessSpecifier)
        break;
      Method m;
      fillComment(ent->cursor, m);
      m.isStatic = (ent->kind!=CXIdxEntity_CXXInstanceMethod);
      CXCursor methodCursor = ent->cursor;
      // bare name
      m.name = clstring(clang_getCursorSpelling(methodCursor)).str();
      m.ns = namespaces(methodCursor);
      CXType ftype = clang_getCursorType(methodCursor);
      CXType_to_Method(ftype, m);
      if (owner)
        owner->methods.push_back(m);
      else
        tu.methods.push_back(m);
    }
    break;
  }
}

void index_indexEntityReference(CXClientData d, const CXIdxEntityRefInfo *)
{
 
}

IndexerCallbacks index_callbacks = {
  &index_abort,
  &index_diagnostic,
  &index_enteredMainFile,
  &index_ppIncludedFile,
  &index_importedASTFile,
  &index_startedTranslationUnit,
  &index_indexDeclaration,
  &index_indexEntityReference
};

void Diagnostic::dump(std::ostream& os) const
{
  for (unsigned i=0; i<errors.size(); ++i)
    errors[i].dump(os);
  for (unsigned i=0; i<warnings.size(); ++i)
    warnings[i].dump(os);
}

void Diagnostic::Message::dump(std::ostream& os) const
{
  if (message.empty())
    return;
  os << message.front() << std::endl;
  for (unsigned i=0; i<message.size(); ++i)
    os << "  " << message[i] << std::endl;
}


Diagnostic TranslationUnit::parse(const std::vector<std::string>& args)
{
  int argc = args.size();
  char** argv = new char*[argc];
  for (unsigned i=0; i<argc; ++i)
    argv[i] = const_cast<char*>(args[i].c_str());
  return parse(argc, argv);
}

Diagnostic TranslationUnit::parse(int argc, char** argv)
{
  CXIndex cxindex = clang_createIndex(0, 0);
  CXIndexAction indexAction = clang_IndexAction_create(cxindex);
  CXTranslationUnit tu;
  clang_indexSourceFile(indexAction, this,
    &index_callbacks, sizeof(index_callbacks),
    CXIndexOpt_None,
    argv[0],
    argv+1, argc-1,
    0, 0,
    &tu, 0); // CXTranslationUnit_None);
  int n = clang_getNumDiagnostics(tu);
  Diagnostic res;
  Diagnostic::Message* lastMessage; // needed to attach notes
  for (unsigned i = 0; i != n; ++i)
  {
    CXDiagnostic diag = clang_getDiagnostic(tu, i);
    clstring str(
      clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions()));
    CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diag);
    switch (severity)
    {
    case CXDiagnostic_Fatal:
    case CXDiagnostic_Error:
      res.errors.push_back(Diagnostic::Message((Diagnostic::Message::Severity)severity, str));
      lastMessage = &res.errors.back();
      break;
    case CXDiagnostic_Warning:
      res.warnings.push_back(Diagnostic::Message((Diagnostic::Message::Severity)severity, str));
      lastMessage = &res.warnings.back();
      break;
    case CXDiagnostic_Note:
      if (!lastMessage)
        break; // should not happen, but can't realy assert on it as it comes from clang
      lastMessage->message.push_back(str);
      break;
    case CXDiagnostic_Ignored:
      break;
    }
    clang_disposeDiagnostic(diag);
  }

  clang_disposeTranslationUnit(tu);
  clang_IndexAction_dispose(indexAction);
  clang_disposeIndex(cxindex);
  return res;
}


class SerialisationNode
{
public:
  class Visitor
  {
  public:
    /// return false to abort visit of this node and children
    virtual bool enter(SerialisationNode& node, const std::string& name) { return true;}
    virtual void attribute(SerialisationNode& node, const std::string& key, const std::string& val) {}
    virtual void endAttributes(SerialisationNode& node, const std::string& data) {}
    virtual bool child(SerialisationNode& node, SerialisationNode& child) { return true;}
    virtual void leave(SerialisationNode& node, const std::string& name) {}
  };
  SerialisationNode() {}
  SerialisationNode(const std::string& name)
  : name(name)
  {}
  std::string name;
  std::string data;
  std::map<std::string, std::string> attributes;
  std::vector<SerialisationNode> children;
  SerialisationNode& setName(const std::string& k)
  {
    name = k;
    return *this;
  }
  SerialisationNode& attr(const std::string& k, bool b)
  {
    return attr(k, b?"1":"0");
  }
  SerialisationNode& attr(const std::string& k, const std::string& v)
  {
    attributes[k] = v;
    return *this;
  }
  template<typename T>
  SerialisationNode& attr(const std::string& k, const T& v)
  {
    attributes[k] = boost::lexical_cast<std::string>(v);
    return *this;
  }
  SerialisationNode& child(const SerialisationNode& b)
  {
    children.push_back(b);
    return *this;
  }
  template<typename T>
  SerialisationNode& child(const std::string& name, const T& v)
  {
    SerialisationNode node(name);
    node << v;
    return child(node);
  }
  template<typename T>
  SerialisationNode& maybeChild(const std::string& name, const T& v)
  {
    if (!v.empty())
      child(name, v);
    return *this;
  }
  SerialisationNode& childData(const std::string& name, const std::string& data)
  {
    SerialisationNode c(name);
    c.data = data;
    children.push_back(c);
    return *this;
  }
  void visit(Visitor& v)
  {
    if (!v.enter(*this, this->name))
      return;
    for(std::map<std::string, std::string>::iterator it = attributes.begin();
      it != attributes.end(); ++it)
    {
      v.attribute(*this, it->first, it->second);
    }
    v.endAttributes(*this, data);
    for (unsigned i=0; i<children.size(); ++i)
    {
      if (v.child(*this, children[i]))
        children[i].visit(v);
    }
    v.leave(*this, this->name);
  }
};

class XmlVisitor: public SerialisationNode::Visitor
{
public:
  XmlVisitor(std::ostream& o) : o(o) {}
  virtual bool enter(SerialisationNode& node, const std::string& name)
  {
    if (node.attributes.empty() && node.children.empty() && node.data.empty())
      return false;
    o << indent << '<' + name;
    indent += "  ";
    return true;
  }
  virtual void attribute(SerialisationNode& node, const std::string& key, const std::string& val)
  {
    o << ' ' << key << "=\"" << val << "\"";
  }
  virtual void endAttributes(SerialisationNode& node, const std::string& data)
  {
    o << '>';
    if (!node.children.empty() || !node.data.empty())
      o << std::endl;
    if (!node.data.empty())
      o << node.data << std::endl;
  }
  virtual void leave(SerialisationNode& node, const std::string& name)
  {
    indent = indent.substr(2);
    if (!node.children.empty() || !node.data.empty())
      o << indent;
    o << "</" << name << ">" << std::endl;
  }
  private:
    std::ostream& o;
    std::string indent;
};

template<typename T> SerialisationNode& operator << (SerialisationNode& s, const T* v)
{
  return s << *v;
}

template<typename T> SerialisationNode& operator << (SerialisationNode& s, const std::vector<T>& v)
{
  for (unsigned i=0; i<v.size(); ++i)
  {
    SerialisationNode n;
    n << v[i];
    s.child(n);
  }
  return s;
}

inline std::string cdata(const std::string& s)
{
  return "<![CDATA[" + s + "]]>";
}

SerialisationNode& operator << (SerialisationNode& node, const Location& l)
{
  return node.attr("location", l.toString());
}

SerialisationNode& operator << (SerialisationNode& node, const Comment& t)
{
  return node.setName("comment")
             .childData("bare", cdata(t.commentRaw)) // this is obviously cheating
             .childData("html", cdata(t.commentHtml))
             .childData("xml", t.commentXml);
}

SerialisationNode& operator << (SerialisationNode& node, const Type& t)
{
  return node.setName("type")
             .attr("name", t.name)
             .attr("namespace", t.namespaces())
             .attr("const", t.isConst)
             .attr("ref", t.isRef)
             .maybeChild("templates", t.templateArguments);
}

SerialisationNode& operator << (SerialisationNode& node, const Enum& e)
{
  return node.setName("enum")
    .attr("name", e.name)
    .attr("namespace", e.namespaces())
    .child("comments", (Comment&)e);
}

SerialisationNode& operator << (SerialisationNode& node, const Field& f)
{
  return node.setName("field")
    .attr("name", f.name)
    .child("comments", (Comment&)f)
    .child("type", f.type);
}

SerialisationNode& operator << (SerialisationNode& node, const Method& m)
{
  return node.setName("method").attr("static", m.isStatic)
    .attr("name", m.name)
    .attr("namespace", m.namespaces())
    .child("comments", (Comment&)m)
    .child("result", m.result)
    .child("arguments", m.arguments);
}

SerialisationNode& operator << (SerialisationNode& node, const Class& c)
{
  return node.setName("class")
    .attr("usr", c.usr)
    .attr("name", c.name)
    .attr("namespace", c.namespaces())
    .child("comments", (Comment&)c)
    .child("constructors", c.constructors)
    .child("methods", c.methods)
    .child("fields", c.fields);
}

SerialisationNode& operator << (SerialisationNode& node, const TranslationUnit& tu)
{
  node.setName("tu");
  return node << tu.methods << tu.classes << tu.enums;
}


  } // namespace clang
} // namespace qi
using namespace qi::clang;

bool filter_contains(const Location& l, const std::string& s)
{
  return l.file.find(s) != std::string::npos;
}
int main(int argc, char** argv)
{
  TranslationUnit tu;
  std::string output;
  bool append = false;
  bool raw = false;
  int argp = 0;
  while (argp < argc)
  {
    std::string arg(argv[++argp]);
    if (arg == "--")
    {
      ++argp;
      break;
    }
    if (arg == "--help" || arg == "-h")
    {
      std::cerr << "Usage: " << argv[0] << " [OPTIONS] -- [CLANG_CMDLINE]" << std::endl
         << "  --filter STRING    : Only output components in locations containing STRING.\n"
            "  --output PATH      : Write output to PATH (defaults stdout)\n"
            "  --append           : Append to output instead of overwriting\n"
            "  --raw              : Output a RAW representation instead of XML\n"
            ;
      return 0;
    }
    if (arg == "--filter")
    {
      std::string filt = argv[++argp];
      tu.filter = boost::bind(&filter_contains, _1, filt);
    }
    else if (arg == "--output")
      output = argv[++argp];
    else if (arg == "--append")
      append = true;
    else if (arg == "--raw")
      raw = true;
    else
      break;
  }
  Diagnostic d = tu.parse(argc -argp, argv + argp);
  SerialisationNode tuNode;
  tuNode << tu;
  std::ostream* outputStream;
  if (output == "-" || output.empty())
  {
    outputStream = &std::cout;
  }
  else
  {
    outputStream = new std::ofstream(output.c_str(),
      append? ( std::ios::out | std::ios::app) : std::ios::out);
  }
  if (raw)
    tu.dump(*outputStream);
  else
  {
    XmlVisitor v(*outputStream);
    tuNode.visit(v);
  }
  d.dump(std::cerr);
  if (outputStream != &std::cout)
    delete (std::ofstream*)outputStream;
  return 0;
}
