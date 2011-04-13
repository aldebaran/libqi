#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_NETWORK_MASTER_ENDPOINT_HPP_
#define _QI_MESSAGING_SRC_NETWORK_MASTER_ENDPOINT_HPP_

#include <string>

namespace qi {
  namespace detail {

    /// <summary> Validates a master endpoint </summary>
    /// <param name="hintAddress">
    /// A partial endpoint. Missing protocol and probably port
    /// </param>
    /// <param name="outEndpointAndPort">
    /// Output a pair containing a fully qualified master endpoint and it's port
    /// </param>
    /// <returns>A bool indicating success</returns>
    bool validateMasterEndpoint(
      const std::string& hintAddress,
      std::pair<std::string, int>& outEndpointAndPort);
  }
}
#endif  // _QI_MESSAGING_SRC_NETWORK_MASTER_ENDPOINT_HPP_

