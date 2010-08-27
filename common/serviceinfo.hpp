#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_SERVICEINFO_HPP_
#define COMMON_SERVICEINFO_HPP_

#include <string>

namespace AL {
  namespace Common {
    struct ServiceInfo {

      std::string nodeName;  // or ID?
      std::string moduleName;
      std::string methodName;
      // TODO functor / args / ret / hash

      ServiceInfo() {}

      ServiceInfo(
        const std::string& nodeName,
        const std::string& module,
        const std::string& method) :
          nodeName(nodeName),
            moduleName(module),
            methodName(method) {}
    };
  }
}

#endif  // COMMON_SERVICEINFO_HPP_

