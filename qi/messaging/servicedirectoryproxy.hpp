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
#include <ka/macroregular.hpp>
#include <qi/anyobject.hpp> // for circular dependency with signal.hpp
#include <qi/anyvalue.hpp>
#include <qi/api.hpp>
#include <qi/future.hpp>
#include <qi/property.hpp>
#include <qi/signal.hpp>
#include <qi/url.hpp>
#include <qi/messaging/ssl/ssl.hpp>

namespace qi
{
class AuthProviderFactory;
using AuthProviderFactoryPtr = boost::shared_ptr<AuthProviderFactory>;
class ClientAuthenticatorFactory;
using ClientAuthenticatorFactoryPtr = boost::shared_ptr<ClientAuthenticatorFactory>;

class ServiceDirectoryProxy;
using ServiceDirectoryProxyPtr = boost::shared_ptr<ServiceDirectoryProxy>;

/// This class implements a proxy to a service directory that clients can connect to in order to
/// access the service directory services. It does so by propagating the services from the service
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
private:
  class Impl;
  std::unique_ptr<Impl> _p; // must be declared before any other member

public:

  // A distinct type is defined instead of a mere alias to
  // `std::function<bool(boost::string_ref)>` to enable aggregate initialization
  // of type `Config` below.
  struct FilterService
  {
    using F = std::function<bool(boost::string_ref)>;
    F f = ka::constant_function(false);

    FilterService() = default;

    inline explicit FilterService(F f) : f(ka::mv(f)) {}

    inline bool operator()(boost::string_ref s)
    {
      return f(s);
    }

    inline bool operator()(boost::string_ref s) const
    {
      return f(s);
    }
  };

  /// File paths used to create a service directory proxy listening with TLS.
  struct TlsFilepaths
  {
    // No constructor is defined since there is no invariant to preserve.
    boost::filesystem::path certChain;
    boost::filesystem::path privateKey;
  };

  /// File paths used to create a service directory proxy listening with mTLS.
  struct MTlsFilepaths
  {
    // No inheritance is used to preserve aggregate initialization.
    // No constructor is defined since there is no invariant to preserve.
    boost::filesystem::path certChain;
    boost::filesystem::path privateKey;
    boost::filesystem::path trustedCert;
  };

  /// File paths corresponding to the possible listening protocols.
  /// `ka::unit_t` corresponds to the TCP case, i.e. no file path is needed. It
  /// is intentionally put first to be selected by default construction.
  using Filepaths = boost::variant<ka::unit_t, TlsFilepaths, MTlsFilepaths>;

  /// The configuration of a service directory proxy. It contains any parameter a proxy can handle.
  struct Config
  {
    /// The URL of the service directory to act as a proxy of. It is used at construction or opening
    /// to connect the service directory client part of the proxy.
    /// This URL must be valid.
    Url serviceDirectoryUrl;

    /// The list of URL to listen on. It is used at construction or opening by the server part of
    /// the proxy once it is connected to the service directory.
    /// Each URL in this list must be valid.
    std::vector<Url> listenUrls;

    // If set, rejects clients that try to skip the authentication step and validate them
    // using a provider made from the factory. If empty, accepts all incoming connections
    // whether or not they authenticate.
    boost::optional<AuthProviderFactoryPtr> authProviderFactory;

    // Used to optionally authenticate to the service directory if authentication is enabled.
    boost::optional<ClientAuthenticatorFactoryPtr> clientAuthenticatorFactory;

    /// A function object that when passed the name of a service returns true if the
    /// service must be filtered and false if it must be made available. By default, this argument
    /// is set to a function that always return false, thus filtering nothing.
    FilterService filterService;

    /// The SSL configuration used for the service directory client part of the proxy.
    /// You should set this field if the service directory is listening on a TLS socket (i.e. the
    /// service directory URL has a 'tcps' or 'tcpsm' scheme).
    ssl::ClientConfig clientSslConfig;

    /// The SSL configuration used for the server part of the proxy.
    /// If one of the listen URL is a TLS URL (i.e. it has a 'tcps' or 'tcpsm' scheme), it must
    /// contain a valid certificate and a private key. If one of the listen URL is a mTLS URL
    /// ('tcpsm'), the trust store should contain at least one certificate used to verify the
    /// certificates presented by clients.
    ssl::ServerConfig serverSslConfig;

