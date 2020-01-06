/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#include <ka/macro.hpp>
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4355, )

#include <sstream>
#include <qi/session.hpp>
#include <ka/scoped.hpp>
#include <ka/functorcontainer.hpp>
#include <ka/functor.hpp>
#include "message.hpp"
#include "messagesocket.hpp"
#include <qi/anyobject.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include <qi/messaging/ssl/ssl.hpp>
#include "remoteobject_p.hpp"
#include "session_p.hpp"
#include <qi/anymodule.hpp>
#include <qi/path.hpp>

#include "authprovider_p.hpp"
#include "clientauthenticator_p.hpp"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/join.hpp>

qiLogCategory("qimessaging.session");

namespace qi {
  SessionPrivate::SessionPrivate(qi::Session* session,
                                 SessionConfig config)
    : _config(std::move(config))
    , _sdClient(_config.clientSslConfig, _config.clientAuthenticatorFactory)
    , _serverObject(&_sdClient, _config.serverSslConfig, _config.authProviderFactory)
    , _socketsCache(_config.clientSslConfig)
    , _serviceHandler(&_socketsCache, &_sdClient, &_serverObject, _config.clientAuthenticatorFactory)
    , _servicesHandler(&_sdClient, &_serverObject)
    , _sd(&_serverObject)
    , _sdClientClosedByThis{ false }
  {
    session->connected.setCallType(qi::MetaCallType_Queued);
    session->disconnected.setCallType(qi::MetaCallType_Queued);
    session->serviceRegistered.setCallType(qi::MetaCallType_Queued);
    session->serviceUnregistered.setCallType(qi::MetaCallType_Queued);

    _sdClient.connected.connect(session->connected);
    _sdClient.disconnected.connect(&SessionPrivate::onServiceDirectoryClientDisconnected, this, _1);
    _sdClient.disconnected.connect(session->disconnected);
    _sdClient.serviceAdded.connect(session->serviceRegistered);
    _sdClient.serviceRemoved.connect(session->serviceUnregistered);
  }

  SessionPrivate::~SessionPrivate()
  {
    destroy();
    try
    {
      close();
    }
    catch (const std::exception& ex)
    {
      qiLogError() << "Exception caught during session destruction: " << ex.what();
    }
    catch (...)
    {
      qiLogError() << "Unknown exception caught during session destruction";
    }
  }

  void SessionPrivate::onServiceDirectoryClientDisconnected(std::string /*error*/)
  {
    if (_sdClientClosedByThis)
      return;

    /*
     * Remove all proxies to services if the SD is fallen.
     */
    // This callback is called only when the SD is disconnected.
    // We don't close the service directory client from here:
    // it has it's own callback to take care of it.
    _serviceHandler.close();
    _serverObject.close();
    _socketsCache.close();
  }

  void SessionPrivate::setAuthProviderFactory(AuthProviderFactoryPtr factory)
  {
    _serverObject.setAuthProviderFactory(factory);
  }

