/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <cstring>

#include <qitype/signature.hpp>
#include <qitype/type.hpp>
#include <boost/make_shared.hpp>
#include "signatureconvertor.hpp"
qiLogCategory("qi.signature");

namespace qi {

  static std::string makeTupleAnnotation(const std::string& name, const std::vector<std::string>& annotations) {
    std::string res;

    res += '<';
    res += name;
    for (unsigned i = 0; i < annotations.size(); ++i)
      res += ',' + annotations[i];
    res += '>';
    return res;
  }

  qi::Signature makeListSignature(const qi::Signature &element) {
    std::stringstream res;
    res << (char)Signature::Type_List;
    res << element.toString();
    res << (char)Signature::Type_List_End;
    return qi::Signature(res.str());
  }

  qi::Signature makeTupleSignature(const qi::Signature &element) {
    std::stringstream res;
    res << (char)Signature::Type_Tuple;
    res << element.toString();
    res << (char)Signature::Type_Tuple_End;
    return qi::Signature(res.str());
  }

  qi::Signature makeMapSignature(const qi::Signature &key, const qi::Signature &value) {
    std::stringstream res;
    res << (char)Signature::Type_Map;
    res << key.toString();
    res << value.toString();
    res << (char)Signature::Type_Map_End;
    return qi::Signature(res.str());
  }

  qi::Signature makeTupleSignature(const std::vector<qi::GenericValuePtr>& vgv, bool resolve, const std::string &name, const std::vector<std::string>& names)
  {
    std::stringstream res;
    res << (char)Signature::Type_Tuple;
    for (unsigned int i = 0; i < vgv.size(); ++i)
      res << vgv[i].signature(resolve).toString();
    res << (char)Signature::Type_Tuple_End;
    if (names.size() == vgv.size())
      res << makeTupleAnnotation(name, names);
    return qi::Signature(res.str());
  }

  qi::Signature makeTupleSignature(const std::vector<Type*>& vt, const std::string &name, const std::vector<std::string>& names)
  {
    std::stringstream res;
    res << (char)Signature::Type_Tuple;
    for (unsigned int i = 0; i < vt.size(); ++i)
      res << vt[i]->signature().toString();
    res << (char)Signature::Type_Tuple_End;
    if (names.size() == vt.size())
      res << makeTupleAnnotation(name, names);
    return qi::Signature(res.str());
  }

  float qi::Signature::isConvertibleTo(const qi::Signature& b) const
  {
    /* The returned float is just a basic heuristic, it does not handle:
   * - comparison between integral types
   * - Weaker error score for deeper (in containers) struct.
  */
    if (size() != b.size())
      return 0;
    int error = 0;
    float childErr = 0.0f;
    static const char numeric[] = "bcCwWiIlLfd";
    static const char integral[] = "bcCwWiIlL";
    static const char floating[] = "fd";
    static const char container[] = "[{(";
    Signature::iterator is = begin();
    Signature::iterator id = b.begin();
    for (;is!=end() && id!= b.end(); ++is,++id)
    {
      Signature::Type s = is.type();
      Signature::Type d = id.type();
      if (d == Type_Unknown)
      {
        // We cannot anwser the question for unknown types. So let it pass
        // and the conversion code will decide.
        // Type_Unknown is not serializable anyway.
        if (s != Type_Unknown)
          error += 10; // Weird but can happen with object pointers
        continue;
      }
      if (d == Type_Dynamic // Dynamic can convert to whatever
          || s == Type_None) // None means parent is empty container
      {
        error += 5; // big malus for dynamic
        continue;
      }
      // Main switch on source sig
      if (strchr(numeric, s))
      { // Numeric
        if (!strchr(numeric, d))
          return 0;
        // Distance heuristic
        if (strchr(integral, s) && strchr(floating, d))
          error += 2; // integral->float
        if (strchr(floating, s) && strchr(integral, d))
          error += 3; // float->integral
        if (s!= 'b' && d == 'b')
          error += 4; // ->bool
      }
      else if (strchr(container, s))
      { // Container, list or map
        if (d != s)
          return 0; // Must be same container
        float childRes = is.children().isConvertibleTo(id.children());
        if (!childRes)
          return 0; // Just check subtype compatibility
        // the lower the error the better
        childErr += 1.0f - childRes*1.01;
      }
      else
        if (d != s)
          return 0;
    }
    assert(is==end() && id==b.end()); // we allready exited on size mismatch
    return 1.0f - childErr - ((float)error) / 100.0f;
  }

  Signature Signature::fromType(Signature::Type t)
  {
    char res[2] = { (char)t, 0};
    return Signature(res);
  }

