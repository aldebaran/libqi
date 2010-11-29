/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#include <qi/transport/detail/network/machine_context.hpp>
#include <qi/transport/detail/network/network.hpp>

namespace qi {
  namespace detail {
    MachineContext::MachineContext():
      hostName(getHostName()),      
      machineID(getFirstMacAddress()),
      publicIP(getPrimaryPublicIPAddress()),
      platformID(getPlatform()) {}

    MachineContext::MachineContext(
      const std::string& pHostName,
      const std::string& pMachineID,
      const std::string& pPublicIPAddress,
      const int&         pPlatformID) :
        hostName(pHostName),
        machineID(pMachineID),
        publicIP(pPublicIPAddress),
        platformID(pPlatformID) {}
  }
}

