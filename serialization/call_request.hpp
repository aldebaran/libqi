#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef SERIALIZATION_CALL_REQUEST_H_
#define SERIALIZATION_CALL_REQUEST_H_

#include <string>

namespace AL {
  namespace Messaging {
    template<typename T>
    struct CallRequest {
      std::string module;
      std::string method;
      T args;
    };
  }
}
#endif  // SERIALIZATION_CALL_REQUEST_H_
