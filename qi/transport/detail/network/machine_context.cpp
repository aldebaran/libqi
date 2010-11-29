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
        machineID(getFirstMacAddress()),
        hostName(getHostName()),
        publicIP(getPrimaryPublicIPAddress()),
        platformID(getPlatform())
        {}
  }
}

