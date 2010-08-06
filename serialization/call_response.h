#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef SERIALIZATION_CALL_RESPONSE_H_
#define SERIALIZATION_CALL_RESPONSE_H_

namespace AL {
  namespace Messaging {
    template<typename T>
    struct CallResponse {
      T response;
    };
  }
}

#endif  // SERIALIZATION_CALL_RESPONSE_H_
