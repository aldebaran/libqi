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
#include "src/signature/pretty_print_signature_visitor.hpp"

namespace qi {

  static bool _is_valid(const char *begin, const char *end) {
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

    _signature = (char *)malloc(size);
    _end = _signature + size;
    _valid = split(signature, signature + size);
  }


  // go forward, add a 0, go forward, add a 0, bouhhh a 1! AHHHHHH scary!
  bool SignaturePrivate::split(const char *signature, const char *sig_end) {

    if (!_is_valid(signature, sig_end))
      return false;

    //std::cout << "fullsig:" << signature << std::endl;
    char *current   = _signature;
    //char *signature = _signature;

    const char *prev = current;
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
          *current = *signature;
          current++;
          signature++;
          break;
        case qi::Signature::List:
          _find_end(&current, &signature, sig_end, '[', ']');
          break;
        case qi::Signature::Map:
          _find_end(&current, &signature, sig_end, '[', ']');
          break;
        default:
          return false;
      }

      if (*signature == '*') {
        *current = *signature;
        current++;
        signature++;
      }


      *current = 0;
      current++;
      //std::cout << "elt:" << prev << std::endl;
      prev = current;
    }
    _end = current - 1;
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

  ::qi::Signature::iterator Signature::begin() const {
    ::qi::Signature::iterator it(_p->_signature, _p->_end);
    return it;
  };

  ::qi::Signature::iterator Signature::end() const {
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
    this->operator ++();
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
      case '@':
        return true;
      default:
       return false;
    };
  }

  bool Signature::iterator::isPointer() const {
    if (!_current)
      return false;

    const char *prev = _current;
    const char *next = _current;
    while (*next) {
      prev = next;
      next++;
    }
    return *prev == '*';
  }

  Signature Signature::iterator::children() const {
    Signature sig;
    size_t    size;
    size_t    toremove = 0;

    const char *fullSignature = _current;
    if (!fullSignature)
      return sig;

    size = strlen(fullSignature);

    while (toremove < size) {
      if (fullSignature[size - 1 - toremove] != '*')
        break;
      toremove++;
    }
    sig._p->init(fullSignature + 1, size - toremove - 1);
    return sig;
  }


  //  std::string Signature::toSTL() {
//    //PrettyPrintSignatureVisitor ppsv(_signature, PrettyPrintSignatureVisitor::STL);
//    //return ppsv.fullSignature();
//  }

//  std::string Signature::toQT() {
//    //PrettyPrintSignatureVisitor ppsv(_signature, PrettyPrintSignatureVisitor::Qt);
//    //return ppsv.fullSignature();
//  }

}

