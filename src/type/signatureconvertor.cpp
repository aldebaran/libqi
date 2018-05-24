/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <cstring>
#include <qi/log.hpp>
#include "signatureconvertor.hpp"
#include <qi/signature.hpp>
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
      case Signature::Type_List:
        visitList(sig);
        break;
      case Signature::Type_Map:
        visitMap(sig);
        break;
      case Signature::Type_Tuple:
        visitTuple(sig);
        break;
      case Signature::Type_VarArgs:
        visitVarArgs(sig);
        break;
      case Signature::Type_Optional:
        visitOptional(sig);
        break;
      default:
        visitSimple(sig);
        break;
    }
  }

  void SignatureConvertor::visitSimple(const Signature& sig) {
    switch(sig.type()) {
      case Signature::Type_Bool:
        _result += "Bool";
        break;
      case Signature::Type_Int8:
        _result += "Int8";
        break;
      case Signature::Type_UInt8:
        _result += "UInt8";
        break;
      case Signature::Type_Int16:
        _result += "Int16";
        break;
      case Signature::Type_UInt16:
        _result += "UInt16";
        break;
      case Signature::Type_Int32:
        _result += "Int32";
        break;
      case Signature::Type_UInt32:
        _result += "UInt32";
        break;
      case Signature::Type_Int64:
        _result += "Int64";
        break;
      case Signature::Type_UInt64:
        _result += "UInt64";
      break;
      case Signature::Type_Float:
        _result += "Float";
        break;
      case Signature::Type_Double:
        _result += "Double";
        break;
      case Signature::Type_Void:
        _result += "Void";
        break;
      case Signature::Type_String:
        _result += "String";
        break;
      case Signature::Type_Dynamic:
        _result += "Value";
        break;
      case Signature::Type_Object:
        _result += "Object";
        break;
      case Signature::Type_Unknown:
        _result += "Unknown";
        break;
      case Signature::Type_Raw:
        _result += "RawBuffer";
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

  void SignatureConvertor::visitVarArgs(const Signature& sig) {
    _result += "VarArgs<";
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

  void SignatureConvertor::visitOptional(const Signature& sig)
  {
    _result += "Optional<";
    visit(sig.children().at(0));
    _result += ">";
  }

};
