#pragma once
/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_GATEWAY_HPP_
#define _QIMESSAGING_GATEWAY_HPP_

#include <boost/shared_ptr.hpp>

#include <qi/api.hpp>
#include <qi/property.hpp>
#include <qi/future.hpp>
#include <qi/url.hpp>
#include <qi/anyvalue.hpp>
#include <qi/signal.hpp>

namespace qi
{
class AuthProviderFactory;
typedef boost::shared_ptr<AuthProviderFactory> AuthProviderFactoryPtr;
class ClientAuthenticatorFactory;
typedef boost::shared_ptr<ClientAuthenticatorFactory> ClientAuthenticatorFactoryPtr;
class GatewayPrivate;
typedef boost::shared_ptr<GatewayPrivate> GatewayPrivatePtr;

class QI_API Gateway
{
  GatewayPrivatePtr _p;
public:
  /**
   * @param enforceAuth If set to true, reject clients that try to skip the authentication step. If false, accept all
   * incoming connections whether or not they authentify.
   */
  Gateway(bool enforceAuth = false);
  ~Gateway();

  Property<bool>& connected;

  void setAuthProviderFactory(AuthProviderFactoryPtr provider);
  void setLocalClientAuthProviderFactory(AuthProviderFactoryPtr provider);
  void setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr authenticator);
  UrlVector endpoints() const;
  bool listen(const Url& url);
  qi::Future<void> attachToServiceDirectory(const Url& serviceDirectoryUrl);
  void close();
};
}

#endif // _QIMESSAGING_GATEWAY_HPP_
