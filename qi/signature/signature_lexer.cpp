/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <cstring>
#include <iostream>
#include <sstream>
#include <qi/signature/signature_lexer.hpp>


namespace qi {

  SignatureLexer::SignatureLexer(const char *signature)
    : _signature(signature),
    _current(signature)
  {
  }

  SignatureLexer::Element SignatureLexer::getNext() {
    Element elt;
    Element child;

    memset(&elt, 0, sizeof(Element));
    elt.signature = _current;

    switch (*_current) {
    case '[':
      eat(); //[
      child = getNext();
      elt.child_1 = child.signature;
      if (*_current != ']') {
        std::stringstream ss;
        ss << "signature bad format: " << _signature;
        ss << ", at " << _current - _signature << ": " << *_current << " should be ]" << std::endl;
        throw BadFormatException(ss.str());
      }
      eat(); //]
      break;

    case '{':
      eat(); //{
      child = getNext();
      elt.child_1 = child.signature;
      child = getNext();
      elt.child_2 = child.signature;
      if (*_current != '}') {
        std::stringstream ss;
        ss << "signature bad format: " << _signature;
        ss << ", at " << _current - _signature << ": " << *_current << " should be }" << std::endl;
        throw BadFormatException(ss.str());
      }
      eat(); //}
      break;

    case '@':
      eat(); //@
      while (*_current != '@' && *_current != 0) {
        _current++;
      }
      if (*_current != '@') {
        std::stringstream ss;
        ss << "signature bad format: " << _signature;
        ss << ", at " << _current - _signature << ": " << *_current << " should be @" << std::endl;
        throw BadFormatException(ss.str());
      }
      eat(); //@
      break;

    case 'b':
    case 'i':
    case 'f':
    case 'd':
    case 'v':
    case 's':
      eat();
      break;

    //this is the end of the signature
    case 0:
      return elt;

    default:
      std::stringstream ss;
      ss << "signature bad format: " << _signature;
      ss << ", at " << _current - _signature << std::endl;
      throw BadFormatException(ss.str());
      break;
    }


    if (*_current == '*') {
      elt.pointer = 1;
      _current++;
    }
    return elt;
  }


}
