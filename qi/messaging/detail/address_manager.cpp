/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#include <qi/messaging/detail/address_manager.hpp>
#include <qi/messaging/detail/constants.hpp>
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
  }
}


