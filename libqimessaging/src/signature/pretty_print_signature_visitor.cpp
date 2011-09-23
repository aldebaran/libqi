/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <cstring>
#include <qi/log.hpp>
#include <qimessaging/signature/error.hpp>
#include <qimessaging/signature/detail/pretty_print_signature_visitor.hpp>

namespace qi {
  namespace detail {

    PrettyPrintSignatureVisitor::PrettyPrintSignatureVisitor(const char *signature, std::string &result)
      : _result(result),
        _current(signature),
        _signature(signature),
        _method("")
    {}

    void PrettyPrintSignatureVisitor::visit(const char *sep) {

      //do we have name::sig ?
      if (_current == _signature)
        if (visitFunction())
          return;

      //eat an element
      visitSingle();

      //this is a function pointer
      if (*_current == '(') {
        _result += " ";
        visitTuple(true);
      }

      //verify something has been eaten
      if (*_current != 0) {
        std::stringstream ss;
        ss << "trailing garbage(" << *_current << ") at index " << _current - _signature;
        ss << ", signature: " << _signature;
        throw qi::BadSignatureError(ss.str());
      }
    }

    //true if match
    bool PrettyPrintSignatureVisitor::visitFunction() {
      const char* sep = "::";
      const char* delimiter = strstr(_signature, sep);
      if (delimiter == NULL)
        return false;
      _method  = std::string(_signature, delimiter - _signature);
      _current = delimiter + 2;
      //return type
      visitSingle();

      _result += " ";
      _result += _method;
      visitTuple(true);
      //_result += "(";
      //_current++;
      //visit(", ");
      //_result += ")";
      return true;
    }

    void PrettyPrintSignatureVisitor::visitSingle() {
      switch(*_current) {
        case '[':
          visitList();
          break;
        case '{':
          visitMap();
          break;
        case '(':
          visitTuple();
          break;
        case '@':
          visitProtobuf();
          break;
        default:
          visitSimple();
          break;
      }

      //pointer is just an extra qualifier over a single type
      if (*_current == '*') {
        _result += "*";
        _current++;
      }
    }

    void PrettyPrintSignatureVisitor::visitSimple() {
      switch(*_current) {
      case 'b':
        _result += "bool";
        break;
      case 'i':
        _result += "int";
        break;
      case 'f':
        _result += "float";
        break;
      case 'd':
        _result += "double";
        break;
      case 'v':
        _result += "void";
        break;
      case 's':
        _result += "string";
        break;
      default:
        _result += "ERRR";
        break;
      }
      _current++;
    }

    void PrettyPrintSignatureVisitor::visitTuple(bool param) {
      int first = 1;
      if (param)
        _result += "(";
      else
        _result += "tuple<";
      _current++;
      while (*_current != 0 && *_current != ')') {
        if (first)
          first = 0;
        else
          _result += ", ";
        visitSingle();
      }
      //visit(", ");
      if (param)
        _result += ")";
      else
        _result += ">";
      _current++;
    }

    void PrettyPrintSignatureVisitor::visitList() {
      _result += "vector<";
      _current++;
      visitSingle();
      _result += ">";
      _current++;
    }

    void PrettyPrintSignatureVisitor::visitMap() {
      _result += "map<";
      _current++;
      visitSingle();
      _result += ", ";
      visitSingle();
      _result += ">";
      _current++;
    }

    void PrettyPrintSignatureVisitor::visitProtobuf() {
      _result += "Protobuf(";
      _current++;

      const char *start = _current;
      while (*_current != '@' && *_current != 0) {
        _current++;
      }

      std::string name(start, _current - start);

      _result += name + ")";
      _current++;
    }
  }
};
