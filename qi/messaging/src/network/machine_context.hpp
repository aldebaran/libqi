#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_NETWORK_MACHINE_CONTEXT_HPP_
#define _QI_TRANSPORT_DETAIL_NETWORK_MACHINE_CONTEXT_HPP_

#include <string>
#include <qi/messaging/src/network/platform.hpp>

namespace qi {
  namespace detail {
    struct MachineContext {
      std::string hostName;
      std::string machineID;
      std::string publicIP;
      Platform platformID;

      MachineContext();

      MachineContext(const std::string& hostName,
        const std::string& machineID,
        const std::string& publicIPAddress,
        const Platform&    platformID);
    };
  }
}
#endif  // _QI_TRANSPORT_DETAIL_NETWORK_MACHINE_CONTEXT_HPP_

