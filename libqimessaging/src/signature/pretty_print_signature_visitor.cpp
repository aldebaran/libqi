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
#include "pretty_print_signature_visitor.hpp"

namespace qi {

  enum PrivateQiSignatureType {
    QI_STRING     = 0,
    QI_LIST       = 1,
    QI_LIST_END   = 2,
    QI_MAP        = 3,
    QI_MAP_END    = 4,
  };

  static const char *gStl[] = {
    "string",
    "std::vector<",
    ">",
    "std::map<",
    ">"
  };

  static const char *gStlConst[] = {
    "const string &",
    "const std::vector<",
    "> &",
    "const std::map<",
    "> &"
  };

  static const char *gQt[] = {
    "QString",
    "QList<",
    ">",
    "QMap<",
    ">"
  };

  static const char *gQtConst[] = {
    "const QString &",
    "const QList<",
    "> &",
    "const QMap<",
    "> &"
  };


  PrettyPrintSignatureVisitor::PrettyPrintSignatureVisitor(const char *signature, SignatureType type)
    : _current(signature),
      _signature(signature),
      _method(""),
      _type(type),
      _done(false)
  {}


  const std::string &PrettyPrintSignatureVisitor::returnSignature() {
    visit();
    return _returnSig;
  }

  const std::string &PrettyPrintSignatureVisitor::functionSignature() {
    visit();
    return _result;
  }

  const char *PrettyPrintSignatureVisitor::elementTypeSTL(int idx)
  {
    if (_constify) {
      if (_type == Qt)
        return gQtConst[idx];
      if (_type == STL)
        return gStlConst[idx];
    } else {
      if (_type == Qt)
        return gQt[idx];
      if (_type == STL)
        return gStl[idx];
    }
    return 0;
  }

  void PrettyPrintSignatureVisitor::visit() {
    if (_done)
      return;
    _done = true;

    //do we have name::sig ?
    const char* sep = "::";
    const char* delimiter = strstr(_signature, sep);

    //not a func sig
    if (delimiter != NULL) {
      _method  = std::string(_signature, delimiter - _signature);
      _current = delimiter + 2;
      _constify = false;
      //return type
      visitSingle();
      _returnSig = _result;
      _constify = true;

      //reset result to old funcname and params
      _result = _method;
    }

    //eat an element
    visitSingle();

    //verify something has been eaten
    if (*_current != 0) {
      std::stringstream ss;
      ss << "trailing garbage(" << *_current << ") at index " << _current - _signature;
      ss << ", signature: " << _signature;
      throw qi::BadSignatureError(ss.str());
    }
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
          _result += elementTypeSTL(QI_STRING);
        break;
      default:
        _result += "ERRR";
        break;
    }
    _current++;
  }

  void PrettyPrintSignatureVisitor::visitList() {
    _result += elementTypeSTL(QI_LIST);
    _current++;
    visitSingle();
    _result += elementTypeSTL(QI_LIST_END);
    _current++;
  }

  void PrettyPrintSignatureVisitor::visitMap() {
    _result += elementTypeSTL(QI_MAP);
    _current++;
    visitSingle();
    _result += ", ";
    visitSingle();
    _result += elementTypeSTL(QI_MAP_END);
    _current++;
  }

  void PrettyPrintSignatureVisitor::visitTuple() {
    int first = 1;
    _result += "(";
    _current++;
    while (*_current != 0 && *_current != ')') {
      if (first)
        first = 0;
      else
        _result += ", ";
      visitSingle();
    }
    _result += ")";
    _current++;
  }


};
