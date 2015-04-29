/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#include <qi/session.hpp>
#include "message.hpp"
#include "transportsocket.hpp"
#include <qi/anyobject.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include "remoteobject_p.hpp"
#include "session_p.hpp"
#include <qi/anymodule.hpp>

#include "authprovider_p.hpp"
#include "clientauthenticator_p.hpp"

qiLogCategory("qimessaging.session");

namespace qi {

  SessionPrivate::SessionPrivate(qi::Session* session, bool enforceAuth)
    : qi::Trackable<SessionPrivate>(this)
    , _sdClient(enforceAuth)
    , _serverObject(&_sdClient, enforceAuth)
    , _serviceHandler(&_socketsCache, &_sdClient, &_serverObject, enforceAuth)
    , _servicesHandler(&_sdClient, &_serverObject)
    , _sd(&_serverObject)
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
    setAuthProviderFactory(AuthProviderFactoryPtr(new NullAuthProviderFactory));
    setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr(new NullClientAuthenticatorFactory));
  }

  SessionPrivate::~SessionPrivate() {
    destroy();
    close();
  }

  void SessionPrivate::onServiceDirectoryClientDisconnected(std::string error) {
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
      _serviceHandler.removeService("ServiceDirectory");
      p.setError(f.error());
      return;
    }

    // Allow the SD process to use the existing socket to talk to our services
    _serverObject.registerSocket(_sdClient.socket());

    /* Allow reusing the SD socket for communicating with services.
     * To do this, we must add it to our socket cache, and for this we need
     * to know the sd machineId
     */
     std::string mid;
     try
     {
       mid = _sdClient.machineId();
     }
     catch (const std::exception& e)
     { // Provide a nice message for backward compatibility
       qiLogVerbose() << e.what();
       qiLogWarning() << "Failed to obtain machineId, connection to service directory will not be reused for other services.";
       p.setValue(0);
       return;
     }
     TransportSocketPtr s = _sdClient.socket();
     qiLogVerbose() << "Inserting sd to cache for " << mid <<" " << url.str() << std::endl;
     _socketsCache.insert(mid, s->remoteEndpoint(), s);
     p.setValue(0);
  }

  qi::FutureSync<void> SessionPrivate::connect(const qi::Url &serviceDirectoryURL)
  {
    if (isConnected()) {
      const char* s = "Session is already connected";
      qiLogInfo() << s;
      return qi::makeFutureError<void>(s);
    }
    _serverObject.open();
    //add the servicedirectory object into the service cache (avoid having
    // two remoteObject registered on the same transportSocket)
    _serviceHandler.addService("ServiceDirectory", _sdClient.object());
    _socketsCache.init();

    qi::Future<void> f = _sdClient.connect(serviceDirectoryURL);
    qi::Promise<void> p;

    // go through hoops to get shared_ptr on this
    f.connect(&SessionPrivate::addSdSocketToCache, this, _1, serviceDirectoryURL, p);
    return p.future();
  }


  qi::FutureSync<void> SessionPrivate::close()
  {
    _serviceHandler.close();
    _serverObject.close();
    _socketsCache.close();
    return _sdClient.close();
  }

  bool SessionPrivate::isConnected() const {
    return _sdClient.isConnected();
  }


  // ###### Session
  Session::Session(bool enforceAuthentication)
    : _p(new SessionPrivate(this, enforceAuthentication))
  {

  }

  Session::~Session()
  {
    //nothing to do here, the _p is shared between multiples sessions
    //so we should not touch it.
  }


  // ###### Client
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
      return endpoints()[0];
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

  qi::FutureSync< qi::AnyObject > Session::service(const std::string &service,
                                               const std::string &protocol)
  {
    if (!isConnected()) {
      return qi::makeFutureError< qi::AnyObject >("Session not connected.");
    }
    return _p->_serviceHandler.service(service, protocol);
  }


  qi::FutureSync<void> Session::listen(const qi::Url &address)
  {
    qiLogInfo() << "Session listener created on " << address.str();
    return _p->_serverObject.listen(address);
  }

  qi::FutureSync<void> Session::listenStandalone(const qi::Url &address)
  {
    return _p->listenStandalone(address);
  }

  qi::FutureSync<void> SessionPrivate::listenStandalone(const qi::Url& address)
  {
    _serverObject.open();
    qi::Promise<void> p;
    //will listen and connect
    qi::Future<void> f = _sd.listenStandalone(address);
    f.connect(&SessionPrivate::listenStandaloneCont, this, p, _1);
    return p.future();
  }

  void SessionPrivate::listenStandaloneCont(qi::Promise<void> p, qi::Future<void> f)
  {
    if (f.hasError())
      p.setError(f.error());
    else
    {
      _sdClient.setServiceDirectory(boost::static_pointer_cast<ServiceBoundObject>(_sd._serviceBoundObject)->object());
      // _sdClient will trigger its connected, which will trigger our connected

      p.setValue(0);
    }
  }

  bool Session::setIdentity(const std::string& key, const std::string& crt)
  {
    return _p->_serverObject.setIdentity(key, crt);
  }

  qi::FutureSync<unsigned int> Session::registerService(const std::string &name, qi::AnyObject obj)
  {
    if (!obj)
      return makeFutureError<unsigned int>("registerService: Object is empty");
    if (endpoints().empty()) {
      qi::Url listeningAddress("tcp://0.0.0.0:0");
      qiLogVerbose() << listeningAddress.str() << "." << std::endl;
      listen(listeningAddress);
    }

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
    return _p->_serverObject.endpoints();
  }

  void Session::loadService(const std::string &moduleName, const std::string& renameModule, const AnyReferenceVector& args)
  {
    size_t separatorPos = moduleName.find_last_of(".");
    std::string package = moduleName.substr(0, separatorPos);
    std::string factory = moduleName.substr(separatorPos + 1);

    std::string rename = renameModule;
    if (rename.empty())
      rename = factory;
    qi::AnyModule p = qi::import(package);

    AnyReferenceVector fullargs;
    SessionPtr thisptr = shared_from_this();
    fullargs.push_back(AnyReference::from(thisptr));
    fullargs.insert(fullargs.end(), args.begin(), args.end());

    int id = p.metaObject().findMethod(factory, fullargs);
    qi::Future<AnyReference> ret;
    if (id > 0)
      ret = p.metaCall(factory, fullargs);
    else
      ret = p.metaCall(factory, args);
    qi::AnyValue retval(ret.value(), false, true);
    registerService(rename, retval.to<qi::AnyObject>());
  }

  void SessionPrivate::onTrackedServiceAdded(const std::string& actual,
      const std::string& expected,
      qi::Promise<void> promise,
      boost::shared_ptr<qi::Atomic<int> > link)
  {
    if (actual != expected)
      return;

    // only do it once in case of multiple calls
    SignalLink link2 = link->swap(0);

    if (link2 == 0)
      return;

    _sdClient.serviceAdded.disconnect(link2);

    promise.setValue(0);
  }

  void SessionPrivate::onServiceTrackingCanceled(qi::Promise<void> promise,
      boost::shared_ptr<qi::Atomic<int> > link)
  {
    SignalLink link2 = link->swap(0);

    if (link2 == 0)
      return;

    _sdClient.serviceAdded.disconnect(link2);
    promise.setCanceled();
  }

  qi::FutureSync<void> Session::waitForService(const std::string& servicename)
  {
    boost::shared_ptr<qi::Atomic<int> > link =
      boost::make_shared<qi::Atomic<int> >(0);

    qi::Promise<void> promise(qi::bindWithFallback<void(qi::Promise<void>)>(
          boost::function<void()>(),
          &SessionPrivate::onServiceTrackingCanceled,
          boost::weak_ptr<SessionPrivate>(_p),
          _1,
          link));
    *link = (int)_p->_sdClient.serviceAdded.connect(
          &SessionPrivate::onTrackedServiceAdded,
          boost::weak_ptr<SessionPrivate>(_p),
          _2,
          servicename,
          promise,
          link);

    qi::Future<qi::AnyObject> s = service(servicename);
    if (!s.hasError())
      // service is already available, trigger manually (it's ok if it's
      // triggered multiple time)
      _p->onTrackedServiceAdded(servicename, servicename, promise, link);

    return promise.future();
  }

}


#ifdef _MSC_VER
# pragma warning( pop )
#endif
