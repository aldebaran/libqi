#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SIGNATURE_SIGNATURE_TO_STRING_HPP_
#define _QI_SIGNATURE_SIGNATURE_TO_STRING_HPP_

#include <qi/api.hpp>
#include <string>

namespace qi {

  /// return a pretty printed a signature.
  /// \ingroup Signature
  /// \include example_qi_signature_pp.cpp
  QI_API void signatureToString(const char *signature, std::string &result);

  /// return a pretty printed a signature.
  /// \ingroup Signature
  QI_API std::string signatureToString(const char *signature);

  /// return a pretty printed a signature.
  /// \ingroup Signature
  QI_API std::string signatureToString(const std::string& signature);

};


#endif  // _QI_SIGNATURE_SIGNATURE_TO_STRING_HPP_
