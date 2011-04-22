/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#include <string>
#include <qi/signature/signature_to_string.hpp>
#include <qi/signature/detail/pretty_print_signature_visitor.hpp>

namespace qi {

  void signatureToString(const char *signature, std::string &result) {
    detail::PrettyPrintSignatureVisitor(signature, result).visit();
  }

  std::string signatureToString(const char *signature) {
    std::string result;
    detail::PrettyPrintSignatureVisitor(signature, result).visit();
    return result;
  }

  std::string signatureToString(const std::string& signature) {
    std::string result;
    detail::PrettyPrintSignatureVisitor(signature.c_str(), result).visit();
    return result;
  }



};
