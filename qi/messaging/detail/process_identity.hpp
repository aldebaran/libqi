#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef QI_NODE_DETAIL_PROCESS_IDENTITY_HPP
#define QI_NODE_DETAIL_PROCESS_IDENTITY_HPP

#include <string>
namespace qi {
  namespace detail {

    struct ProcessIdentity {
      std::string hostName;
      std::string macAddress;
      int         processID;
      ProcessIdentity();
    };

    int getProcessID();
    std::string getHostName();

    // disambiguates hosts
    std::string getFirstMacAddress();
  }
}
#endif  // QI_NODE_DETAIL_PROCESS_IDENTITY_HPP

