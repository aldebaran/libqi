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
    struct EndpointContext {
      std::string endpointID;
      std::string machineID;
      int processID;
      // not discoverable at construction
      std::string name;
      std::string contextID;
      int port;
      EndpointContext();
    };
  }
}
#endif // __QI_MESSAGING_DETAIL_ENDPOINT_CONTEXT_HPP__

