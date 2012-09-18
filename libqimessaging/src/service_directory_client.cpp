/*
** Author(s):
**  -  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "service_directory_client.hpp"
#include <qimessaging/objectbuilder.hpp>
#include "src/service_directory_p.hpp"
#include "src/tcptransportsocket.hpp"

namespace qi {

  static qi::MetaObject serviceDirectoryMetaObject() {
    qi::ObjectBuilder  ob;

    //Do not look at the following 5 lines... and yes I know what I'am doing here.
    ob.advertiseMethod("service",           (ServiceDirectoryPrivate *)0, &ServiceDirectoryPrivate::service);
    ob.advertiseMethod("services",          (ServiceDirectoryPrivate *)0, &ServiceDirectoryPrivate::services);
    ob.advertiseMethod("registerService",   (ServiceDirectoryPrivate *)0, &ServiceDirectoryPrivate::registerService);
    ob.advertiseMethod("unregisterService", (ServiceDirectoryPrivate *)0, &ServiceDirectoryPrivate::unregisterService);
    ob.advertiseMethod("serviceReady",      (ServiceDirectoryPrivate *)0, &ServiceDirectoryPrivate::serviceReady);
    ob.advertiseEvent<void (std::string)>("serviceAdded");
    ob.advertiseEvent<void (std::string)>("serviceRemoved");

    qi::MetaObject m = ob.object().metaObject();
    //verify that we respect the WIRE protocol
    assert(m.methodId("service::(s)") == qi::Message::ServiceDirectoryFunction_Service);
    assert(m.methodId("services::()") == qi::Message::ServiceDirectoryFunction_Services);
    assert(m.methodId("registerService::((sIsI[s]))") == qi::Message::ServiceDirectoryFunction_RegisterService);
    assert(m.methodId("unregisterService::(I)") == qi::Message::ServiceDirectoryFunction_UnregisterService);
    assert(m.methodId("serviceReady::(I)") == qi::Message::ServiceDirectoryFunction_ServiceReady);
    return m;
  }

  ServiceDirectoryClient::ServiceDirectoryClient()
    : _socket(qi::TcpTransportSocketPtr(new TcpTransportSocket()))
    , _object(_socket, qi::Message::Service_ServiceDirectory, serviceDirectoryMetaObject())
  {
    //TODO: add callback on the socket for SessionDisconnected
  }

  ServiceDirectoryClient::~ServiceDirectoryClient()
  {
    boost::mutex::scoped_lock sl(_callbacksMutex);
    _callbacks.clear();
  }

  qi::FutureSync<bool> ServiceDirectoryClient::connect(const qi::Url &serviceDirectoryURL)
  {
    return _socket->connect(serviceDirectoryURL);
  }

  bool ServiceDirectoryClient::isConnected() const {
    return _socket->isConnected();
  }


  void ServiceDirectoryClient::addCallbacks(SessionInterface *delegate, void *data)
  {
    if (!delegate) {
      qiLogError("qimessaging.Session") << "Trying to set invalid callback on the session.";
      return;
    }
    {
      boost::mutex::scoped_lock l(_callbacksMutex);
      _callbacks.push_back(std::make_pair(delegate, data));
    }
  }

  void ServiceDirectoryClient::removeCallbacks(SessionInterface *delegate)
  {
    if (!delegate) {
      qiLogError("qimessaging.Session") << "Trying to erase invalid callback on the session.";
      return;
    }
    {
      boost::mutex::scoped_lock l(_callbacksMutex);
      std::vector< std::pair<SessionInterface *, void *> >::iterator it;
      for (it = _callbacks.begin(); it != _callbacks.end(); ++it)
      {
        if (it->first == delegate) {
          _callbacks.erase(it);
          break;
        }
      }
    }
  }

  void ServiceDirectoryClient::onSocketDisconnected(TransportSocketPtr client, void *data)
  {
    std::vector< std::pair<qi::SessionInterface *, void *> > localCallbacks;
    {
      boost::mutex::scoped_lock l(_callbacksMutex);
      localCallbacks = _callbacks;
    }
    std::vector< std::pair<qi::SessionInterface *, void *> >::const_iterator it;
    for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
      it->first->onSessionDisconnected(_session, it->second);
  }

  qi::FutureSync<void> ServiceDirectoryClient::disconnect() {
    return _socket->disconnect();
  }

  qi::Url ServiceDirectoryClient::url() const {
    return _socket->url();
  }

  qi::Future< std::vector<ServiceInfo> > ServiceDirectoryClient::services() {
    return _object.call< std::vector<ServiceInfo> >("services");
  }

  qi::Future<ServiceInfo>              ServiceDirectoryClient::service(const std::string &name) {
    return _object.call< ServiceInfo >("service", name);
  }

  qi::Future<unsigned int>             ServiceDirectoryClient::registerService(const ServiceInfo &svcinfo) {
    return _object.call< unsigned int >("registerService", svcinfo);
  }

  qi::Future<void>                     ServiceDirectoryClient::unregisterService(const unsigned int &idx) {
    return _object.call<void>("unregisterService", idx);
  }

  qi::Future<void>                     ServiceDirectoryClient::serviceReady(const unsigned int &idx) {
    return _object.call<void>("serviceReady", idx);
  }


}
