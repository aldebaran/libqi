#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_NETWORK_IP_ADDRESS_HPP_
#define _QI_TRANSPORT_DETAIL_NETWORK_IP_ADDRESS_HPP_

#include <string>
#include <vector>

namespace qi {
  namespace detail {

    std::string getPrimaryPublicIPAddress();

    std::vector<std::string> getIPAddresses();

    bool isValidAddress(const std::string& userHostString, std::pair<std::string, int>& hostAndPort);

    bool isValidHostAndPort(const std::string& literalHostName, std::string& numericHostName);
  }
}
#endif  // _QI_TRANSPORT_DETAIL_NETWORK_IP_ADDRESS_HPP_

