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
    : _sdClient()
    , _server(&_sdClient)
    , _serviceHandler(&_socketsCache, &_sdClient, &_server)
    , _servicesHandler(&_sdClient, &_server)
    , _watcher(session)
  {
  }

  SessionPrivate::~SessionPrivate() {
    if (_sdSocket)
      _sdSocket->disconnect();
    _server.close();
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
    _p->_sdSocket = qi::makeTransportSocket(serviceDirectoryURL.protocol());
    if (!_p->_sdSocket)
      return qi::Future<bool>(false);
    _p->_sdClient.setTransportSocket(_p->_sdSocket);
    return _p->_sdSocket->connect(serviceDirectoryURL);
  }

  bool Session::isConnected() const {
    if (!_p->_sdSocket)
      return false;
    return _p->_sdSocket->isConnected();
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

  qi::FutureSync<void> Session::close()
  {
    _p->_serviceHandler.close();
    _p->_server.close();
    if (!_p->_sdSocket)
      return qi::Future<void>(0);
    return _p->_sdSocket->disconnect();
  }

  qi::FutureSync<unsigned int> Session::registerService(const std::string &name,
                                                        const qi::GenericObject  &obj)
  {
    return _p->_server.registerService(name, obj);
  }

  qi::Future<void> Session::unregisterService(unsigned int idx)
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