  //compare signature without taking annotation into account
  bool Signature::operator==(const Signature &rhs) const {
    Signature::iterator it;
    Signature::iterator it2;
    it2 = begin();
    for (it = rhs.begin(); it != rhs.end(); ++it) {
      if (it2 == end()) //not the same number of elements
        return false;
      if (it.type() != it2.type()) //different types
        return false;
      if (it.hasChildren() != it2.hasChildren())
        return false;
      if (it.hasChildren() && it.children() != it2.children()) //different children
        return false;
      ++it2;
    }
    return (it2 == end());
  }


  static bool _is_valid(const std::string& s, unsigned int& current, qi::Signature::Type type, qi::Signature::Type closing)
  {
    int         arguments = 0;
    while (current < s.size() && s[current] != closing)
    {
      switch (static_cast<qi::Signature::Type>(s[current]))
      {
      case qi::Signature::Type_Void:
      case qi::Signature::Type_Bool:
      case qi::Signature::Type_Int8:
      case qi::Signature::Type_UInt8:
      case qi::Signature::Type_Int16:
      case qi::Signature::Type_UInt16:
      case qi::Signature::Type_Int32:
      case qi::Signature::Type_UInt32:
      case qi::Signature::Type_Int64:
      case qi::Signature::Type_UInt64:
      case qi::Signature::Type_Float:
      case qi::Signature::Type_Double:
      case qi::Signature::Type_String:
      case qi::Signature::Type_List_End:
      case qi::Signature::Type_Map_End:
      case qi::Signature::Type_Tuple_End:
      case qi::Signature::Type_Dynamic:
      case qi::Signature::Type_Raw:
      case qi::Signature::Type_Pointer:
      case qi::Signature::Type_Object:
      case qi::Signature::Type_Unknown:
      case qi::Signature::Type_None:
        break;
      case qi::Signature::Type_List:
      {
        if (_is_valid(s, ++current, qi::Signature::Type_List, qi::Signature::Type_List_End) == false)
          return false;

        break;
      }
      case qi::Signature::Type_Tuple:
      {
        if (s[++current] == qi::Signature::Type_Tuple_End)
          return true; // Empty tuple is valid.
        if (_is_valid(s, current, qi::Signature::Type_Tuple, qi::Signature::Type_Tuple_End) == false)
          return false;
        break;
      }
      case qi::Signature::Type_Map:
        if (_is_valid(s, ++current, qi::Signature::Type_Map, qi::Signature::Type_Map_End) == false)
          return false;
        break;
      default:
        qiLogVerbose() << "Element '" << s[current] << "' is unknown in signature '" << s << "'";
        return false;
        break;
      }
      current++;
      arguments++;
      if (current >= s.size() || s[current] == closing)
        break;
      if (s[current] == '<')
      {
        int count = 0;
        while (current < s.size())
        {
          if (s[current] == '<')
            ++count;
          else if (s[current] == '>')
            --count;
          ++current;
          if (!count)
            break;
        }
        if (count)
        {
          qiLogVerbose() << "Annotation not closed in '" << s << "'";
          return false;
        }
      }

    }

    // Check complex type validity
    if (type == qi::Signature::Type_Map && (arguments != 2 || s[current] != qi::Signature::Type_Map_End))
    {
      qiLogVerbose() << "Map must have a key and a value.";
      return false;
    }
    if (type == qi::Signature::Type_List && (arguments != 1 || s[current] != qi::Signature::Type_List_End))
    {
      qiLogVerbose() << "List must contain only one element, but has " << arguments;
      return false;
    }
    return true;
  }

  static const char* _find_begin(const char* current, const char* start, char open, char close)
  {
    int count = 0; // start at 0
    while (current >= start)
    { // update count with current before testing
      if (*current == close)
        ++count;
      else if (*current == open)
        --count;
      if (!count)
        return current;
      --current;
    }
    return 0;
  }

  static int _find_end(char **pcurrent, const char **psignature, const char *sigend, char copen, char close)
  {
    int         opencount  = 1;
    int         closecount = 0;
    char       *current    = *pcurrent;
    const char *signature  = *psignature;

    *current = *signature;
    signature++;
    current++;
    while ((signature < sigend) && (opencount != closecount))
    {
      if (*signature == copen)
        opencount++;
      if (*signature == close)
        closecount++;
      *current = *signature;
      signature++;
      current++;
    }
    *pcurrent   = current;
    *psignature = signature;

    return opencount != closecount ? 0 : 1;
  }


  class SignaturePrivate {
  public:
    SignaturePrivate()
      : _signature(0),
        _end(0),
        _valid(false)
    {
    }

