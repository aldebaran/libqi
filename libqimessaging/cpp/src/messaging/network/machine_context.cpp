/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/messaging/network/machine_context.hpp"
#include "src/messaging/network/network.hpp"

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
      const Platform&    pPlatformID) :
        hostName(pHostName),
        machineID(pMachineID),
        publicIP(pPublicIPAddress),
        platformID(pPlatformID) {}
  }
}

