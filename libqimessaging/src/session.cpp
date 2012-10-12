/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/session.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
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
  }

  SessionPrivate::~SessionPrivate() {
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

  qi::FutureSync<bool> SessionPrivate::connect(const qi::Url &serviceDirectoryURL)
  {
    if (isConnected()) {
      qiLogInfo("qi.Session") << "Session is already connected";
      return qi::Future<bool>(false);
    }
    _sdSocket = qi::makeTransportSocket(serviceDirectoryURL.protocol());
    if (!_sdSocket)
      return qi::Future<bool>(false);
    _sdSocketConnectedLink    = _sdSocket->connected.connect(boost::bind<void>(&SessionPrivate::onConnected, this));
    _sdSocketDisconnectedLink = _sdSocket->disconnected.connect(boost::bind<void>(&SessionPrivate::onDisconnected, this, _1));
    _sdClient.setTransportSocket(_sdSocket);
    _socketsCache.init();
    return _sdSocket->connect(serviceDirectoryURL);
  }

  qi::FutureSync<void> SessionPrivate::close()
  {
    _serviceHandler.close();
    _serverObject.close();
    _socketsCache.close();
    if (!_sdSocket)
      return qi::Future<void>(0);
    qi::Future<void> fut = _sdSocket->disconnect();
    _sdSocket->connected.disconnect(_sdSocketConnectedLink);
    _sdSocket->disconnected.disconnect(_sdSocketDisconnectedLink);
    _sdSocket.reset();
    return fut;
  }

  bool SessionPrivate::isConnected() const {
    if (!_sdSocket)
      return false;
    return _sdSocket->isConnected();
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
    if (!_p->_sdSocket)
      return qi::Url();
    return _p->_sdSocket->url();
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


  bool Session::listen(const std::string &address)
  {
    return _p->_serverObject.listen(address);
  }

  qi::FutureSync<unsigned int> Session::registerService(const std::string &name, qi::ObjectPtr obj)
  {

    //qi::Promise<unsigned int> promise;
    //qi::Future<unsigned int>  fut =
    return _p->_serverObject.registerService(name, obj);
    //fut.connect(boost::bind<void>(&registerObjectToServer, _1, &_p->_server, promise, obj));
    //return promise.future();
  }

  qi::FutureSync<void> Session::unregisterService(unsigned int idx)
  {
    //qi::Promise<void> promise;
    //qi::Future<void>  fut =
    return _p->_serverObject.unregisterService(idx);
    //fut.connect(boost::bind<void>(&unregisterObjectToServer, _1, &_p->_server, promise, idx));
    //return promise.future();
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

}

