/*
**  Copyright (C) 2015 Aldebaran Robotics
**  See COPYING for the license
*/

#pragma once
#ifndef _SRC_MESSAGING_PAMCLIENTAUTHENTICATOR_HPP_
#define _SRC_MESSAGING_PAMCLIENTAUTHENTICATOR_HPP_

#include <qi/messaging/clientauthenticator.hpp>
#include <qi/messaging/clientauthenticatorfactory.hpp>

#include "pamauthapi.hpp"

namespace qi
{
class QIPAMAUTH_API PAMClientAuthenticator : public ClientAuthenticator
{
public:
  static const std::string PAM_USER_KEY;
  static const std::string PAM_PASSWORD_KEY;

  PAMClientAuthenticator(const std::string& user, const std::string& pass);
  ~PAMClientAuthenticator();

  virtual CapabilityMap initialAuthData();

protected:
  virtual CapabilityMap _processAuth(const CapabilityMap&);

private:
  CapabilityMap _identity;
};

class QIPAMAUTH_API PAMClientAuthenticatorFactory : public ClientAuthenticatorFactory
{
public:
  PAMClientAuthenticatorFactory(const std::string& user, const std::string& pass);
  ~PAMClientAuthenticatorFactory();

  virtual ClientAuthenticatorPtr newAuthenticator();

private:
  std::string _user;
  std::string _pass;
};
}

#endif
