#pragma once
/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_MESSAGING_AUTHPROVIDER_HPP_
#define _QI_MESSAGING_AUTHPROVIDER_HPP_

#include <string>
#include <map>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include <qi/api.hpp>
#include <qi/anyvalue.hpp>

namespace qi
{
using CapabilityMap = std::map<std::string, AnyValue>;

class QI_API AuthProvider
{
public:
  static const std::string State_Key;
  enum State
  {
    State_Error = 1,
    State_Cont,
    State_Done,
  };
  static const std::string Error_Reason_Key;
  static const std::string UserAuthPrefix;

  virtual ~AuthProvider()
  {
  }
  CapabilityMap processAuth(const CapabilityMap& authData);

protected:
  /*
  * Processes an authentication messages, then returns a map of values.
  * The returned map MUST include an element at index `State_Key`,
  * containing:
  * - `State_Error` if the authentication attempt is unsuccessful; if a
  * value exists at index `Error_Reason_Key` it will be sent to the client.
  * - `State_Cont` if the authentication attempt needs further processing;
  * the rest of the map will be sent to the client.
  * - `State_Done` if the authentication succeeded. The rest of the map will
  * be left unused.
  * If this token is missing or yields an unknown value the server will QI_ASSERT()
  * and terminate the client's connection.
  */
  virtual CapabilityMap _processAuth(const CapabilityMap& authData) = 0;
};

using AuthProviderPtr = boost::shared_ptr<AuthProvider>;
}

#endif
