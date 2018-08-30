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

#include <ka/functional.hpp>
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

/// This class implements a proxy to a service directory that clients can connect to in order to
/// access the service directory services. It does that by propagating the services from the service
/// directory to itself and its own services back to the service directory.
///
/// It can also filter out some of the services to disable their access from clients.
///
/// The following diagram roughly explains how the service propagation is implemented:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Proxy Client                   Proxy               Service Directory             SD Client
/// ------------                   -----               -----------------             ---------
///      |                           |                         |                         |
///      | --- Register service ---> | --- Mirror to SD -----> |                         |
///      | --- Unregister service -> | --- Unmirror to SD ---> |                         |
///      |                           |                         |                         |
///      |                           | <-- Mirror from SD ---- | <- Register service --- |
///      |                           | <-- Unmirror from SD -- | <- Unregister service - |
///      |                           |                         |                         |
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

  enum class ListenStatus
  {
    NotListening,        ///< The proxy is not set to listen.
    Listening,           ///< The proxy is successfully listening.
    Starting,            ///< The proxy started setup for listening.
    PendingConnection,   ///< The proxy is waiting for a connection to a service directory to
                         ///  start listening.
  };

  enum class ConnectionStatus
  {
    NotConnected,       ///< The proxy is not set to connect to the service directory.
    Connected,          ///< The proxy is connected to the service directory.
    Starting,           ///< The proxy started connection to the service directory.
  };

  using ServiceFilter = std::function<bool(boost::string_ref)>;

  struct Status
  {
    bool isReady() const
    {
      return connection == ConnectionStatus::Connected
          && listen == ListenStatus::Listening;
    }

    bool isConnected() const { return connection == ConnectionStatus::Connected; }
    bool isListening() const { return listen == ListenStatus::Listening; }

    ConnectionStatus connection;
    ListenStatus listen;
    KA_GENERATE_FRIEND_REGULAR_OPS_2(Status, connection, listen);
  };


  /**
   * @param enforceAuth If set to true, rejects clients that try to skip the authentication step. If
   * false, accepts all incoming connections whether or not they authenticate.
   */
  ServiceDirectoryProxy(bool enforceAuth = true);
  ~ServiceDirectoryProxy();

  QI_API_DEPRECATED_MSG("Use `status` instead.")
  Property<bool>& connected;

  Property<Status>& status;

  UrlVector endpoints() const;

  Future<ListenStatus> listenAsync(const Url& url);

  Future<IdValidationStatus> setValidateIdentity(const std::string& key, const std::string& crt);

  void setAuthProviderFactory(AuthProviderFactoryPtr provider);
  qi::Future<void> attachToServiceDirectory(const Url& serviceDirectoryUrl);
  void close();

  /// @param filter A function object that when passed the name of a service returns true if the
  /// service must be filtered and false if it must be made available. By default, this argument
  /// will be set to a function that always return false, thus filtering nothing.
  /// @returns The previous filter
  Future<ServiceFilter> setServiceFilter(ServiceFilter filter
     = ka::constant_function(false));
};

QI_API std::ostream& operator<<(std::ostream&, ServiceDirectoryProxy::IdValidationStatus);
QI_API std::ostream& operator<<(std::ostream&, ServiceDirectoryProxy::ListenStatus);
QI_API std::ostream& operator<<(std::ostream&, ServiceDirectoryProxy::ConnectionStatus);

}

#endif // _QIMESSAGING_SERVICEDIRECTORYPROXY_HPP_
