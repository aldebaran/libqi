/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <string>

#include <qi/type/typeinterface.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include <qi/url.hpp>

#include "serviceinfo_p.hpp"

namespace qi
{
  ServiceInfo::ServiceInfo()
    : _p(new ServiceInfoPrivate())
  {
  }

  ServiceInfo::ServiceInfo(const ServiceInfo& svcinfo)
    : _p(new ServiceInfoPrivate())
  {
    *_p = *(svcinfo._p);
  }

  ServiceInfo& ServiceInfo::operator= (const ServiceInfo& svcinfo) {
    *_p = *(svcinfo._p);
    return (*this);
  }

  ServiceInfo::~ServiceInfo() {
    delete _p;
  }

  void ServiceInfo::setName(const std::string& name) {
    _p->name = name;
  }

  void ServiceInfo::setServiceId(unsigned int serviceId) {
    _p->serviceId = serviceId;
  }

  void ServiceInfo::setMachineId(const std::string& machineId) {
    _p->machineId = machineId;
  }

  void ServiceInfo::setProcessId(unsigned int processId) {
    _p->processId = processId;
  }

  void ServiceInfo::setEndpoints(const qi::UrlVector& endpoints) {
    _p->endpoints = endpoints;
  }

  void ServiceInfo::setSessionId(const std::string& sessionId) {
    _p->sessionId = sessionId;
  }

  void ServiceInfo::addEndpoint(const qi::Url& endpoint) {
    _p->endpoints.push_back(endpoint);
  }

  const std::string& ServiceInfo::name() const {
    return _p->name;
  }

  unsigned int ServiceInfo::serviceId() const {
    return _p->serviceId;
  }

  const std::string& ServiceInfo::machineId() const {
    return _p->machineId;
  }

  unsigned int ServiceInfo::processId() const {
    return _p->processId;
  }

  const qi::UrlVector& ServiceInfo::endpoints() const {
    return _p->endpoints;
  }

  const std::string& ServiceInfo::sessionId() const {
    return _p->sessionId;
  }

  ServiceInfoPrivate::ServiceInfoPrivate()
    : name()
    , serviceId()
    , machineId()
    , processId()
    , endpoints()
  {}
} // !qi
