/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#include <qi/nodes/detail/address_manager.hpp>
#include <qi/nodes/detail/constants.hpp>
#include <cstdio>

namespace qi {
  namespace detail {
    AddressManager::AddressManager() {
      _nextFreePort = kDefaultMasterPort+1;
    }

    AddressManager::~AddressManager() {}

    int AddressManager::getMasterPort() const {
      return kDefaultMasterPort;
    }

    int AddressManager::getNewPort() {
      return _nextFreePort++;
    }

    std::vector<std::string> AddressManager::getNewServerBindAddresses(void) {
      return getServerBindAddresses(getNewPort());
    }

    std::vector<std::string> AddressManager::getServerBindAddresses(int port) {
      std::vector<std::string> addresses;
      char buf[30];

      // TODO get public IP addresses

      // FIXME buffer overflow for very big ports
      sprintf(buf, "tcp://127.0.0.1:%d", port);
      addresses.push_back(buf);
      sprintf(buf, "inproc://127.0.0.1:%d", port);
      addresses.push_back(buf);
#ifndef _WIN32
      // windows does not support ipc://
      sprintf(buf, "icp://127.0.0.1:%d", port);
      addresses.push_back(buf);
#endif
      return addresses;
    }
  }
}


