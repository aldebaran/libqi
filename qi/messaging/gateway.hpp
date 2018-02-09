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

class QI_API Gateway
{
  class Impl;
  std::unique_ptr<Impl> _p;
public:
  enum class IdValidationStatus
  {
    Valid,                ///< The provided identity is stored and checked as valid.
    Invalid,              ///< The provided identity is invalid.
    PendingCheckOnListen, ///< The provided identity is stored and will be checked on next listen.
  };

  /**
   * @param enforceAuth If set to true, reject clients that try to skip the authentication step. If false, accept all
   * incoming connections whether or not they authentify.
   */
  Gateway(bool enforceAuth = false);
  ~Gateway();

  Property<bool>& connected;

  UrlVector endpoints() const;
  bool listen(const Url& url);

  QI_API_DEPRECATED_MSG("Use setValidateIdentity() instead.")
  bool setIdentity(const std::string& key, const std::string& crt);

  Future<IdValidationStatus> setValidateIdentity(const std::string& key, const std::string& crt);
  qi::Future<void> attachToServiceDirectory(const Url& serviceDirectoryUrl);
  void close();
};
}

#endif // _QIMESSAGING_GATEWAY_HPP_
