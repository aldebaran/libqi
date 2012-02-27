/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#pragma once
#ifndef _SRC_MESSAGING_NETWORK_IP_ADDRESS_HPP_
#define _SRC_MESSAGING_NETWORK_IP_ADDRESS_HPP_

#include <string>
#include <vector>

namespace qi {
  namespace detail {

    struct Address
    {
      std::string transport;
      std::string address;
      int port;
    };

    std::string getPrimaryPublicIPAddress();

    std::vector<std::string> getIPAddresses();

    bool splitAddress(const std::string& userHostString, Address& hostAndPort,
        std::vector<std::string>& parts);

    bool isValidAddress(const std::string& userHostString,
        Address& hostAndPort);

    bool isValidHostAndPort(const std::string& literalHostName,
        std::string& numericHostName);
  }
}
#endif  // _SRC_MESSAGING_NETWORK_IP_ADDRESS_HPP_

