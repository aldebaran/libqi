/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>
#include <vector>
#include <map>
#include <set>

#include <qitype/genericobject.hpp>
#include <qimessaging/transportserver.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qitype/objecttypebuilder.hpp>
#include "transportserver_p.hpp"
#include "serverresult.hpp"
#include "session_p.hpp"
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qimessaging/url.hpp>
#include "servicedirectory_p.hpp"
#include "server.hpp"

namespace qi
{


  ServiceDirectoryPrivate::ServiceDirectoryPrivate()
    : _sdbo(boost::shared_ptr<ServiceDirectoryBoundObject>(new ServiceDirectoryBoundObject))
  {
    _server.addObject(1, _sdbo);
  }

  ServiceDirectoryPrivate::~ServiceDirectoryPrivate()
  {
  }


  qi::ObjectPtr createSDP(ServiceDirectoryBoundObject* self) {
    qi::ObjectTypeBuilder<ServiceDirectoryBoundObject> ob;

    ob.advertiseMethod("service", &ServiceDirectoryBoundObject::service);
    ob.advertiseMethod("services", &ServiceDirectoryBoundObject::services);
    ob.advertiseMethod("registerService", &ServiceDirectoryBoundObject::registerService);
    ob.advertiseMethod("unregisterService", &ServiceDirectoryBoundObject::unregisterService);
    ob.advertiseMethod("serviceReady", &ServiceDirectoryBoundObject::serviceReady);
    ob.advertiseEvent("serviceAdded", &ServiceDirectoryBoundObject::serviceAdded);
    ob.advertiseEvent("serviceRemoved", &ServiceDirectoryBoundObject::serviceRemoved);
    return ob.object(self);
  }

  ServiceDirectoryBoundObject::ServiceDirectoryBoundObject()
    : ServiceBoundObject(1, createSDP(this), qi::MetaCallType_Direct)
    , servicesCount(0)
  {
  }

  ServiceDirectoryBoundObject::~ServiceDirectoryBoundObject()
  {
  }

  void ServiceDirectoryBoundObject::onSocketDisconnected(TransportSocketPtr socket, int error)
  {
    // if services were connected behind the socket
    std::map<TransportSocketPtr, std::vector<unsigned int> >::iterator it;
    it = socketToIdx.find(socket);
    if (it == socketToIdx.end()) {
      return;
    }
    // Copy the vector, iterators will be invalidated.
    std::vector<unsigned int> ids = it->second;
    // Always start at the beginning since we erase elements on unregisterService
    // and mess up the iterator
    for (std::vector<unsigned int>::iterator it2 = ids.begin();
         it2 != ids.end();
         ++it2)
    {
      qiLogInfo("qimessaging.ServiceDirectory") << "Service #" << *it2 << " disconnected";
      try {
        unregisterService(*it2);
      } catch (std::runtime_error &) {
        qiLogWarning("ServiceDirectory") << "Cannot unregister service #" << *it2;
      }
    }
    socketToIdx.erase(it);
    ServiceBoundObject::onSocketDisconnected(socket, error);
  }

  std::vector<ServiceInfo> ServiceDirectoryBoundObject::services()
  {
    std::vector<ServiceInfo> result;
    std::map<unsigned int, ServiceInfo>::const_iterator it;

    for (it = connectedServices.begin(); it != connectedServices.end(); ++it)
      result.push_back(it->second);

    return result;
  }

  ServiceInfo ServiceDirectoryBoundObject::service(const std::string &name)
  {
    std::map<unsigned int, ServiceInfo>::const_iterator servicesIt;
    std::map<std::string, unsigned int>::const_iterator it;

    it = nameToIdx.find(name);
    if (it == nameToIdx.end()) {
      std::stringstream ss;
      ss << "Cannot find service '" << name << "' in index";
      throw std::runtime_error(ss.str());
    }

    unsigned int idx = it->second;

    servicesIt = connectedServices.find(idx);
    if (servicesIt == connectedServices.end()) {
      std::stringstream ss;
      ss << "Cannot find ServiceInfo for service '" << name << "'";
      throw std::runtime_error(ss.str());
    }
    return servicesIt->second;
  }

