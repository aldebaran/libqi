/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#pragma warning(disable: 4355)

#include <qimessaging/session.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qitype/genericobject.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qitype/objectfactory.hpp>
#include "remoteobject_p.hpp"
#include "session_p.hpp"

namespace qi {


  SessionPrivate::SessionPrivate(qi::Session *session)
    : _self(session)
    , _sdClient()
    , _serverObject(&_sdClient)
    , _serviceHandler(&_socketsCache, &_sdClient, &_serverObject)
    , _servicesHandler(&_sdClient, &_serverObject)
    , _watcher(session)
  {
    _sdClientConnectedLink    = _sdClient.connected.connect(boost::bind<void>(&SessionPrivate::onConnected, this));
    _sdClientDisconnectedLink = _sdClient.disconnected.connect(boost::bind<void>(&SessionPrivate::onDisconnected, this, _1));
    _sdClientServiceAddedLink = _sdClient.serviceAdded.connect(boost::bind<void>(&SessionPrivate::onServiceAdded, this, _1, _2));
    _sdClientServiceRemovedLink = _sdClient.serviceRemoved.connect(boost::bind<void>(&SessionPrivate::onServiceRemoved, this, _1, _2));
  }

  SessionPrivate::~SessionPrivate() {
    _sdClient.connected.disconnect(_sdClientConnectedLink);
    _sdClient.disconnected.disconnect(_sdClientDisconnectedLink);
    _sdClient.serviceAdded.disconnect(_sdClientServiceAddedLink);
    _sdClient.serviceRemoved.disconnect(_sdClientServiceRemovedLink);
    _self->disconnected.disconnectAll();
    _self->connected.disconnectAll();
    close();
  }


  void SessionPrivate::onConnected() {
    _self->connected();
  }

  void SessionPrivate::onDisconnected(int error) {
    _self->disconnected(error);
  }

  void SessionPrivate::onServiceAdded(unsigned int idx, const std::string &name) {
    _self->serviceRegistered(idx, name);
  }

  void SessionPrivate::onServiceRemoved(unsigned int idx, const std::string &name) {
    _self->serviceUnregistered(idx, name);
  }

  qi::FutureSync<bool> SessionPrivate::connect(const qi::Url &serviceDirectoryURL)
  {
    if (isConnected()) {
      qiLogInfo("qi.Session") << "Session is already connected";
      return qi::Future<bool>(false);
    }
    _socketsCache.init();
    return _sdClient.connect(serviceDirectoryURL);
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
    close();
    delete _p;
  }


  // ###### Client
  qi::FutureSync<bool> Session::connect(const qi::Url &serviceDirectoryURL)
  {
    return _p->connect(serviceDirectoryURL);
  }

  qi::FutureSync<void> Session::close() {
    return _p->close();
  }

  bool Session::isConnected() const {
    return _p->isConnected();
  }

  qi::Url Session::url() const {
    return _p->_sdClient.url();
  }

  bool Session::waitForServiceReady(const std::string &service, int msecs) {
    return _p->_watcher.waitForServiceReady(service, msecs);
  }

  //3 cases:
  //  - local service => just return the vector
  //  - remote => ask the sd return the result
  //  - all => ask the sd, append local services, return the result
  qi::Future< std::vector<ServiceInfo> > Session::services(ServiceLocality locality)
  {
    return _p->_servicesHandler.services(locality);
  }

  qi::Future< qi::ObjectPtr > Session::service(const std::string &service,
                                               ServiceLocality locality)
  {
    return _p->_serviceHandler.service(service, locality);
  }


  bool Session::listen(const qi::Url &address)
  {
    return _p->_serverObject.listen(address);
  }

  qi::FutureSync<unsigned int> Session::registerService(const std::string &name, qi::ObjectPtr obj)
  {
    if (!listenUrl().isValid()) {
      qi::Url listeningAddress("tcp://0.0.0.0:0");
      qiLogVerbose("Session listening on ") << listeningAddress.str() << "." << std::endl;
      listen(listeningAddress);
    }

    return _p->_serverObject.registerService(name, obj);
  }

  qi::FutureSync<void> Session::unregisterService(unsigned int idx)
  {
    return _p->_serverObject.unregisterService(idx);
  }

  qi::Url Session::listenUrl() const
  {
    return _p->_serverObject.listenUrl();
  }

  std::vector<std::string> Session::loadService(const std::string& name, int flags)
  {
    std::vector<std::string> names = ::qi::loadObject(name, flags);
    for (unsigned int i = 0; i < names.size(); ++i)
      registerService(names[i], createObject(names[i]));
    return names;
  }

  namespace details {

    // Only needed for ALModule.
    // (metacall should be direct when called in local, and threaded when coming from a server)
    void setSessionServerDefaultCallType(qi::Session *session, qi::MetaCallType callType) {
      session->_p->_serverObject.setDefaultCallType(callType);
    }
  };

}

