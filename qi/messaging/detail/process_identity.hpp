#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_PROCESS_IDENTITY_HPP__
#define   __QI_MESSAGING_DETAIL_PROCESS_IDENTITY_HPP__

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
#endif // __QI_MESSAGING_DETAIL_PROCESS_IDENTITY_HPP__

