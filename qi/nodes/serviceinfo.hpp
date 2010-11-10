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
#include <qi/functors/functor.hpp>

namespace qi {
  namespace Nodes {
    struct ServiceInfo {
      std::string  methodName;
      qi::Functor     *functor;

      ServiceInfo() {}

      ServiceInfo(const std::string  &module,
                  const std::string  &method,
                  qi::Functor        *functor)
        : methodName(module + std::string(".") + method),
          functor(functor)
      {}

      ServiceInfo(const std::string  &method,
          qi::Functor        *functor)
        : methodName(method),
          functor(functor)
      {}
    };
  }
}

#endif  // COMMON_SERVICEINFO_HPP_

