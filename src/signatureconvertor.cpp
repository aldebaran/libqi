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
      visit(*_sig);
      _done = true;
    }
    return _result;
  }

  void SignatureConvertor::visit(const qi::SignatureVector& elements) {

    SignatureVector::const_iterator it;

    for (it = elements.begin(); it != elements.end(); ++it) {
      visit(*it);
    }
  }

  void SignatureConvertor::visit(const qi::Signature& sig) {
    switch(sig.type()) {
      case '[':
        visitList(sig);
        break;
      case '{':
        visitMap(sig);
        break;
      case '(':
        visitTuple(sig);
        break;
      default:
        visitSimple(sig);
        break;
    }
  }

  void SignatureConvertor::visitSimple(const Signature& sig) {
    switch(sig.type()) {
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

  void SignatureConvertor::visitList(const Signature& sig) {
    _result += "List<";
    visit(sig.children().at(0));
    _result += ">";
  }

  void SignatureConvertor::visitMap(const Signature& sig) {
    _result += "Map<";
    visit(sig.children().at(0));
    _result += ",";
    visit(sig.children().at(1));
    _result += ">";
  }

  //not really tuple but fonction arguments
  void SignatureConvertor::visitTuple(const Signature &sig) {
    std::vector<std::string> annot;
    std::string annotation(sig.annotation());
    boost::algorithm::split(annot, annotation, boost::algorithm::is_any_of(","));

    if (!annotation.empty() && !annot.empty()) {
      //the first elem is the name
      _result += annot[0];
      return;
    }
    _result += "(";
    const qi::SignatureVector& csig = sig.children();

    qi::SignatureVector::const_iterator it = csig.begin();
    while (it != csig.end()) {
      visit(*it);
      ++it;
      if (it != csig.end())
        _result += ",";
      else
        break;
    }
    _result += ")";
  }


};
