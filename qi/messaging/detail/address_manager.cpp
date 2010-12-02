/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/address_manager.hpp>
#include <qi/transport/detail/network/constants.hpp>
#include <cstdio>

namespace qi {
  namespace detail {
    AddressManager::AddressManager() :
      _masterPort(kDefaultMasterPort),
      _nextFreePort(kDefaultMasterPort)
    {}

    AddressManager::~AddressManager() {}

    int AddressManager::getMasterPort() const {
      return _masterPort;
    }

    void AddressManager::setMasterPort(int masterPort) {
      // FIXME ehem, hope no other ports have been given out.
      _masterPort = masterPort;
      _nextFreePort = masterPort;
    }

    int AddressManager::getNewPort(const std::string& machineID) {
      // FIXME: machineID currently ignored
      // should really have per machine lists
      return ++_nextFreePort;
    }
  }
}


