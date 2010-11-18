#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_GET_PROTOCOL_HPP__
#define   __QI_MESSAGING_DETAIL_GET_PROTOCOL_HPP__

#include <string>

namespace qi {
  namespace detail {
    inline const std::string getProtocol(const std::string& myAddress, const std::string& otherAddress) {
      // if thisIP = remoteIP && UNIX/MAX? return "ipc://"
      return "tcp://";
    }
  }
}

#endif // __QI_MESSAGING_DETAIL_GET_PROTOCOL_HPP__

