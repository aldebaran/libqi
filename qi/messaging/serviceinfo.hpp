#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SERVICEINFO_HPP_
#define _QIMESSAGING_SERVICEINFO_HPP_

#include <string>
#include <vector>

#include <boost/optional.hpp>

#include <qi/api.hpp>
#include <qi/url.hpp>
#include <qi/uri.hpp>
#include <qi/type/typeinterface.hpp>
#include <qi/objectuid.hpp>

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
    /// @throws `std::runtime_error` if any URL is not compliant with RFC 3986.
    void setEndpoints(const qi::UrlVector& endpoints);
    void setEndpoints(const std::vector<Uri>& endpoints);
    /// @throws `std::runtime_error` if the URL is not compliant with RFC 3986.
    void addEndpoint(const qi::Url& endpoint);
    void addEndpoint(const Uri& endpoint);
    void setSessionId(const std::string& sessionId);
    void setObjectUid(const std::string& newUid);

    const std::string& name() const;
    unsigned int serviceId() const;
    const std::string& machineId() const;
    unsigned int processId() const;
    qi::UrlVector endpoints() const;
    const std::vector<Uri>& uriEndpoints() const;
    const std::string& sessionId() const;

    std::string objectUid() const;

    ServiceInfoPrivate* _p;

  protected:
    friend class TypeImpl<ServiceInfo>;
  };

  using ServiceInfoVector = std::vector<qi::ServiceInfo>;
} // !qi

#endif  // _QIMESSAGING_SERVICEINFO_HPP_
