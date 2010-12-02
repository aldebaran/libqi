#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_MESSAGING_SERVICEINFO_HPP_
#define _QI_MESSAGING_SERVICEINFO_HPP_

#include <string>
#include <qi/functors/functor.hpp>

namespace qi {
  struct ServiceInfo {
    std::string  methodName;
    qi::Functor     *functor;

    ServiceInfo() {}

    //ServiceInfo(const std::string  &module,
    //            const std::string  &method,
    //            qi::Functor        *functor)
    //  : methodName(module + std::string(".") + method),
    //    functor(functor)
    //{}

    ServiceInfo(const std::string  &method,
        qi::Functor        *functor)
      : methodName(method),
        functor(functor)
    {}
  };
}

#endif  // _QI_MESSAGING_SERVICEINFO_HPP_

