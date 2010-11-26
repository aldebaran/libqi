#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_TRANSPORT_DETAIL_NETWORK_IP_ADDRESS_HPP__
#define   __QI_TRANSPORT_DETAIL_NETWORK_IP_ADDRESS_HPP__

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
#endif // __QI_TRANSPORT_DETAIL_NETWORK_IP_ADDRESS_HPP__

