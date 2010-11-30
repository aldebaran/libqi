/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/signature/detail/signature_visitor.hpp>

namespace qi {

  void signatureToString(const char *signature, std::string &result) {
    detail::SignatureVisitor(signature, result).visit();
  }

  std::string signatureToString(const char *signature) {
    std::string result;
    detail::SignatureVisitor(signature, result).visit();
    return result;
  }



};
