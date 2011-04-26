#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_SERVICEINFO_HPP_
#define _QI_MESSAGING_SRC_SERVICEINFO_HPP_

#include <string>
#include <qimessaging/functors/functor.hpp>

namespace qi {
  namespace detail {
    struct ServiceInfo {
      std::string  methodName;
      qi::Functor     *functor;

      ServiceInfo() {}

      ServiceInfo(const std::string  &method,
        qi::Functor        *functor)
        : methodName(method),
        functor(functor)
      {}
    };
  }
}
#endif  // _QI_MESSAGING_SRC_SERVICEINFO_HPP_

