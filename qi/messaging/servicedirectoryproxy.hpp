#pragma once
/*
**  Copyright (C) 2018 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SERVICEDIRECTORYPROXY_HPP_
#define _QIMESSAGING_SERVICEDIRECTORYPROXY_HPP_

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility/string_ref.hpp>

#include <qi/anyobject.hpp> // for circular dependency with signal.hpp
#include <qi/anyvalue.hpp>
#include <qi/api.hpp>
#include <qi/future.hpp>
#include <qi/property.hpp>
#include <qi/signal.hpp>
#include <qi/url.hpp>

namespace qi
{
class AuthProviderFactory;
using AuthProviderFactoryPtr = boost::shared_ptr<AuthProviderFactory>;
class ClientAuthenticatorFactory;
using ClientAuthenticatorFactoryPtr = boost::shared_ptr<ClientAuthenticatorFactory>;

class QI_API ServiceDirectoryProxy
{
  class Impl;
  std::unique_ptr<Impl> _p; // must be declared before any other member
public:
  enum class IdValidationStatus
  {
    Done,                 ///< The provided identity has been validated
    PendingCheckOnListen, ///< The provided identity is stored and will be validated on next listen.
  };

  enum class ListeningStatus
  {
    Done,                ///< The proxy successfully started listening
    PendingOnConnection, ///< The proxy is waiting for a connection to a service directory to
                         ///  start listening
  };

  using ServiceFilter = std::function<bool(boost::string_ref)>;

  /**
   * @param enforceAuth If set to true, rejects clients that try to skip the authentication step. If
   * false, accepts all incoming connections whether or not they authentify.
   */
  ServiceDirectoryProxy(bool enforceAuth = true);
  ~ServiceDirectoryProxy();

  Property<bool>& connected;

  UrlVector endpoints() const;

  Future<ListeningStatus> listenAsync(const Url& url);

  Future<IdValidationStatus> setValidateIdentity(const std::string& key, const std::string& crt);

  void setAuthProviderFactory(AuthProviderFactoryPtr provider);
  qi::Future<void> attachToServiceDirectory(const Url& serviceDirectoryUrl);
  void close();

  /// @param filter A function object that when passed the name of a service returns true if the
  /// service must be filtered and false if it must be made available. By default, this argument
  /// will be set to a function that always return false, thus filtering nothing.
  /// @returns The previous filter
  Future<ServiceFilter> setServiceFilter(ServiceFilter filter
                                         = PolymorphicConstantFunction<bool>{ false });
};
}

#endif // _QIMESSAGING_SERVICEDIRECTORYPROXY_HPP_
