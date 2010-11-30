#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_ENDPOINT_CONTEXT_HPP__
#define   __QI_MESSAGING_DETAIL_ENDPOINT_CONTEXT_HPP__

#include <string>

namespace qi {
  namespace detail {

    enum EndpointType {
      SERVER_ENDPOINT     = 0,
      CLIENT_ENDPOINT     = 1,
      PUBLISHER_ENDPOINT  = 2,
      SUBSCRIBER_ENDPOINT = 3
    };

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
#endif // __QI_MESSAGING_DETAIL_ENDPOINT_CONTEXT_HPP__

