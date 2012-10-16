/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "servicedirectoryclient.hpp"
#include <qitype/objecttypebuilder.hpp>
#include "src/servicedirectory_p.hpp"
#include "src/tcptransportsocket.hpp"

namespace qi {

  static qi::MetaObject serviceDirectoryMetaObject() {
    qi::ObjectTypeBuilder<ServiceDirectoryBoundObject> ob;
    ObjectTypeBuilderBase::SignalMemberGetter dummy;

    ob.advertiseMethod("service",           &ServiceDirectoryBoundObject::service,           qi::Message::ServiceDirectoryFunction_Service);
    ob.advertiseMethod("services",          &ServiceDirectoryBoundObject::services,          qi::Message::ServiceDirectoryFunction_Services);
    ob.advertiseMethod("registerService",   &ServiceDirectoryBoundObject::registerService,   qi::Message::ServiceDirectoryFunction_RegisterService);
    ob.advertiseMethod("unregisterService", &ServiceDirectoryBoundObject::unregisterService, qi::Message::ServiceDirectoryFunction_UnregisterService);
    ob.advertiseMethod("serviceReady",      &ServiceDirectoryBoundObject::serviceReady,      qi::Message::ServiceDirectoryFunction_ServiceReady);
    ob.advertiseEvent<void (std::string)>("serviceAdded", dummy);
    ob.advertiseEvent<void (std::string)>("serviceRemoved", dummy);

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

  void ServiceDirectoryClient::setTransportSocket(qi::TransportSocketPtr socket) {
    _remoteObject.setTransportSocket(socket);
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
