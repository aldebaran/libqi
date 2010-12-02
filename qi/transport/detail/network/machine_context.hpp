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

namespace qi {
  namespace detail {
    struct MachineContext {
      std::string hostName;
      std::string machineID;
      std::string publicIP;
      int platformID;

      MachineContext();

      MachineContext(const std::string& hostName,
        const std::string& machineID,
        const std::string& publicIPAddress,
        const int&         platformID);
    };
  }
}
#endif  // _QI_TRANSPORT_DETAIL_NETWORK_MACHINE_CONTEXT_HPP_

