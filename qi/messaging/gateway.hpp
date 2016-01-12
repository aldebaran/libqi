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
using AuthProviderFactoryPtr = boost::shared_ptr<AuthProviderFactory>;
class ClientAuthenticatorFactory;
using ClientAuthenticatorFactoryPtr = boost::shared_ptr<ClientAuthenticatorFactory>;
class GatewayPrivate;
using GatewayPrivatePtr = boost::shared_ptr<GatewayPrivate>;

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
  bool setIdentity(const std::string& key, const std::string& crt);
  qi::Future<void> attachToServiceDirectory(const Url& serviceDirectoryUrl);
  void close();
};
}

#endif // _QIMESSAGING_GATEWAY_HPP_
