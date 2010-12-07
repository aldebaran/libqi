#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_NETWORK_MAC_ADDRESS_HPP_
#define _QI_TRANSPORT_DETAIL_NETWORK_MAC_ADDRESS_HPP_

#include <string>

namespace qi {
  namespace detail {
    // to disambiguates hosts with ambiguous hostnames
    std::string getFirstMacAddress();

#ifndef _WIN32
    // linux and mac support sane interface names
    std::string getMacAddress(std::string pInterfaceName);
#endif
  }
}
#endif  // _QI_TRANSPORT_DETAIL_NETWORK_MAC_ADDRESS_HPP_

