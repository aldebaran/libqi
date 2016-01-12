#pragma once
/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_MESSAGING_CLIENTAUTHENTICATORFACTORY_HPP_
#define _QI_MESSAGING_CLIENTAUTHENTICATORFACTORY_HPP_

#include <boost/shared_ptr.hpp>

#include <qi/api.hpp>
#include <qi/messaging/clientauthenticator.hpp>

namespace qi
{

  class QI_API ClientAuthenticatorFactory
  {
  public:
    virtual ~ClientAuthenticatorFactory() {}
    virtual ClientAuthenticatorPtr newAuthenticator() = 0;
    virtual unsigned int authVersionMajor() { return 1; }
    virtual unsigned int authVersionMinor() { return 0; }
  };

  using ClientAuthenticatorFactoryPtr = boost::shared_ptr<ClientAuthenticatorFactory>;
}


#endif