    /// Creates a configuration based on the listening protocol.
    ///
    /// When listening with
    ///
    /// - TCP (scheme "tcp"):
    ///     + No file path must be provided (type `ka::unit_t`).
    ///
    /// - TLS (scheme "tcps"):
    ///     + The certificate chain file path and private key file path must be
    ///     provided (type `TlsFilepaths`).
    ///
    ///     + The server certificate chain is read and set from corresponding file path.
    ///
    ///     + The server private key is read and set from corresponding file path.
    ///
    /// - mTLS (scheme "tcpsm"):
    ///     + In addition to the two previous file paths, the
    ///     trusted certificate file path must be provided (type `MTlsFilepaths`).
    ///
    ///     + The server trust store is fed with the *first* certificate from
    ///     the corresponding file.
    ///
    ///     + Partial chain verification is enabled.
    ///
    /// The files' data must be encoded in the PEM/PKCS#8 format.
    ///
    /// Other parameters (SD URL, listening URL, service filter, authentication
    /// factory) are put as-is in their corresponding fields in the
    /// configuration.
    ///
    /// The client authenticator factory is left empty.
    ///
    /// Any error will result in a `std::exception` being thrown.
    static Config createFromListeningProtocol(
      Url sdUrl, Url listenUrl, FilterService, Filepaths = {},
      boost::optional<boost::shared_ptr<AuthProviderFactory>> = {});
  };

  /// Proxy state diagram:
  /// --------------------
  ///    ╔═══════════════╗
  ///    ║    Created    ║
  ///    ║       /       ║
  ///    ║ Initializing  ║
  ///    ╚═══════╤═══════╝
  ///            ▼
  ///     ╔═════════════╗
  ///     ║ Initialized ║
  ///     ║      /      ║◀╌╌╌┐
  ///     ║ Connecting  ║    ╎
  ///     ╚══════╤══════╝    ╎
  ///            ▼           ╎
  ///   ╔═════════════════╗  ╎
  ///   ║    Connected    ║  ╎ Connection to the
  ///   ║        /        ║  ╎ service directory
  ///   ║ TryingToListen  ║  ╎      is lost
  ///   ╚════════╤════════╝  ╎
  ///            ▼           ╎
  ///      ╔═══════════╗     ╎
  ///      ║   Ready   ║     ╎
  ///      ║     /     ╟╌╌╌╌╌┘
  ///      ║ Listening ║
  ///      ╚═══════════╝
  enum class Status
  {
    Initializing,
    Created = Initializing,
    Connecting,
    Initialized = Connecting,
    TryingToListen,
    Connected = TryingToListen,
    Listening,
    Ready = Listening,
  };

private:
  explicit ServiceDirectoryProxy(Config config, Promise<void> ready);

public:
  /// Creates a new service directory proxy with a configuration created through
  /// `Config::createFromListeningProtocol`.
  /// Parameters are passed as-is.
  static Future<ServiceDirectoryProxyPtr> createFromListeningProtocol(
    Url sdUrl, Url listenUrl, FilterService, Filepaths = {},
    boost::optional<boost::shared_ptr<AuthProviderFactory>> = {}
  );

  /// Creates a new service directory proxy according to the given configuration.
  /// Returns a future that is set with a non-null pointer once the proxy is ready
  /// (see `Status`).
  /// Returns a future in error if any URL specified in the configuration is invalid, if the
  /// connection to the service directory fails or if the server fails to listen on the specified
  /// endpoints.
  static Future<ServiceDirectoryProxyPtr> create(Config config);

  ServiceDirectoryProxy(ServiceDirectoryProxy&& o) noexcept = default;
  ServiceDirectoryProxy& operator=(ServiceDirectoryProxy&& o) noexcept = default;

  ~ServiceDirectoryProxy();

  /// Returns a property of the status of the proxy.
  Property<Status>& status();

  /// @overload
  const Property<Status>& status() const;

  /// Returns the list of endpoints as URL the proxy listens on.
  UrlVector endpoints() const;
};

QI_API std::ostream& operator<<(std::ostream&, ServiceDirectoryProxy::Status);

}

#endif // _QIMESSAGING_SERVICEDIRECTORYPROXY_HPP_
