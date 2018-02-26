#pragma once
/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_MESSAGING_AUTHPROVIDER_P_HPP_
#define _SRC_MESSAGING_AUTHPROVIDER_P_HPP_

#include <qi/api.hpp>
#include <qi/messaging/authprovider.hpp>
#include <qi/messaging/authproviderfactory.hpp>

namespace qi
{

  class QI_API NullAuthProviderFactory : public AuthProviderFactory
  {
  public:
    virtual ~NullAuthProviderFactory() {}
    virtual AuthProviderPtr newProvider();
  };

  class QI_API NullAuthProvider : public AuthProvider
  {
  public:
    virtual ~NullAuthProvider() {}
  protected:
    virtual std::map<std::string, AnyValue> _processAuth(const std::map<std::string, AnyValue> &authData);
  };

}

QI_TYPE_ENUM(qi::AuthProvider::State);
#endif
