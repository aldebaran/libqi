/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "servicedirectoryclient.hpp"
#include <qitype/objecttypebuilder.hpp>
#include "servicedirectory_p.hpp"
#include "tcptransportsocket.hpp"

namespace qi {

  static qi::MetaObject serviceDirectoryMetaObject() {
    qi::ObjectTypeBuilder<ServiceDirectoryBoundObject> ob;

    ob.advertiseMethod("service",           &ServiceDirectoryBoundObject::service,           qi::Message::ServiceDirectoryFunction_Service);
    ob.advertiseMethod("services",          &ServiceDirectoryBoundObject::services,          qi::Message::ServiceDirectoryFunction_Services);
    ob.advertiseMethod("registerService",   &ServiceDirectoryBoundObject::registerService,   qi::Message::ServiceDirectoryFunction_RegisterService);
    ob.advertiseMethod("unregisterService", &ServiceDirectoryBoundObject::unregisterService, qi::Message::ServiceDirectoryFunction_UnregisterService);
    ob.advertiseMethod("serviceReady",      &ServiceDirectoryBoundObject::serviceReady,      qi::Message::ServiceDirectoryFunction_ServiceReady);
    ob.advertiseEvent("serviceAdded"  , &ServiceDirectoryBoundObject::serviceAdded);
    ob.advertiseEvent("serviceRemoved", &ServiceDirectoryBoundObject::serviceRemoved);

    qi::MetaObject m = ob.metaObject();
    //verify that we respect the WIRE protocol
    assert(m.methodId("service::(s)") == qi::Message::ServiceDirectoryFunction_Service);
    assert(m.methodId("services::()") == qi::Message::ServiceDirectoryFunction_Services);
    assert(m.methodId("registerService::((sIsI[s]))") == qi::Message::ServiceDirectoryFunction_RegisterService);
    assert(m.methodId("unregisterService::(I)") == qi::Message::ServiceDirectoryFunction_UnregisterService);
    assert(m.methodId("serviceReady::(I)") == qi::Message::ServiceDirectoryFunction_ServiceReady);
    return m;
  }

  ServiceDirectoryClient::ServiceDirectoryClient()
    : _remoteObject(qi::Message::Service_ServiceDirectory, serviceDirectoryMetaObject())
  {
    _object = makeDynamicObjectPtr(&_remoteObject, false);
  }

  ServiceDirectoryClient::~ServiceDirectoryClient()
  {
  }

  qi::FutureSync<bool> ServiceDirectoryClient::connect(const qi::Url &serviceDirectoryURL) {
    if (isConnected()) {
      qiLogInfo("qi.Session") << "Session is already connected";
      return qi::Future<bool>(false);
    }
    _sdSocket = qi::makeTransportSocket(serviceDirectoryURL.protocol());
    if (!_sdSocket)
      return qi::Future<bool>(false);
    _sdSocketConnectedLink    = _sdSocket->connected.connect(boost::bind<void>(&ServiceDirectoryClient::onSocketConnected, this));
    _sdSocketDisconnectedLink = _sdSocket->disconnected.connect(boost::bind<void>(&ServiceDirectoryClient::onSocketDisconnected, this, _1));
    _remoteObject.setTransportSocket(_sdSocket);
    return _sdSocket->connect(serviceDirectoryURL);
  }


  static void sharedPtrHolder(TransportSocketPtr ptr)
  {
  }

  qi::FutureSync<void> ServiceDirectoryClient::close() {
    if (!_sdSocket)
      return qi::Future<void>(0);
    qi::Future<void> fut = _sdSocket->disconnect();
    // Hold the socket shared ptr alive until the future returns.
    // otherwise, the destructor will block us until disconnect terminates
    fut.connect(boost::bind(&sharedPtrHolder, _sdSocket));
    _sdSocket->connected.disconnect(_sdSocketConnectedLink);
    _sdSocket->disconnected.disconnect(_sdSocketDisconnectedLink);
    _sdSocket.reset();
    return fut;
  }

  bool                 ServiceDirectoryClient::isConnected() const {
    return _sdSocket == 0 ? false : _sdSocket->isConnected();
  }

  qi::Url              ServiceDirectoryClient::url() const {
    return _sdSocket->url();
  }

  void ServiceDirectoryClient::onSocketConnected() {
    connected();
  }

  void ServiceDirectoryClient::onSocketDisconnected(int error) {
    disconnected(error);
  }

  qi::Future< std::vector<ServiceInfo> > ServiceDirectoryClient::services() {
    return _object->call< std::vector<ServiceInfo> >("services");
  }

  qi::Future<ServiceInfo>              ServiceDirectoryClient::service(const std::string &name) {
    return _object->call< ServiceInfo >("service", name);
  }

  qi::Future<unsigned int>             ServiceDirectoryClient::registerService(const ServiceInfo &svcinfo) {
    return _object->call< unsigned int >("registerService", svcinfo);
  }

  qi::Future<void>                     ServiceDirectoryClient::unregisterService(const unsigned int &idx) {
    return _object->call<void>("unregisterService", idx);
  }

  qi::Future<void>                     ServiceDirectoryClient::serviceReady(const unsigned int &idx) {
    return _object->call<void>("serviceReady", idx);
  }


}
