/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <string>

#include <qi/type/typeinterface.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include <qi/url.hpp>
#include <ka/functorcontainer.hpp>
#include <ka/functor.hpp>
#include <ka/empty.hpp>

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

  namespace
  {
    std::string uriParseError(const Url& url)
    {
      return "URI parsing error: '" + url.str() + "' is not a valid URI.";
    }
  }

  void ServiceInfo::setEndpoints(const qi::UrlVector& endpoints) {
    _p->endpoints = ka::fmap(
      [](const Url& url) {
        auto optUri = toUri(url);
        if (ka::empty(optUri))
          throw std::runtime_error(uriParseError(url));
        return ka::src(optUri);
      },
      endpoints);
  }

  void ServiceInfo::setEndpoints(const std::vector<Uri>& endpoints) {
    _p->endpoints = endpoints;
  }

  void ServiceInfo::setSessionId(const std::string& sessionId) {
    _p->sessionId = sessionId;
  }

  void ServiceInfo::addEndpoint(const qi::Url& endpoint) {
    auto optUri = toUri(endpoint);
    if (ka::empty(optUri))
      throw std::runtime_error(uriParseError(endpoint));
    _p->endpoints.push_back(ka::src(optUri));
  }

  void ServiceInfo::addEndpoint(const qi::Uri& endpoint) {
    _p->endpoints.push_back(endpoint);
  }

  void ServiceInfo::setObjectUid(const std::string& newUid)
  {
    _p->objectUid = newUid;
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

  UrlVector ServiceInfo::endpoints() const {
    return ka::fmap(toUrl, _p->endpoints);
  }

  const std::vector<Uri>& ServiceInfo::uriEndpoints() const {
    return _p->endpoints;
  }

  const std::string& ServiceInfo::sessionId() const {
    return _p->sessionId;
  }

  std::string ServiceInfo::objectUid() const {
    return _p->objectUid;
  }


  ServiceInfoPrivate::ServiceInfoPrivate()
    : name()
    , serviceId()
    , machineId()
    , processId()
    , endpoints()
  {}
} // !qi
