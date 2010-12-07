#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_NETWORK_ENDPOINT_CONTEXT_HPP_
#define _QI_TRANSPORT_DETAIL_NETWORK_ENDPOINT_CONTEXT_HPP_

#include <string>

namespace qi {
  namespace detail {

    enum EndpointType {
      SERVER_ENDPOINT     = 0,
      CLIENT_ENDPOINT     = 1,
      PUBLISHER_ENDPOINT  = 2,
      SUBSCRIBER_ENDPOINT = 3,
      UNDEFINED_ENDPOINT  = 99,
    };

    std::string endpointTypeAsString(EndpointType type);

    struct EndpointContext {
      EndpointType type;
      std::string  name;
      std::string  endpointID;
      std::string  contextID;
      std::string  machineID;
      int          processID;
      int          port;

      EndpointContext();

      EndpointContext(
        const EndpointType& type,
        const std::string& name,
        const std::string& endpointID,
        const std::string& contextID,
        const std::string& machineID,
        const int& processID,
        const int& port);
    };
  }
}
#endif  // _QI_TRANSPORT_DETAIL_NETWORK_ENDPOINT_CONTEXT_HPP_

