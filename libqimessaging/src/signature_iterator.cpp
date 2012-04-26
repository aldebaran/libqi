/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <qimessaging/signature.hpp>
#include "src/signature_convertor.hpp"

namespace qi {

  //TODO
  static bool _is_valid(const char *begin, const char *QI_UNUSED(end)) {
    const char *current = begin;

    while (*current) {
      current++;
    };
    return true;
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

    return 0;
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
  };

  void SignaturePrivate::init(const char *signature, size_t len) {
    if (!signature) {
      _valid = false;
      return;
    }
    //worst case: each char is a POD, we need 2x space (for \0)
    size_t size = len * 2;

    if (_signature)
      free(_signature);
    if (size == 0)
      size = 1;
    _signature = new char[size];
    if (size == 1)
      _signature[0] = 0;
    _end = _signature + size;
    _valid = split(signature, signature + len);
  }


  // go forward, add a 0, go forward, add a 0, bouhhh a 1! AHHHHHH scary!
  bool SignaturePrivate::split(const char *signature, const char *sig_end) {

    if (!_is_valid(signature, sig_end))
      return false;

    //std::cout << "fullsig:" << signature << std::endl;
    char *current   = _signature;
    //char *signature = _signature;

    while(*signature) {
      if (signature >= sig_end || _signature >= _end)
        break;
      //verify that the current signature is correct
      switch(static_cast<qi::Signature::Type>(*signature)) {
        case qi::Signature::Void:
        case qi::Signature::Bool:
        case qi::Signature::Char:
        case qi::Signature::Int:
        case qi::Signature::Float:
        case qi::Signature::Double:
        case qi::Signature::String:
        case 'X':
          *current = *signature;
          current++;
          signature++;
          break;
        case qi::Signature::List:
          _find_end(&current, &signature, sig_end, '[', ']');
          break;
        case qi::Signature::Map:
          _find_end(&current, &signature, sig_end, '{', '}');
          break;
        case qi::Signature::Tuple:
          _find_end(&current, &signature, sig_end, '(', ')');
          break;
        default:
          qiLogError("qimessaging.Signature") << "Signature is invalid:" << signature;
          return false;
      }

      while (*signature == '*') {
        *current = *signature;
        current++;
        signature++;
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
    : _p(boost::shared_ptr<SignaturePrivate>(new SignaturePrivate()))
  {
    if (!fullSignature)
      return;
    size_t size = strlen(fullSignature);
    _p->init(fullSignature, size);
  }

  Signature::Signature(const std::string &subsig)
    : _p(boost::shared_ptr<SignaturePrivate>(new SignaturePrivate()))
  {
    _p->init(subsig.c_str(), subsig.size());
  }

  bool Signature::isValid() const {
    return _p->_valid;
  }

  std::string Signature::toString() const {
    char *current = _p->_signature;
    std::string ret;

    //yeah kinda too big, but it's ok
    ret.reserve(_p->_end - _p->_signature);
    while (current != _p->_end) {
      if (*current)
        ret += *current;
      current++;
    }
    return ret;
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
      return None;
    return static_cast<Type>(*_current);
  }

  std::string Signature::iterator::signature()const {
    if (!_current)
      return std::string();
    return std::string(_current);
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

  int Signature::iterator::pointer() const {
    int count = 0;
    if (!_current)
      return false;

    const char *prev = _current;
    const char *next = _current;
    while (*next) {
      prev = next;
      next++;
    }
    while (*prev == '*') {
      count++;
      prev--;
      if (prev < _current)
        break;
    }
    return count;
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
    if (idx1 != fullSignature.npos)
      funcName = fullSignature.substr(0, idx1);
    size_t idx2 = fullSignature.find("(", idx1);
    if (idx2 != fullSignature.npos) {
      retSig = fullSignature.substr(idx1 + 2, idx2 - idx1 - 2);
      parSig = fullSignature.substr(idx2);
    }

    ret.push_back(retSig);
    ret.push_back(funcName);
    ret.push_back(parSig);
    return ret;
  }


}

