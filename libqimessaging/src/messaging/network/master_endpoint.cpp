/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <stdio.h>
#include "src/messaging/network/master_endpoint.hpp"
#include "src/messaging/network/ip_address.hpp"
#include "src/messaging/network/constants.hpp"

namespace qi {
  namespace detail {

    bool validateMasterEndpoint(
      const std::string& hintAddress,
      std::pair<std::string, int>& outEndpointAndPort) {

        // check if it looks like it will be valid
        if (!isValidAddress(hintAddress, outEndpointAndPort)) {
          return false;
        }

        // implies that no port was given
        if (outEndpointAndPort.second == 0) {
          outEndpointAndPort.second = kDefaultMasterPort;
        }

        // convert the numeric port to char
        char szPort[256];
        sprintf(szPort, "%d", outEndpointAndPort.second);

        // create fully qualified address;
        outEndpointAndPort.first =
          std::string("tcp://") +
          outEndpointAndPort.first +
          std::string(":") + szPort;

        return true;
    }
  }
}


