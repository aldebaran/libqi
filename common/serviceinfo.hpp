#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_SERVICEINFO_H_
#define COMMON_SERVICEINFO_H_

#include <string>

namespace AL {
  namespace Common {
    struct ServiceInfo {
      std::string nodeName;  // or ID?
      std::string moduleName;
      std::string methodName;
      // TODO functor / args / ret

      ServiceInfo() {}

      ServiceInfo(const std::string& name,
        const std::string& module,
        const std::string& method) :
          nodeName(name), moduleName(module), methodName(method) {}
    };
  }  // namespace Common
}  // namespace AL

#endif  // COMMON_NODE_H_

