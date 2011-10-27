/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <sstream>
#include "src/messaging/network/master_endpoint.hpp"
#include "src/messaging/network/constants.hpp"

/* XXX: DEBUG */
#include <iostream>

namespace qi {
namespace detail {

bool validateMasterEndpoint(
    const std::string& hintAddress,
    std::pair<std::string, int>& outEndpointAndPort)
{
  qi::detail::Address outAddress;

  // check if it looks like it will be valid
  if (!isValidAddress(hintAddress, outAddress))
    return false;

  // implies that no port was given
  if (outAddress.port == 0)
    outAddress.port = kDefaultMasterPort;

  // convert the numeric port to char
  std::stringstream port;
  port << outAddress.port;

  // create fully qualified address;
  outEndpointAndPort.first =
      outAddress.transport + "://" +
      outAddress.address + ":" + port.str();
  outEndpointAndPort.second = outAddress.port;

  /* XXX: DEBUG */
  std::clog << outEndpointAndPort.first << std::endl;

  return true;
}

} // namespace detail
} // namespace qi
