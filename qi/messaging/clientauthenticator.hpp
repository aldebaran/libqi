#pragma once
/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_MESSAGING_CLIENTAUTHENTICATOR_HPP_
#define _QI_MESSAGING_CLIENTAUTHENTICATOR_HPP_

#include <boost/shared_ptr.hpp>

#include <qi/api.hpp>
#include <qi/anyvalue.hpp>

namespace qi
{
using CapabilityMap = std::map<std::string, AnyValue>;

class QI_API ClientAuthenticator
{
public:
  virtual ~ClientAuthenticator()
  {
  }

  virtual CapabilityMap initialAuthData();
  CapabilityMap processAuth(const CapabilityMap& authData);

protected:
  /**
   * @brief processAuth Processes an authentication message client side.
   * This can be used to process a challenge sent by the server, for example.
   * If the authentication is meant to be done in a single step (with `InitialAuthData`)
   * then it's not necessary to override this function.
   * @param authData authentication data sent by the remote server.
   * @return A map of data that the server will use in the authentication process.
   * For example, a resolved challenge.
   */
  virtual CapabilityMap _processAuth(const CapabilityMap& /*authData*/)
  {
    return CapabilityMap();
  }
};

using ClientAuthenticatorPtr = boost::shared_ptr<ClientAuthenticator>;
}

#endif
