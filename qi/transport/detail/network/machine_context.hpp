#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_MACHINE_CONTEXT_HPP__
#define   __QI_MESSAGING_DETAIL_MACHINE_CONTEXT_HPP__

#include <string>

namespace qi {
  namespace detail {
    struct MachineContext {
      std::string machineID;
      std::string hostName;
      std::string publicIP;
      int platformID;
      MachineContext();
    };
  }
}
#endif // __QI_MESSAGING_DETAIL_MACHINE_CONTEXT_HPP__

