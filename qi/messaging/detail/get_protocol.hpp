#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef QI_NODES_DETAIL_GETPROTOCOL_HPP_
#define QI_NODES_DETAIL_GETPROTOCOL_HPP_

#include <string>

namespace qi {
  namespace detail {
    inline const std::string getProtocol(const std::string& myAddress, const std::string& otherAddress) {
      // if thisIP = remoteIP && UNIX/MAX? return "ipc://"
      return "tcp://";
    }
  }
}

#endif  // QI_NODES_DETAIL_GETPROTOCOL_HPP_

