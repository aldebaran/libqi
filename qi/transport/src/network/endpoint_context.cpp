/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/messaging/src/network/endpoint_context.hpp>
#include <qi/transport/messaging/src/network/network.hpp>

namespace qi {
  namespace detail {
    std::string endpointTypeAsString(EndpointType type) {
      if (type == SERVER_ENDPOINT) {
        return "Server";
      }
      if (type == CLIENT_ENDPOINT) {
        return "Client";
      }
      if (type == PUBLISHER_ENDPOINT) {
        return "Publisher";
      }
      if (type == SUBSCRIBER_ENDPOINT) {
        return "Subscriber";
      }
      return "Unknown";
    }

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