    ~SignaturePrivate() {
      delete [] _signature;
    }

    bool split(const char *, const char *);
    void init(const char *signature, size_t len);

    char          *_signature;
    const char    *_end;
    bool           _valid;
    std::string    _fsig;
  };

  void SignaturePrivate::init(const char *signature, size_t len) {
    if (!signature) {
      _valid = false;
      return;
    }
    //worst case: each char is a POD, we need 2x space (for \0)
    size_t size = len * 2;

    if (_signature)
      delete[] _signature;
    if (size == 0)
      size = 1;
    _signature = new char[size];
    if (size == 1)
      _signature[0] = 0;
    _end = _signature + size;
    _valid = split(signature, signature + len);
    if (_valid)
      _fsig = std::string(signature, len);
  }


  // go forward, add a 0, go forward, add a 0, bouhhh a 1! AHHHHHH scary!
  bool SignaturePrivate::split(const char *signature, const char *sig_end) {
    unsigned int i = 0;
    if (!_is_valid(std::string(signature, sig_end - signature), i, qi::Signature::Type_None, qi::Signature::Type_None))
      return false;

    char *current   = _signature;
    while(*signature)
    {
      if (signature >= sig_end || _signature >= _end)
        break;
      //verify that the current signature is correct
      switch(static_cast<qi::Signature::Type>(*signature))
      {
        case qi::Signature::Type_Void:
        case qi::Signature::Type_Bool:
        case qi::Signature::Type_Int8:
        case qi::Signature::Type_UInt8:
        case qi::Signature::Type_Int16:
        case qi::Signature::Type_UInt16:
        case qi::Signature::Type_Int32:
        case qi::Signature::Type_UInt32:
        case qi::Signature::Type_Int64:
        case qi::Signature::Type_UInt64:
        case qi::Signature::Type_Float:
        case qi::Signature::Type_Double:
        case qi::Signature::Type_String:
        case qi::Signature::Type_Dynamic:
        case qi::Signature::Type_Raw:
        case qi::Signature::Type_Object:
        case qi::Signature::Type_Unknown:
        case qi::Signature::Type_None:
          *current = *signature;
          current++;
          signature++;
          break;
        case qi::Signature::Type_List:
          if (!_find_end(&current, &signature, sig_end, '[', ']'))
            return false;
          break;
        case qi::Signature::Type_Map:
          if (!_find_end(&current, &signature, sig_end, '{', '}'))
            return false;
          break;
        case qi::Signature::Type_Tuple:
          if (!_find_end(&current, &signature, sig_end, '(', ')'))
            return false;
          break;
        default:
          qiLogVerbose() << "Signature element is invalid: '" << signature << "'";
          return false;
      }

      while (*signature == '*') { // dead code?
        *current = *signature;
        current++;
        signature++;
      }
      // handle annotation
      if (signature < sig_end && *signature == '<')
      {
        if (!_find_end(&current, &signature, sig_end, '<', '>'))
          return false;
      }

      *current = 0;
      current++;
      //std::cout << "elt:" << prev << std::endl;
    }
    if (current != _signature)
      _end = current - 1;
    else
      _end = _signature;
    return true;
  };



  Signature::Signature(const char *fullSignature)
    : _p(boost::make_shared<SignaturePrivate>())
  {
    if (!fullSignature)
      return;
    size_t size = strlen(fullSignature);
    _p->init(fullSignature, size);

    if (this->size() != 1)
    {
      qiLogVerbose() << "Signature has more than one element: '" << fullSignature << "'";
      _p->_valid = false;
    }
  }

  Signature::Signature(const std::string &subsig)
    : _p(boost::make_shared<SignaturePrivate>())
  {
    _p->init(subsig.c_str(), subsig.size());

    if (this->size() != 1)
    {
      qiLogVerbose() << "Signature has more than one element: '" << subsig << "'";
      _p->_valid = false;
    }
  }

  bool Signature::isValid() const {
    return _p->_valid;
  }

  const std::string& Signature::toString() const {
    return _p->_fsig;
  }

  Signature::iterator Signature::begin() const {
    if (!isValid())
      return end();
    if (_p->_signature == _p->_end)
      return ::qi::Signature::iterator();
    ::qi::Signature::iterator it(_p->_signature, _p->_end);
    return it;
  };

  Signature::iterator Signature::end() const {
    ::qi::Signature::iterator it;
    return it;
  };

  unsigned int Signature::size() const {
    unsigned int res = 0;
    for (iterator i = begin(); i != end(); ++i, ++res)
      ;
    return res;
  }


