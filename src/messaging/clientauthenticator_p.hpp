#pragma once
/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_MESSAGING_CLIENTAUTHENTICATOR_P_HPP_
#define _SRC_MESSAGING_CLIENTAUTHENTICATOR_P_HPP_

#include <qi/anyvalue.hpp>

#include <qi/messaging/clientauthenticator.hpp>
#include <qi/messaging/clientauthenticatorfactory.hpp>

namespace qi
{

  using CapabilityMap = std::map<std::string, AnyValue>;

  class QI_API NullClientAuthenticatorFactory : public ClientAuthenticatorFactory
  {
  public:
    virtual ~NullClientAuthenticatorFactory() {}
    virtual ClientAuthenticatorPtr newAuthenticator();
  };

  class QI_API NullClientAuthenticator : public ClientAuthenticator
  {
  public:
    virtual ~NullClientAuthenticator() {}
  protected:
    virtual CapabilityMap _processAuth(const CapabilityMap& authData);
  };

}

#endif
