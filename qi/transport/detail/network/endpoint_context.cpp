/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#include <qi/transport/detail/network/endpoint_context.hpp>
#include <qi/transport/detail/network/network.hpp>

namespace qi {
  namespace detail {

    EndpointContext::EndpointContext():
        endpointID(getUUID()),
        machineID(getFirstMacAddress()),
        hostName(getHostName()),
        processID(getProcessID()),
        platformID(getPlatform()),
        publicIP(getPrimaryPublicIPAddress()),
        name(""),
        contextID(""),
        port(0)
        {}
  }
}

