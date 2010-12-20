/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <cstring>
#include <iostream>
#include <sstream>
#include <qi/signature/signature_iterator.hpp>


namespace qi {

  Signature::iterator &Signature::iterator::next() {
    const char *_current_signature = _current;

    //reinit ourself
    child_1   = 0;
    child_2   = 0;
    pointer   = 0;
    signature = 0;

    switch (*_current) {
    case '[':
      {
        eat(); //[
        //small recursion into ourself, dont be afraid, we stored _current!
//        next();
//        child_1 = signature;
        Signature s1(_current);
        Signature::iterator it1 = s1.begin();
        child_1 = it1.signature;
        _current = it1._current;
        if (*_current != ']') {
          std::stringstream ss;
          ss << "signature bad format: " << _signature;
          ss << ", at " << _current - _signature << ": " << *_current << " should be ]" << std::endl;
          throw BadFormatException(ss.str());
        }
        eat(); //]
      }
      break;

    case '{':
      {
        eat(); //{
        Signature s1(_current);
        Signature::iterator it1 = s1.begin();
        child_1 = it1.signature;
        _current = it1._current;
      //small recursion into ourself, dont be afraid, we stored _current!
      //next();
        //      child_1 = signature;
        Signature s2(_current);
        Signature::iterator it2 = s2.begin();
        child_2 = it2.signature;
        _current = it2._current;
        //      //small recursion into ourself, dont be afraid, we stored _current!
        //      next();
        //      child_2 = signature;

        if (*_current != '}') {
          std::stringstream ss;
          ss << "signature bad format: " << _signature;
          ss << ", at " << _current - _signature << ": " << *_current << " should be }" << std::endl;
          throw BadFormatException(ss.str());
        }
        eat(); //}
      }
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
      eat(); //eat 0
      break;

    default:
      std::stringstream ss;
      ss << "signature bad format: " << _signature;
      ss << ", at " << _current - _signature << std::endl;
      throw BadFormatException(ss.str());
      break;
    }

    if (*_current == '*') {
      pointer = 1;
      eat();
    }
    signature = _current_signature;
    return *this;
  }

}

