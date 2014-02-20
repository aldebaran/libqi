/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#include <qimessaging/session.hpp>
#include "message.hpp"
#include "transportsocket.hpp"
#include <qitype/anyobject.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qitype/objectfactory.hpp>
#include "remoteobject_p.hpp"
#include "session_p.hpp"

qiLogCategory("qimessaging.session");

namespace qi {


  SessionPrivate::SessionPrivate(qi::Session *session)
    : _sdClient()
    , _serverObject(&_sdClient, session)
    , _serviceHandler(&_socketsCache, &_sdClient, &_serverObject)
    , _servicesHandler(&_sdClient, &_serverObject)
    , _sd(&_serverObject)
  {
    session->connected.setCallType(qi::MetaCallType_Queued);
    session->disconnected.setCallType(qi::MetaCallType_Queued);
    session->serviceRegistered.setCallType(qi::MetaCallType_Queued);
    session->serviceUnregistered.setCallType(qi::MetaCallType_Queued);

    _sdClient.connected.connect(session->connected);
    //take a shared_ptr first, or the weak_ptr wont be correct.
    _sdClient.disconnected.connect(&SessionPrivate::onDisconnected, boost::weak_ptr<SessionPrivate>(shared_from_raw(this)), _1);
    _sdClient.disconnected.connect(session->disconnected);
    _sdClient.serviceAdded.connect(session->serviceRegistered);
    _sdClient.serviceRemoved.connect(session->serviceUnregistered);
  }

  SessionPrivate::~SessionPrivate() {
    close();
  }


  void SessionPrivate::onDisconnected(std::string error) {
    /*
     * Remove all proxies to services if the SD is fallen.
     */
    _serviceHandler.close();
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
    f.connect(&SessionPrivate::addSdSocketToCache, weak_from_raw(this), _1, serviceDirectoryURL, p);
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
  Session::Session()
    : _p(new SessionPrivate(this))
  {

  }

  Session::~Session()
  {
    //nothing to do here, the _p is shared between multiples sessions
    //so we should not touch it.
  }


  // ###### Client
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
    f.connect(&SessionPrivate::listenStandaloneCont, weak_from_raw(this), p, _1);
    return p.future();
  }

  void SessionPrivate::listenStandaloneCont(qi::Promise<void> p, qi::Future<void> f)
  {
    if (f.hasError())
      p.setError(f.error());
    else
    {
      _sdClient.setServiceDirectory(static_cast<ServiceBoundObject*>(_sd._sdbo.get())->object());
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

  std::vector<std::string> Session::loadService(const std::string& name, int flags)
  {
    //TODO: what happens if the session is not connected?
    std::vector<std::string> names = ::qi::loadObject(name, flags);
    for (unsigned int i = 0; i < names.size(); ++i)
      registerService(names[i], createObject(names[i]));
    return names;
  }

}


#ifdef _MSC_VER
# pragma warning( pop )
#endif
