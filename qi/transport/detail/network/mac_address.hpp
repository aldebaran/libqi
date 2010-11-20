#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_TRANSPORT_DETAIL_NETWORK_MAC_ADDRESS_HPP__
#define   __QI_TRANSPORT_DETAIL_NETWORK_MAC_ADDRESS_HPP__

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
#endif // __QI_TRANSPORT_DETAIL_NETWORK_MAC_ADDRESS_HPP__

