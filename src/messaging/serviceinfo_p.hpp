#pragma once

#ifndef _QI_MESSAGING_SERVICEINFO_P_HPP_
#define _QI_MESSAGING_SERVICEINFO_P_HPP_

#include <qi/messaging/serviceinfo.hpp>
#include <string>

namespace qi
{

class ServiceInfoPrivate
{
public:
  ServiceInfoPrivate();

  std::string name;
  unsigned int serviceId;
  std::string machineId;
  unsigned int processId;
  qi::UrlVector endpoints;
  std::string sessionId;
  boost::optional<qi::ObjectUid> objectUid;
};

}

#endif // _QI_MESSAGING_SERVICEINFO_P_HPP_
