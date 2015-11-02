#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SERVICEINFO_HPP_
#define _QIMESSAGING_SERVICEINFO_HPP_

#include <string>
#include <vector>

#include <qi/api.hpp>
#include <qi/url.hpp>
#include <qi/type/typeinterface.hpp>

namespace qi
{
  class ServiceInfoPrivate;
  class QI_API ServiceInfo {
  public:
    ServiceInfo();
    ServiceInfo(const ServiceInfo& svcinfo);

    ~ServiceInfo();

    ServiceInfo& operator= (const ServiceInfo& svcinfo);

    void setName(const std::string& name);
    void setServiceId(unsigned int serviceId);
    void setMachineId(const std::string& machineId);
    void setProcessId(unsigned int processId);
    void setEndpoints(const qi::UrlVector& endpoints);
    void addEndpoint(const qi::Url& endpoint);
    void setSessionId(const std::string& sessionId);

    const std::string& name() const;
    unsigned int serviceId() const;
    const std::string& machineId() const;
    unsigned int processId() const;
    const qi::UrlVector& endpoints() const;
    const std::string& sessionId() const;

    ServiceInfoPrivate* _p;

  protected:
    friend class TypeImpl<ServiceInfo>;
  };

  using ServiceInfoVector = std::vector<qi::ServiceInfo>;
} // !qi

#endif  // _QIMESSAGING_SERVICEINFO_HPP_
