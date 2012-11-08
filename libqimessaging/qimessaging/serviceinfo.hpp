#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SERVICEINFO_HPP_
#define _QIMESSAGING_SERVICEINFO_HPP_

#include <string>
#include <vector>

#include <qitype/genericvalue.hpp>
#include <qitype/signature.hpp>
#include <qimessaging/api.hpp>
#include <qimessaging/datastream.hpp>

namespace qi
{
  class ServiceInfoPrivate;
  class QIMESSAGING_API ServiceInfo {
  public:
    ServiceInfo();
    ServiceInfo(const ServiceInfo& svcinfo);

    ~ServiceInfo();

    ServiceInfo& operator= (const ServiceInfo& svcinfo);

    void setName(const std::string& name);
    void setServiceId(unsigned int serviceId);
    void setMachineId(const std::string& machineId);
    void setProcessId(unsigned int processId);
    void setEndpoints(const std::vector<std::string>& endpoints);
    void addEndpoint(const std::string& endpoints);

    const std::string& name() const;
    unsigned int serviceId() const;
    const std::string& machineId() const;
    unsigned int processId() const;
    const std::vector<std::string>& endpoints() const;

    ServiceInfoPrivate* _p;

  protected:
    friend class TypeImpl<ServiceInfo>;
  };

  typedef std::vector<qi::ServiceInfo> ServiceInfoVector;
} // !qi

#endif  // _QIMESSAGING_SERVICEINFO_HPP_
