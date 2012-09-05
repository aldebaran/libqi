/*
** Author(s):
**  -  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "service_directory_client.hpp"
#include <qimessaging/metaobjectbuilder.hpp>
#include "src/service_directory_p.hpp"

namespace qi {

  static qi::MetaObject *serviceDirectoryMetaObject() {
    qi::MetaObject * mo = new qi::MetaObject();

    qi::MetaObjectBuilder  mob(mo);

    //Do not look at the following 5 lines... and yes I know what I'am doing here.
    mob.advertiseMethod("__metaObject",      (qi::Object *)0, &ServiceDirectoryPrivate::metaObject);
    mob.advertiseMethod("service",           (ServiceDirectoryPrivate *)0, &ServiceDirectoryPrivate::service);
    mob.advertiseMethod("services",          (ServiceDirectoryPrivate *)0, &ServiceDirectoryPrivate::services);
    mob.advertiseMethod("registerService",   (ServiceDirectoryPrivate *)0, &ServiceDirectoryPrivate::registerService);
    mob.advertiseMethod("unregisterService", (ServiceDirectoryPrivate *)0, &ServiceDirectoryPrivate::unregisterService);
    mob.advertiseMethod("serviceReady",      (ServiceDirectoryPrivate *)0, &ServiceDirectoryPrivate::serviceReady);
    mob.advertiseEvent<void (std::string)>("serviceAdded");
    mob.advertiseEvent<void (std::string)>("serviceRemoved");

    //verify that we respect the WIRE protocol
    assert(mo->methodId("service::(s)") == qi::Message::ServiceDirectoryFunction_Service);
    assert(mo->methodId("services::()") == qi::Message::ServiceDirectoryFunction_Services);
    assert(mo->methodId("registerService::((sIsI[s]))") == qi::Message::ServiceDirectoryFunction_RegisterService);
    assert(mo->methodId("unregisterService::(I)") == qi::Message::ServiceDirectoryFunction_UnregisterService);
    assert(mo->methodId("serviceReady::(I)") == qi::Message::ServiceDirectoryFunction_ServiceReady);
    return mo;
  }

  ServiceDirectoryClient::ServiceDirectoryClient()
    : _socket(new qi::TransportSocket)
    , _object(_socket, qi::Message::Service_ServiceDirectory, serviceDirectoryMetaObject())
  {
  }

  ServiceDirectoryClient::~ServiceDirectoryClient()
  {
    boost::mutex::scoped_lock sl(_callbacksMutex);
    _callbacks.clear();
    _object._ts = 0;
    //do not delete _socket it is deleted by _object at the moment.
  }

  bool ServiceDirectoryClient::connect(const qi::Url &serviceDirectoryURL)
  {
    return _socket->connect(serviceDirectoryURL);
  }

  bool ServiceDirectoryClient::waitForConnected(int msecs)
  {
    return _socket->waitForConnected(msecs);
  }

  bool ServiceDirectoryClient::waitForDisconnected(int msecs)
  {
    return _socket->waitForDisconnected(msecs);
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
  void ServiceDirectoryClient::onSocketDisconnected(TransportSocket *client, void *data)
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

  void ServiceDirectoryClient::disconnect() {
    _socket->disconnect();
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
