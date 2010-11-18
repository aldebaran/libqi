#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_SERVICEINFO_HPP__
#define   __QI_MESSAGING_SERVICEINFO_HPP__

#include <string>
#include <qi/functors/functor.hpp>

namespace qi {
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

#endif // __QI_MESSAGING_SERVICEINFO_HPP__

