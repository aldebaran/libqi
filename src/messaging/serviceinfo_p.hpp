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
  std::vector<Uri> endpoints;
  std::string sessionId;
  std::string objectUid; // TODO: Revert to use `boost::optional<qi::ObjectUid>` once retro-compatibility is not an issue anymore.
};

}

#endif // _QI_MESSAGING_SERVICEINFO_P_HPP_