  void SessionPrivate::setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr factory)
  {
    _sdClient.setClientAuthenticatorFactory(factory);
    _serviceHandler.setClientAuthenticatorFactory(factory);
  }

  void SessionPrivate::addSdSocketToCache(Future<void> f, const qi::Url& url,
                                          qi::Promise<void> p)
  {
    qiLogDebug() << "addSocketToCache processing";
    if (f.hasError())
    {
      qiLogDebug() << "addSdSocketToCache: connect reported failure";
      _serviceHandler.removeService(Session::serviceDirectoryServiceName());
      p.setError(f.error());
      return;
    }

    // Allow the SD process to use the existing socket to talk to our services
    MessageSocketPtr sdSocket = _sdClient.socket();
    _serverObject.addOutgoingSocket(sdSocket);

    /* Allow reusing the SD socket for communicating with services.
     * To do this, we must add it to our socket cache, and for this we need
     * to know the sd machineId
     */
     std::string mid;
     try
     {
       mid = _sdClient.machineId().value();
     }
     catch (const std::exception& e)
     { // Provide a nice message for backward compatibility
       qiLogVerbose() << e.what();
       qiLogWarning() << "Failed to obtain machineId, connection to service directory will not be reused for other services.";
       p.setError(e.what());
       return;
     }
     qiLogVerbose() << "Inserting sd to cache for " << mid <<" " << url.str();
     _socketsCache.insert(mid, *toUri(sdSocket->remoteEndpoint().value()), sdSocket);
     // Also add a relative endpoint to the ServiceDirectory with the same socket, so that for
     // services that expose this relative endpoint, the client may reuse its socket.
     _socketsCache.insert(mid, *uri("qi:ServiceDirectory"), sdSocket);
     p.setValue(0);
  }

  qi::FutureSync<void> SessionPrivate::connect(const qi::Url &serviceDirectoryURL)
  {
    if (isConnected()) {
      const char* s = "Session is already connected";
      qiLogVerbose() << s;
      return qi::makeFutureError<void>(s);
    }
    _serverObject.open();
    //add the servicedirectory object into the service cache (avoid having
    // two remoteObject registered on the same transportSocket)
    _serviceHandler.addService(Session::serviceDirectoryServiceName(), _sdClient.object());
    _socketsCache.init();

    qi::Future<void> f = _sdClient.connect(serviceDirectoryURL);
    qi::Promise<void> p;

    f.then(track([=](Future<void> f) {
      _sdClientClosedByThis = false;
      addSdSocketToCache(f, serviceDirectoryURL, p);
    }, this));
    return p.future();
  }


  qi::FutureSync<void> SessionPrivate::close()
  {
    _sdClientClosedByThis = true;
    _serviceHandler.close();
    _serverObject.close();
    _socketsCache.close();
    return _sdClient.close().async();
  }

  bool SessionPrivate::isConnected() const {
    return _sdClient.isConnected();
  }

  Url SessionConfig::defaultConnectUrl()
  {
    static const Url url("tcp://127.0.0.1:9559");
    QI_ASSERT_TRUE(url.isValid());
    return url;
  }

  Url SessionConfig::defaultListenUrl()
  {
    static const Url url("tcp://127.0.0.1:0");
    QI_ASSERT_TRUE(url.isValid());
    return url;
  }

  // ###### Session
  Session::Session()
    : _p(new SessionPrivate(this, {}))
  {
  }

  Session::Session(bool enforceAuthentication, SessionConfig config)
    : Session([&] {
          // Merge the authentication boolean into the configuration.
          if (!enforceAuthentication && config.authProviderFactory)
            config.authProviderFactory = {};
          else if (enforceAuthentication && !config.authProviderFactory)
            config.authProviderFactory = boost::make_shared<NullAuthProviderFactory>();
          return std::move(config);
        }())
  {
  }

  Session::Session(SessionConfig config)
    : _p(new SessionPrivate(this, std::move(config)))
  {
  }

  Session::~Session()
  {
    // Reset the pointer before the end of the destructor in case it is tracked by callbacks that
    // might use other members of Session.
    _p.reset();
  }

  const char* Session::serviceDirectoryServiceName()
  {
    static const auto sdServiceName = "ServiceDirectory";
    return sdServiceName;
  }

  const SessionConfig& Session::config() const
  {
    return _p->_config;
  }

  // ###### Client
  qi::FutureSync<void> Session::connect()
  {
    // If no connect URL was specified in the configuration, fallback on the hardcoded default
    // connect URL. This is to have an homogeneous behavior with `listen`.
    const auto connectUrl = [&]{
        const auto& cfgUrl = _p->_config.connectUrl;
        if (cfgUrl) return *cfgUrl;
        const auto defaultUrl = SessionConfig::defaultConnectUrl();
        qiLogVerbose() << "No connect URL configured, using the hardcoded default value '"
                       << defaultUrl << "'";
        return defaultUrl;
      }();
    return connect(connectUrl);
  }

  qi::FutureSync<void> Session::connect(const char* serviceDirectoryURL)
  {
    return _p->connect(qi::Url(serviceDirectoryURL, "tcp", 9559));
  }
  qi::FutureSync<void> Session::connect(const std::string &serviceDirectoryURL)
  {
    return _p->connect(qi::Url(serviceDirectoryURL, "tcp", 9559));
  }
  qi::FutureSync<void> Session::connect(const qi::Url &serviceDirectoryURL)
  {
    return _p->connect(serviceDirectoryURL);
  }

  qi::FutureSync<void> Session::close() {
    qi::Future<void> f = _p->close();
    return f;
  }

  bool Session::isConnected() const {
    return _p->isConnected();
  }

  qi::Url Session::url() const {
    if (_p->_sdClient.isLocal())
      return endpoints().front();
    else
      return _p->_sdClient.url();
  }

  void Session::setAuthProviderFactory(AuthProviderFactoryPtr factory)
  {
    _p->setAuthProviderFactory(factory);
  }

  void Session::setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr factory)
  {
    _p->setClientAuthenticatorFactory(factory);
  }

  //3 cases:
  //  - local service => just return the vector
  //  - remote => ask the sd return the result
  //  - all => ask the sd, append local services, return the result
  qi::FutureSync< std::vector<ServiceInfo> > Session::services(ServiceLocality locality)
  {
    if (!isConnected()) {
      return qi::makeFutureError< std::vector<ServiceInfo> >("Session not connected.");
    }
    return _p->_servicesHandler.services(locality);
  }

  qi::FutureSync< qi::AnyObject > Session::service(
    const std::string& service, const std::string& protocol, qi::MilliSeconds timeout)
  {
    if (!isConnected()) {
      return qi::makeFutureError< qi::AnyObject >("Session not connected.");
    }
    return cancelOnTimeout(_p->_serviceHandler.service(service, protocol), timeout);
  }

  qi::FutureSync<void> Session::listen()
  {
    // If no listen URL was specified in the configuration, then the list is empty and the `listen`
    // overload will use the hardcoded default value.
    return listen(_p->_config.listenUrls);
  }

  qi::FutureSync<void> Session::listen(const qi::Url& address)
  {
    qiLogVerbose() << "Session listener created on " << address.str();
    return _p->_serverObject.listen(address);
  }

  qi::FutureSync<void> Session::listen(const std::vector<Url>& addresses)
  {
    if (addresses.empty())
    {
      const auto defaultListenUrl = SessionConfig::defaultListenUrl();
      qiLogWarning() << "No listen URL specified, using the hardcoded default value '"
                     << defaultListenUrl << "', consider specifying a value.";
      return listen(defaultListenUrl);
    }

    qiLogVerbose() << "Session listener created on "
                << boost::join(boost::adaptors::transform(addresses,
                                                          [](const Url& address) {
                                                            return address.str();
                                                          }),
                               ", ");
    std::vector<Future<void>> futs;
    futs.reserve(addresses.size());
    std::transform(addresses.cbegin(), addresses.cend(), std::back_inserter(futs),
                   [&](const Url& listenUrl) { return listen(listenUrl).async(); });

    Promise<void> prom;
    waitForAll(futs).async().andThen([prom](const std::vector<Future<void>>& futs) mutable {
      std::ostringstream oss;
      bool failure = false;
      for (const auto& fut : futs)
      {
        if (fut.hasValue())
          continue;

        const auto wasFailure = ka::exchange(failure, true);
        if (wasFailure)
          oss << ", ";
        if (fut.hasError())
          oss << fut.error();
        else
        {
          QI_ASSERT_TRUE(fut.isCanceled());
          oss << "listen request was canceled";
        }
      }
      if (failure)
        prom.setError("Failed to listen on all URLs: " + oss.str());
      else
        prom.setValue(nullptr);
    });
    return prom.future();
  }

  qi::FutureSync<void> Session::listenStandalone()
  {
    // If no listen URL was specified in the configuration, fallback on the hardcoded default
    // listen URL. This is to have an homogeneous behavior with `listen`.
    const auto& listenUrls = _p->_config.listenUrls;
    if (listenUrls.empty())
    {
      const auto defaultListenUrl = SessionConfig::defaultListenUrl();
      qiLogWarning() << "No listen URL configured, using the hardcoded default value '"
                     << defaultListenUrl << "', consider specifying a value.";
      return listenStandalone(defaultListenUrl);
    }
    return listenStandalone(listenUrls);
  }

  qi::FutureSync<void> Session::listenStandalone(const qi::Url &address)
  {
    return _p->listenStandalone({address});
  }

  qi::FutureSync<void> Session::listenStandalone(const std::vector<qi::Url> &addresses)
  {
    return _p->listenStandalone(addresses);
  }

  qi::FutureSync<void> SessionPrivate::listenStandalone(const std::vector<qi::Url>& addresses)
  {
    _serverObject.open();
    qi::Promise<void> p;
    //will listen and connect
    qi::Future<void> f = _sd.listenStandalone(addresses);
    f.then(track(std::bind(&SessionPrivate::listenStandaloneCont, this, p, std::placeholders::_1), this));
    return p.future();
  }

  void SessionPrivate::listenStandaloneCont(qi::Promise<void> p, qi::Future<void> f)
  {
    if (f.hasError())
      p.setError(f.error());
    else
    {
      _sdClient.setServiceDirectory(_sd._serviceBoundObject->object());
      // _sdClient will trigger its connected, which will trigger our connected

      p.setValue(0);
    }
  }

  qi::FutureSync<unsigned int> Session::registerService(const std::string &name, qi::AnyObject obj)
  {
    if (!obj)
      return makeFutureError<unsigned int>("registerService: Object is empty");

    // Compatibility: Exposing a service means the session must be a server (it must be listening
    // for connections). A better solution would probably be to raise an error, but since a lot of
    // code relies on the following behavior, we're keeping it that way.
    if (endpoints().empty())
      listen();

    if (!isConnected()) {
      return qi::makeFutureError< unsigned int >("Session not connected.");
    }

    return _p->_serverObject.registerService(name, obj);
  }

  qi::FutureSync<void> Session::unregisterService(unsigned int idx)
  {
    if (!isConnected()) {
      return qi::makeFutureError<void>("Session not connected.");
    }

    return _p->_serverObject.unregisterService(idx);
  }

  std::vector<qi::Url> Session::endpoints() const
  {
    return ka::fmap(toUrl, _p->_serverObject.endpoints().value());
  }

  qi::FutureSync<unsigned int> Session::loadService(const std::string &moduleName, const std::string& renameModule, const AnyReferenceVector& args)
  {
    size_t separatorPos = moduleName.find_last_of(".");
    std::string function = moduleName.substr(separatorPos + 1);

    std::string rename = renameModule;
    if (rename.empty())
      rename = function;

    qi::AnyValue retval = _callModule(moduleName, args, qi::MetaCallType_Direct).value();
    return registerService(rename, retval.to<qi::AnyObject>());
  }

  qi::Future<AnyValue> Session::_callModule(const std::string &moduleFunction,
      const AnyReferenceVector& args,
      qi::MetaCallType metacallType)
  {
    size_t separatorPos = moduleFunction.find_last_of(".");
    std::string moduleName = moduleFunction.substr(0, separatorPos);
    std::string functionName = moduleFunction.substr(separatorPos + 1);

    qi::AnyModule module = qi::import(moduleName);

    AnyReferenceVector fullargs;
    SessionPtr thisptr = shared_from_this();
    fullargs.push_back(AnyReference::from(thisptr));
    fullargs.insert(fullargs.end(), args.begin(), args.end());

    int id = module.metaObject().findMethod(functionName, fullargs);
    qi::Future<AnyReference> ret;
    if (id > 0)
      ret = module.metaCall(functionName, fullargs, metacallType);
    else
      ret = module.metaCall(functionName, args, metacallType);

    qi::Promise<AnyValue> promise;
    promise.setOnCancel([ret](qi::Promise<AnyValue>&) mutable { ret.cancel(); });
    ret.then(qi::bind(qi::detail::futureAdapter<qi::AnyValue>, _1, promise));
    return promise.future();
  }

  FutureSync<void> Session::waitForService(const std::string& servicename)
  {
    return waitForService(servicename, defaultWaitForServiceTimeout());
  }

  FutureSync<void> Session::waitForService(const std::string& servicename, MilliSeconds timeout)
  {
    return cancelOnTimeout(waitForServiceImpl(servicename).async(), timeout);
  }

  qi::FutureSync<void> Session::waitForServiceImpl(const std::string& servicename)
  {
    qi::Promise<void> promise(
          [](qi::Promise<void> &promise)
          {
            try
            {
              promise.setCanceled();
            }
            catch (...) {} // promise already set
          });

    auto onServiceAdded =
          [promise, servicename](unsigned int, const std::string &name) mutable
          {
            if (name != servicename)
              return;
            try
            {
              promise.setValue(nullptr);
            }
            catch (...) {} // promise already set
          };

    auto futureLink = _p->_sdClient.serviceAdded.connectAsync(
          qi::AnyFunction::from(onServiceAdded)).andThen( // TODO avoid qi::AnyFunction. see #42091
          [](const SignalSubscriber &sub) { return sub.link(); });

    // check if the service is already there *after* connecting to the signal,
    // to avoid a race.
    auto privSession = _p.get(); //< raw pointer to a Trackable
    auto futureService = futureLink.andThen(track(
          [privSession, servicename](qi::SignalLink) mutable
          {
            // Do not use the `Session_Service::service` method that returns an object for the
            // service, instead use the service directory client `service` method that returns the
            // service info. The reason behind this choice is that to construct a full object, we
            // need to first get the service info from the service directory, then connect to the
            // service (by authentifying this session to it) and get the meta object of the service,
            // meaning at least 3 RPC to finally discard the object because all we need to know is
            // if the service already exists or not.
            return privSession->_sdClient.service(servicename);
          }, privSession)).unwrap();

    futureService.connect(
          [promise](qi::Future<ServiceInfo> futureService) mutable
          {
             if (futureService.hasValue())
             {
               try
               {
                 promise.setValue(nullptr);
               }
               catch (...) {} // promise already set
             }
          });

    // schedule some clean up
    promise.future().connect(
          track([futureLink, privSession] (qi::Future<void>) mutable
          {
            futureLink.cancel();
            futureLink.andThen(track(
                  [privSession](qi::SignalLink link)
                  {
                    privSession->_sdClient.serviceAdded.disconnectAsync(link);
                  }, privSession));
          }, privSession));
    return promise.future();
  }

}

KA_WARNING_POP()
