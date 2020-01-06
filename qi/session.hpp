#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_MESSAGING_SESSION_HPP_
#define _QI_MESSAGING_SESSION_HPP_

#include <qi/api.hpp>
#include <qi/clock.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include <qi/messaging/authproviderfactory.hpp>
#include <qi/messaging/clientauthenticatorfactory.hpp>
#include <qi/messaging/ssl/ssl.hpp>
#include <qi/future.hpp>
#include <qi/anyobject.hpp>
#include <ka/macro.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4251, )

namespace qi {

  class SessionPrivate;
  class AuthProvider;
  using AuthProviderPtr = boost::shared_ptr<AuthProvider>;
  using CapabilityMap = std::map<std::string, AnyValue>;

  class Session;
  using SessionPtr = boost::shared_ptr<Session>;

  struct QI_API SessionConfig
  {
    /// These URLs are guaranteed to be valid.
    static Url defaultConnectUrl();
    static Url defaultListenUrl();

    boost::optional<Url> connectUrl;
    std::vector<Url> listenUrls;

    /// Client side configuration for SSL/TLS. This is used when the session tries to connect
    /// to a remote service (including a service directory), through `connect` or `service(s)`.
    ssl::ClientConfig clientSslConfig;

    /// Server side configuration for SSL/TLS. This is used when the session runs a TCP server,
    // through either `listen`, `listenStandalone` or `registerService`.
    ssl::ServerConfig serverSslConfig;

    /// If set, enables client side authentication on the session, using
    /// this client authenticator factory.
    /// If not set, disables client side authentication on the session.
    boost::optional<ClientAuthenticatorFactoryPtr> clientAuthenticatorFactory;

    /// If set, enables server side authentication on the session, using
    /// this authentication provider factory.
    /// If not set, disables server side authentication on the session.
    boost::optional<AuthProviderFactoryPtr> authProviderFactory;
  };

  /** A Session allows you to interconnect services on the same machine or over
   * the network.
   *
   * \includename{qi/session.hpp}
   */
  class QI_API Session : boost::noncopyable, public ::boost::enable_shared_from_this<Session> {
  public:
    using Config = SessionConfig;

    Session();

    /// This constructor merges the boolean into the configuration as such:
    ///   - if the boolean is true and the authentication provider factory is not set
    ///   in the configuration, then it is set with a default authentication provider
    ///   factory that creates authentication providers that authorize any client.
    ///   - if the boolean is false and the authentication provider factory is set in
    ///   the configuration, then it is reset and the previous value is discarded (it
    ///   would have been ignored anyway as the authentication would have been disabled).
    /// @deprecated Enforcing the authentication is now done by setting the
    ///             authentication provider factory of the session configuration.
    QI_API_DEPRECATED_MSG(enforcing the authentication is now done by setting the
                          authentication provider factory and the client authenticator
                          factory of the session configuration)
    Session(bool enforceAuthentication, Config config = {});

    explicit Session(Config config);

    virtual ~Session();

    enum ServiceLocality {
      ServiceLocality_All    = 0,
      ServiceLocality_Local  = 1,
      ServiceLocality_Remote = 2
    };

    static const char* serviceDirectoryServiceName();

    const SessionConfig& config() const;

    // Client
    /// Uses the connection URL from the configuration or the hardcoded default connect URL if the
    /// first one isn't set.
    qi::FutureSync<void> connect();

    /// Ignores the configuration URL and uses the given one instead.
    qi::FutureSync<void> connect(const char* serviceDirectoryURL);
    qi::FutureSync<void> connect(const std::string &serviceDirectoryURL);
    qi::FutureSync<void> connect(const qi::Url &serviceDirectoryURL);

    bool isConnected() const;

    /// Returns the URL of the service directory used by this session.
    ///
    /// If the session is standalone, returns the first of the endpoints
    /// ordered by preference as defined by the `qi::isPreferredEndpoint`
    /// predicate. Otherwise, returns the URL that was used to connect to the
    /// service directory.
    qi::Url url() const;

    qi::FutureSync< std::vector<ServiceInfo> > services(ServiceLocality locality = ServiceLocality_All);

    static qi::MilliSeconds defaultServiceTimeout()
    {
      return qi::Minutes{1};
    }

    qi::FutureSync<qi::AnyObject> service(const std::string& name)
    {
      return service(name, "", defaultServiceTimeout());
    }

    qi::FutureSync<qi::AnyObject> service(const std::string& name, qi::MilliSeconds timeout)
    {
      return service(name, "", timeout);
    }

    qi::FutureSync< qi::AnyObject > service(const std::string& name,
                                            const std::string& protocol)
    {
      return service(name, protocol, defaultServiceTimeout());
    }

    /// Returns the asked service.
    ///
    /// If the timeout triggers, the returned future is canceled.
    qi::FutureSync< qi::AnyObject > service(const std::string& name,
                                            const std::string& protocol,
                                            qi::MilliSeconds timeout);

    // Server
    /// Uses the listen URLs from the configuration.
    qi::FutureSync<void> listen();

