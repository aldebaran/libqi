/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_SIGNATURE_SIGNATURE_TO_STRING_HPP__
#define   __QI_SIGNATURE_SIGNATURE_TO_STRING_HPP__

namespace qi {

  void signatureToString(const char *signature, std::string &result);

  std::string signatureToString(const char *signature);

};


#endif // __QI_SIGNATURE_SIGNATURE_TO_STRING_HPP__
