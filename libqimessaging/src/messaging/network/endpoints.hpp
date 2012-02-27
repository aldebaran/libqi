/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#pragma once
#ifndef _SRC_MESSAGING_NETWORK_ENDPOINTS_HPP_
#define _SRC_MESSAGING_NETWORK_ENDPOINTS_HPP_

#include <vector>
#include "src/messaging/network/endpoint_context.hpp"
#include "src/messaging/network/machine_context.hpp"

namespace qi {
  namespace detail {

    /// <summary> Returns a fully qualified endpoint for connecting a client and a server </summary>
    std::string negotiateEndpoint(const EndpointContext& clientContext,
                                  const EndpointContext& serverContext,
                                  const MachineContext&  serverMachineContext);

    /// <summary>
    /// Gets the list of endpoints that can be bound to.
    /// This result is only valid if made on the platform of the server
    /// </summary>
    std::vector<std::string> getEndpoints(const EndpointContext serverContext,
                                          const MachineContext& serverMachineContext);
  }
}
#endif  // _SRC_MESSAGING_NETWORK_ENDPOINTS_HPP_