  unsigned int ServiceDirectoryBoundObject::registerService(const ServiceInfo &svcinfo)
  {
    std::map<std::string, unsigned int>::iterator it;
    it = nameToIdx.find(svcinfo.name());
    if (it != nameToIdx.end())
    {
      std::stringstream ss;
      ss << "Service \"" <<
        svcinfo.name() <<
        "\" (#" << it->second << ") is already registered. " <<
        "Rejecting conflicting registration attempt.";
      qiLogWarning("qimessaging.ServiceDirectory")  << ss.str();
      throw std::runtime_error(ss.str());
    }

    unsigned int idx = ++servicesCount;
    nameToIdx[svcinfo.name()] = idx;
    // Do not add serviceDirectory on the map (socket() == null)
    if (idx != qi::Message::Service_ServiceDirectory)
    {
      socketToIdx[currentSocket()].push_back(idx);
    }
    pendingServices[idx] = svcinfo;
    pendingServices[idx].setServiceId(idx);

    qiLogInfo("qimessaging.ServiceDirectory")  << "Registered Service \"" << svcinfo.name() << "\" (#" << idx << ")";
    qi::UrlVector::const_iterator jt;
    for (jt = svcinfo.endpoints().begin(); jt != svcinfo.endpoints().end(); ++jt)
    {
      qiLogDebug("qimessaging.ServiceDirectory") << "Service \"" << svcinfo.name() << "\" is now on " << jt->str();
    }

    return idx;
  }

  void ServiceDirectoryBoundObject::unregisterService(const unsigned int &idx)
  {
    // search the id before accessing it
    // otherwise operator[] create a empty entry
    std::map<unsigned int, ServiceInfo>::iterator it2;
    it2 = connectedServices.find(idx);
    if (it2 == connectedServices.end())
    {
      std::stringstream ss;
      ss << "Unregister Service: Can't find service #" << idx;
      qiLogError("qimessaging.ServiceDirectory") << ss.str();
      throw std::runtime_error(ss.str());
    }

    std::map<std::string, unsigned int>::iterator it;
    it = nameToIdx.find(connectedServices[idx].name());
    if (it == nameToIdx.end())
    {
      std::stringstream ss;
      ss << "Unregister Service: Mapping error, service #" << idx << " not in nameToIdx";
      qiLogError("qimessaging.ServiceDirectory") << ss.str();
      throw std::runtime_error(ss.str());
    }
    std::string serviceName = it2->second.name();
    qiLogInfo("qimessaging.ServiceDirectory") <<
      "Unregistered Service  \""
      << serviceName
      << "\" (#" << idx << ")";
    nameToIdx.erase(it);
    connectedServices.erase(it2);

    // Find and remove serviceId into socketToIdx map
    {
      std::map<TransportSocketPtr , std::vector<unsigned int> >::iterator it;
      for (it = socketToIdx.begin(); it != socketToIdx.end(); ++it) {
        std::vector<unsigned int>::iterator jt;
        for (jt = it->second.begin(); jt != it->second.end(); ++jt) {
          if (*jt == idx) {
            it->second.erase(jt);
            //socketToIdx is erased by onSocketDisconnected
            break;
          }
        }
      }
    }
    serviceRemoved(idx, serviceName);
  }

  void ServiceDirectoryBoundObject::serviceReady(const unsigned int &idx)
  {
    // search the id before accessing it
    // otherwise operator[] create a empty entry
    std::map<unsigned int, ServiceInfo>::iterator itService;
    itService = pendingServices.find(idx);
    if (itService == pendingServices.end())
    {
      std::stringstream ss;
      ss << "Can't find pending service #" << idx;
      qiLogError("qimessaging.ServiceDirectory") << ss.str();
      throw std::runtime_error(ss.str());
    }

    std::string serviceName = itService->second.name();
    connectedServices[idx] = itService->second;
    pendingServices.erase(itService);

    serviceAdded(idx, serviceName);
  }

ServiceDirectory::ServiceDirectory()
  : _p(new ServiceDirectoryPrivate())
{
}

ServiceDirectory::~ServiceDirectory()
{
  close();
  delete _p;
}

bool ServiceDirectory::listen(const qi::Url &address)
{
  bool b = _p->_server.listen(address);
  if (!b)
    return false;

  ServiceDirectoryBoundObject *sdbo = static_cast<ServiceDirectoryBoundObject*>(_p->_sdbo.get());

  ServiceInfo si;
  si.setName("ServiceDirectory");
  si.setServiceId(1);
  si.setMachineId(qi::os::getMachineId());
  si.setEndpoints(_p->_server.endpoints());
  unsigned int regid = sdbo->registerService(si);
  sdbo->serviceReady(1);
  //serviceDirectory must have id '1'
  assert(regid == 1);
  qiLogInfo("ServiceDirectory") << "ServiceDirectory listening on: " << address.str();
  return true;
}

qi::UrlVector ServiceDirectory::endpoints() const {
  return _p->_server.endpoints();
}
void ServiceDirectory::close() {
  qiLogInfo("ServiceDirectory") << "Closing ServiceDirectory";
  _p->_server.close();
}

qi::Url ServiceDirectory::listenUrl() const {
  return _p->_server.listenUrl();
}


}; // !qi