    /// Ignores the configuration listen URLs and uses the given one instead.
    qi::FutureSync<void> listen(const qi::Url &address);

    /// Ignores the configuration listen URLs and uses the given ones instead. If the parameter is
    /// empty, uses the hardcoded default listen URL.
    qi::FutureSync<void> listen(const std::vector<qi::Url>& addresses);

    /// Returns the list of endpoints for this session. Returns an empty list
    /// if the session is not listening. The list of endpoints is ordered by
    /// preference as defined by the `qi::isPreferredEndpoint` binary predicate.
    std::vector<qi::Url> endpoints() const;

    //close both client and server side
    qi::FutureSync<void>    close();

    //this create a listen and create a service directory
    /// Uses the listen URLs from the configuration or the hardcoded default listen URL if the first
    /// one is empty.
    qi::FutureSync<void> listenStandalone();

    /// Ignores the configuration listen URLs and uses the given one instead.
    qi::FutureSync<void> listenStandalone(const qi::Url &address);

    /// Ignores the configuration listen URLs and uses the given ones instead.
    qi::FutureSync<void> listenStandalone(const std::vector<qi::Url> &addresses);

    qi::FutureSync<unsigned int> registerService(const std::string &name, AnyObject object);
    qi::FutureSync<void>         unregisterService(unsigned int serviceId);


    void setAuthProviderFactory(AuthProviderFactoryPtr);
    void setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr);

    /** Load a module and register the specified object on the session
     *
     * Tries to call the factory with (this, args...) if possible, otherwise it
     * calls it with (args...) only.
     */
    qi::FutureSync<unsigned int> loadService(const std::string& moduleName, const std::string &renameModule = "",
        const AnyReferenceVector& args = AnyReferenceVector());

    /** Load a module and call the specified function asynchronously
     *
     * Tries to call the function with (this, args...) if possible, otherwise it
     * calls it with (args...) only.
     */
    template <typename T>
    qi::FutureSync<T> callModule(const std::string& moduleName, const AnyReferenceVector& args = AnyReferenceVector())
    {
      qi::Promise<T> promise;
      qi::Future<qi::AnyValue> future = _callModule(moduleName, args, qi::MetaCallType_Queued);
      promise.setOnCancel([future](qi::Promise<T>&) mutable {future.cancel();}); // keeps the future alive
      future.then(qi::bind(qi::detail::futureAdapterVal<T>, future, promise));
      return promise.future();
    }

#define pushi(z, n, _) params.push_back(p ## n);
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)             \
  void loadService(                                                   \
      const std::string& moduleName, const std::string& renameModule, \
      qi::AutoAnyReference pp0 comma                                  \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))               \
  {                                                                   \
    std::vector<qi::AnyReference> params;                             \
    params.reserve(n+1);                                              \
    params.push_back(pp0);                                            \
    BOOST_PP_REPEAT(n, pushi, _)                                      \
    loadService(moduleName, renameModule, params);                    \
  }                                                                   \
  template <typename T>                                               \
  qi::FutureSync<T> callModule(                                       \
      const std::string& moduleName,                                  \
      qi::AutoAnyReference pp0 comma                                  \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))               \
  {                                                                   \
    std::vector<qi::AnyReference> params;                             \
    params.reserve(n+1);                                              \
    params.push_back(pp0);                                            \
    BOOST_PP_REPEAT(n, pushi, _)                                      \
    return callModule<T>(moduleName, params);                         \
  }
QI_GEN(genCall)
#undef genCall
#undef pushi

    /** Waits for a service to become available and fails if the timeout has expired.
     * The future is set immediately if the service is already available.
     * The future is canceled if the timeout triggered.
     */
    FutureSync<void> waitForService(const std::string& servicename, MilliSeconds timeout);

    /** Waits for a service to become available. The future is set immediately
     * if the service is already available.
     * The timeout used is given by `defaultWaitForServiceTimeout()`.
     */
    qi::FutureSync<void> waitForService(const std::string& service);

  public:
    qi::Signal<unsigned int, std::string> serviceRegistered;
    qi::Signal<unsigned int, std::string> serviceUnregistered;
    // C4251
    qi::Signal<>                          connected;
    // C4251
    qi::Signal<std::string>               disconnected;

    inline static MilliSeconds defaultWaitForServiceTimeout()
    {
      return Minutes{5};
    }

  protected:
    friend class SessionPrivate;
    std::unique_ptr<SessionPrivate> _p;

  private:
    qi::Future<AnyValue> _callModule(const std::string& moduleName,
        const AnyReferenceVector& args,
        qi::MetaCallType metacallType);

    qi::FutureSync<void> waitForServiceImpl(const std::string& service);
  };

  template <typename... Args>
  SessionPtr makeSession(Args&&... args)
  {
    return boost::make_shared<qi::Session>(std::forward<Args>(args)...);
  }
}

QI_TYPE_ENUM(qi::Session::ServiceLocality);

KA_WARNING_POP()

#endif  // _QIMESSAGING_SESSION_HPP_
