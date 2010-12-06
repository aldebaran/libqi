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

namespace qi {

  void signatureToString(const char *signature, std::string &result);

  std::string signatureToString(const char *signature);

  std::string signatureToString(const std::string& signature);

};


#endif  // _QI_SIGNATURE_SIGNATURE_TO_STRING_HPP_
