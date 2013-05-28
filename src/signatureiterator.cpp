/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <boost/make_shared.hpp>

#include <qitype/signature.hpp>
#include <qi/log.hpp>
#include "signatureconvertor.hpp"

qiLogCategory("qitype.signature");

namespace qi {

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
        qiLogError() << "`" << s[current] << "' : Type unknown in `" << s << "'";
        return false;
        break;
      }
      current++;
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
          qiLogError() << "Annotation not closed in '" << s << "'";
          return false;
        }
      }
      arguments++;
    }

    // Check complex type validity
    if (type == qi::Signature::Type_Map && (arguments != 2 || s[current] != qi::Signature::Type_Map_End))
    {
      qiLogError() << "Map must have a key and a value.";
      return false;
    }
    if (type == qi::Signature::Type_List && (arguments != 1 || s[current] != qi::Signature::Type_List_End))
    {
      qiLogError() << "List must contain only one element, but has " << arguments;
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
    if (!_is_valid(std::string(signature), i, qi::Signature::Type_None, qi::Signature::Type_None))
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
          qiLogError() << "Signature element is invalid: `" << signature << "'";
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
      qiLogError() << "Signature has more than one element: `" << fullSignature << "'";
      _p->_valid = false;
    }
  }

  Signature::Signature(const std::string &subsig)
    : _p(boost::make_shared<SignaturePrivate>())
  {
    _p->init(subsig.c_str(), subsig.size());

    if (this->size() != 1)
    {
      qiLogError() << "Signature has more than one element: `" << subsig << "'";
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

  std::string Signature::iterator::signature()const {
    if (!_current)
      return std::string();
    return std::string(_current);
  }

  std::string Signature::iterator::annotation()const {
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

  std::string Signature::toSTLSignature(bool constify) const {
    SignatureConvertor sc(this, SignatureConvertor::STL, constify);
    return sc.signature();
  }

  std::string Signature::toQtSignature(bool constify) const {
    SignatureConvertor sc(this, SignatureConvertor::Qt, constify);
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
      //we should have a valid signature
      qi::Signature parent("(" + fullSignature.substr(idx1+2) + ")");
      qi::Signature sig = parent.begin().children();

      // Expect valid signatures.
      if (fullSignature.substr(idx1+2) == "" || parent.isValid() == false || sig.isValid() == false)
        throw std::runtime_error("Signature " + fullSignature + " is not valid");

      if (sig.isValid() && sig.size() == 2)
      {
        qi::Signature::iterator it = sig.begin();
        retSig = it.signature();
        ++it;
        parSig = it.signature();
      }
      else if (sig.isValid() && sig.size() == 1)
        parSig = sig.begin().signature();
    } else {
      funcName = fullSignature;
    }
    ret.push_back(retSig);
    ret.push_back(funcName);
    ret.push_back(parSig);
    return ret;
  }


}

