/*
**  Copyright (C) 2015 Aldebaran Robotics
**  See COPYING for the license
*/

#pragma once

#ifndef _QI_MESSAGING_PAMAUTHPROVIDER_HPP_
#define _QI_MESSAGING_PAMAUTHPROVIDER_HPP_

#include <qi/messaging/authprovider.hpp>
#include <qi/messaging/authproviderfactory.hpp>

#include "pamauthapi.hpp"

namespace qi
{
class QIPAMAUTH_API PAMAuthProvider : public AuthProvider
{
protected:
  virtual CapabilityMap _processAuth(const CapabilityMap& authData);
};

class QIPAMAUTH_API PAMAuthProviderFactory : public AuthProviderFactory
{
public:
  virtual AuthProviderPtr newProvider();
};
}

#endif
