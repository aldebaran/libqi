/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#include <string>
#include <qimessaging/signature.hpp>
#include "pretty_print_signature_visitor.hpp"

namespace qi {

  void signatureToString(const char *signature, std::string &result) {
    PrettyPrintSignatureVisitor ppsv(signature);
    result = ppsv.returnSignature() + " " + ppsv.functionSignature();
  }

  std::string signatureToString(const char *signature) {
    std::string result;
    PrettyPrintSignatureVisitor ppsv(signature);
    result = ppsv.returnSignature() + " " + ppsv.functionSignature();
    return result;
  }

  std::string signatureToString(const std::string& signature) {
    std::string result;
    PrettyPrintSignatureVisitor ppsv(signature.c_str());
    result = ppsv.returnSignature() + " " + ppsv.functionSignature();
    return result;
  }

};
