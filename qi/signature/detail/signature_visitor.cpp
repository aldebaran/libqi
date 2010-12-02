#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <qi/log.hpp>
#include <qi/signature/detail/signature_visitor.hpp>

namespace qi {

  namespace detail {



    SignatureVisitor::SignatureVisitor(const char *signature, std::string &result)
      : _result(result),
        _current(signature),
        _signature(signature)
    {

    }

    void SignatureVisitor::visit(const char *sep) {
      char first = 1;
      while(*_current != 0)
      {
        const char *prev = _current;
        if (!first)
          _result += sep;
        first = 0;

        visitSingle();

        //verify something has been eaten
        if (prev == _current) {
          qisWarning << "SignatureVisitor parser error" << std::endl;
          break;
        }
      }
    }

    void SignatureVisitor::visitSingle() {
      switch(*_current) {
      case '[':
        visitList();
        break;
      case '{':
        visitMap();
        break;
      case ':':
        visitFunctionArguments();
        break;
      case '@':
        visitProtobuf();
        break;
      default:
        visitSimple();
        break;
      }

      //pointer is just an extra qualifer over a single type
      if (*_current == '*') {
        _result += "*";
        _current++;
      }


    }

    void SignatureVisitor::visitSimple() {
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

    void SignatureVisitor::visitFunctionArguments() {
      _result += "(";
      _current++;
      visit(", ");
      _result += ")";
    }

    void SignatureVisitor::visitList() {
      _result += "vector<";
      _current++;
      visitSingle();
      _result += ">";
      _current++;
    }

    void SignatureVisitor::visitMap() {
      _result += "map<";
      _current++;
      visitSingle();
      _result += ", ";
      visitSingle();
      _result += ">";
      _current++;
    }

    void SignatureVisitor::visitProtobuf() {
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
