#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef QI_NODE_DETAIL_ADDRESS_MANAGER_HPP
#define QI_NODE_DETAIL_ADDRESS_MANAGER_HPP

#include <string>
#include <vector>

namespace qi {
  namespace detail {
    class AddressManager {
    public:
      AddressManager();
      virtual ~AddressManager();

      int getMasterPort() const;
      int getNewPort();

      std::vector<std::string> getNewServerBindAddresses(void);
      std::vector<std::string> getServerBindAddresses(int port);

    protected:
      int _nextFreePort;
    };
  }
}
#endif  // QI_NODE_DETAIL_ADDRESS_MANAGER_HPP