  ::qi::Signature::iterator &Signature::iterator::operator++() {
    if (!_current)
      return *this;
    //go to next \0
    while (*_current && (_current <= _end)) {
      ++_current;
    }
    //eat one more
    if (!*_current)
      _current++;
    if (_current >= _end)
      _current = 0;
    return *this;
  }

  ::qi::Signature::iterator &Signature::iterator::operator++(int) {
    this->operator++();
    return *this;
  }

  Signature::Type Signature::iterator::type()const {
    if (!_current)
      return Type_None;
    return static_cast<Type>(*_current);
  }

  Signature Signature::iterator::signature()const {
    if (!_current)
      return Signature();
    return Signature(_current);
  }

  std::string Signature::iterator::annotation()const {
    if (!_current)
      return std::string();
    // Since we have an end marker, use it, it will simplify annotation lookup
    const char* next = _current;
    while (*next && next <= _end)
      ++next;
    --next; // last caracter of this element
    if (*next != '>')
      return std::string();
    const char* astart = _find_begin(next, _current, '<', '>');
    if (!astart)
      return std::string();
    return std::string(astart + 1, next);
  }

  bool Signature::iterator::hasChildren() const {
    if (!_current)
      return false;

    switch (*_current) {
      case '[':
      case '{':
      case '(':
      case '@':
        return true;
      default:
       return false;
    };
  }

  Signature Signature::iterator::children() const {
    if (!_current)
      return Signature();
    Signature sig;
    size_t    size;
    size_t    toremove = 0;

    const char *fullSignature = _current;
    if (!fullSignature)
      return sig;

    size = strlen(fullSignature);

    if (size < 2)
      return sig;
    // remove annotation
    if (fullSignature[size - 1] == '>')
    {
      const char* astart = _find_begin(fullSignature + size - 1, _current, '<', '>');
      size = astart - fullSignature;
    }
    while (toremove <= (size - 2)) {
      if (fullSignature[size - 1 - toremove] != '*')
        break;
      toremove++;
    }
    sig._p->init(fullSignature + 1, size - toremove - 2);
    return sig;
  }

  std::string Signature::toPrettySignature() const {
    if (!isValid())
      return std::string();
    SignatureConvertor sc(this);
    return sc.signature();
  }

  std::vector<std::string> signatureSplit(const std::string &fullSignature) {
    std::vector<std::string> ret;
    std::string retSig;
    std::string parSig;
    std::string funcName;

    size_t idx1 = fullSignature.find("::");
    if (idx1 != fullSignature.npos) {
      // If :: are given, expect parameter tuple.
      if (fullSignature.find("(") == fullSignature.npos || fullSignature.find(")") == fullSignature.npos)
        throw std::runtime_error("Signature " + fullSignature + " is not valid");

      funcName = fullSignature.substr(0, idx1);
      // we should have a valid signature
      qi::Signature parent("(" + fullSignature.substr(idx1+2) + ")");
      qi::Signature sig = parent.begin().children();

      // Expect valid signatures.
      if (fullSignature.substr(idx1+2) == "" || parent.isValid() == false || sig.isValid() == false)
        throw std::runtime_error("Signature " + fullSignature + " is not valid");

      if (sig.isValid() && sig.size() == 2)
      {
        qi::Signature::iterator it = sig.begin();
        retSig = it.signature().toString();
        ++it;
        parSig = it.signature().toString();
      }
      else if (sig.isValid() && sig.size() == 1)
        parSig = sig.begin().signature().toString();
    } else {
      funcName = fullSignature;
    }
    ret.push_back(retSig);
    ret.push_back(funcName);
    ret.push_back(parSig);
    return ret;
  }

  static GenericValue signatureToData(Signature::iterator it)
  {
    std::vector<GenericValue> res;
    std::string t;
    t += (char)it.type();
    res.push_back(GenericValue::from(t));
    if (it.hasChildren())
      res.push_back(it.children().toData());
    else
      res.push_back(GenericValue::from(std::vector<GenericValue>()));
    res.push_back(GenericValue::from(it.annotation()));
    return GenericValue::from(res);
  }

  GenericValue Signature::toData() const
  {
    std::vector<GenericValue> res;
    if (!isValid())
      return GenericValue::from(res);
    for (Signature::iterator it = begin(); it != end(); ++it)
      res.push_back(signatureToData(it));
    return GenericValue::from(res);
  }
}

char* signature_to_json(const char* sig)
{
  static char* resc = 0;
  qi::Signature s(sig);
  std::string res = qi::encodeJSON(s.toData());
  if (resc)
    free(resc);
  resc = strdup(res.c_str());
  return resc;
}

QI_EQUIVALENT_STRING_REGISTER(qi::Signature, &qi::Signature::toString);
