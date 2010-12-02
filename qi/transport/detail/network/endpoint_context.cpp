/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/detail/network/endpoint_context.hpp>
#include <qi/transport/detail/network/network.hpp>

namespace qi {
  namespace detail {
    EndpointContext::EndpointContext():
      type(CLIENT_ENDPOINT),
      name(""),
      endpointID(getUUID()),
      contextID(""),
      machineID(getFirstMacAddress()),
      processID(getProcessID()),
      port(0) {}

      EndpointContext::EndpointContext(
        const EndpointType& pType,
        const std::string& pName,
        const std::string& pEndpointID,
        const std::string& pContextID,
        const std::string& pMachineID,
        const int& pProcessID,
        const int& pPort) :
        type(pType),
        name(pName),
        endpointID(pEndpointID),
        contextID(pContextID),
        machineID(pMachineID),
        processID(pProcessID),
        port(pPort) {}
  }
}

