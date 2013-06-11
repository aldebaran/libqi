/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>
#include <cstring>
#include <qi/log.hpp>
#include "signatureconvertor.hpp"
#include <qitype/signature.hpp>
#include <boost/algorithm/string.hpp>

namespace qi {

  SignatureConvertor::SignatureConvertor(const Signature *sig)
    : _done(false),
      _sig(sig)
  {}

  const std::string &SignatureConvertor::signature() {
    if (!_done) {
      visit(_sig);
      _done = true;
    }
    return _result;
  }

  void SignatureConvertor::visit(const qi::Signature *sig) {

    qi::Signature::iterator it;

    for (it = sig->begin(); it != sig->end(); ++it) {
      visitSingle(&it);
    }
  }

  void SignatureConvertor::visitSingle(qi::Signature::iterator *it) {
    switch(it->type()) {
      case '[':
        visitList(it);
        break;
      case '{':
        visitMap(it);
        break;
      case '(':
        visitTuple(it);
        break;
      default:
        visitSimple(it);
        break;
    }
  }

  void SignatureConvertor::visitSimple(qi::Signature::iterator *it) {
    switch(it->type()) {
      case 'b':
        _result += "Bool";
        break;
      case 'c':
        _result += "Int8";
        break;
      case 'C':
        _result += "UInt8";
        break;
      case 'i':
        _result += "Int32";
        break;
      case 'I':
        _result += "UInt32";
        break;
      case 'l':
        _result += "Int64";
        break;
      case 'L':
        _result += "UInt64";
      break;
      case 'f':
        _result += "Float";
        break;
      case 'd':
        _result += "Double";
        break;
      case 'v':
        _result += "Void";
        break;
      case 's':
        _result += "String";
        break;
      case 'm':
        _result += "Value";
        break;
      case 'o':
        _result += "Object";
        break;
      case 'X':
        _result += "Unknown";
        break;
      default:
        _result += "BUG";
        break;
    }
  }

  void SignatureConvertor::visitList(qi::Signature::iterator *it) {
    _result += "List<";
    qi::Signature sig = it->children();
    qi::Signature::iterator it2 = sig.begin();
    visitSingle(&it2);
    _result += ">";
  }

  void SignatureConvertor::visitMap(qi::Signature::iterator *it) {
    _result += "Map<";
    qi::Signature sig = it->children();
    qi::Signature::iterator it2 = sig.begin();
    visitSingle(&it2);
    _result += ",";
    ++it2;
    visitSingle(&it2);
    _result += ">";
  }

  //not really tuple but fonction arguments
  void SignatureConvertor::visitTuple(qi::Signature::iterator *it) {
    std::vector<std::string> annot;
    std::string annotation(it->annotation());
    boost::algorithm::split(annot, annotation, boost::algorithm::is_any_of(","));

    if (!annotation.empty() && !annot.empty()) {
      //the first elem is the name
      _result += annot[0];
      return;
    }
    _result += "(";
    qi::Signature sig = it->children();

    qi::Signature::iterator it2 = sig.begin();
    while (it2 != sig.end()) {
      visitSingle(&it2);
      ++it2;
      if (it2 != sig.end())
        _result += ",";
      else
        break;
    }
    _result += ")";
  }


};
