/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/session.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qimessaging/objectfactory.hpp>
#include "src/remoteobject_p.hpp"
#include "src/session_p.hpp"
#include "src/object_p.hpp"

namespace qi {


  SessionPrivate::SessionPrivate(qi::Session *session)
    : _self(session)
    , _sdClient()
    , _server(&_sdClient)
    , _serviceHandler(&_socketsCache, &_sdClient, &_server)
    , _servicesHandler(&_sdClient, &_server)
    , _watcher(session)
  {
  }

  SessionPrivate::~SessionPrivate() {
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
    return _sdSocket->connect(serviceDirectoryURL);
  }

  qi::FutureSync<void> SessionPrivate::close()
  {
    _serviceHandler.close();
    _server.close();
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

  qi::Future< qi::GenericObject > Session::service(const std::string &service,
                                            ServiceLocality locality,
                                            const std::string &type)
  {
    return _p->_serviceHandler.service(service, locality, type);
  }


  bool Session::listen(const std::string &address)
  {
    return _p->_server.listen(address);
  }

  qi::FutureSync<unsigned int> Session::registerService(const std::string &name,
                                                        const qi::GenericObject  &obj)
  {
    return _p->_server.registerService(name, obj);
  }

  qi::FutureSync<void> Session::unregisterService(unsigned int idx)
  {
    return _p->_server.unregisterService(idx);
  }

  qi::Url Session::listenUrl() const
  {
    return _p->_server.listenUrl();
  }

  std::vector<std::string> Session::loadService(const std::string& name, int flags)
  {
    std::vector<std::string> names = ::qi::loadObject(name, flags);
    for (unsigned int i = 0; i < names.size(); ++i)
      registerService(names[i], createObject(names[i]));
    return names;
  }

}

