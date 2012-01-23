/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/messaging/network/endpoint_context.hpp"
#include "src/messaging/network/network.hpp"
#include <qi/os.hpp>

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
      if (type == FORWARDER_IN_ENDPOINT) {
        return "ForwarderIn";
      }
      if (type == FORWARDER_OUT_ENDPOINT) {
        return "ForwarderOut";
      }
      return "Unknown";
    }

    EndpointContext::EndpointContext():
      type(UNDEFINED_ENDPOINT),
      name(""),
      endpointID(getUUID()),
      contextID(""),
      machineID(getFirstMacAddress()),
      processID(qi::os::getpid()),
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

