#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_ADDRESS_MANAGER_HPP__
#define   __QI_MESSAGING_DETAIL_ADDRESS_MANAGER_HPP__

#include <string>

namespace qi {
  namespace detail {
    class AddressManager {
    public:
      AddressManager();
      virtual ~AddressManager();

      int getMasterPort() const;
      int getNewPort();

    protected:
      int _nextFreePort;
    };
  }
}
#endif // __QI_MESSAGING_DETAIL_ADDRESS_MANAGER_HPP__

