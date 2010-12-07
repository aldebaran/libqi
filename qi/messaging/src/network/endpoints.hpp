#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_NETWORK_ENDPOINTS_HPP_
#define _QI_TRANSPORT_DETAIL_NETWORK_ENDPOINTS_HPP_

#include <vector>
#include <qi/messaging/src/network/endpoint_context.hpp>
#include <qi/messaging/src/network/machine_context.hpp>

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
#endif  // _QI_TRANSPORT_DETAIL_NETWORK_ENDPOINTS_HPP_

