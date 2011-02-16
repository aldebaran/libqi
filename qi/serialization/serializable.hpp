#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_SERIALIZATION_SERIALIZABLE_HPP_
#define _QI_SERIALIZATION_SERIALIZABLE_HPP_

namespace qi {
  namespace serialization {
    class Serializer;

    class Serializable {
    public:
      virtual void accept(Serializer& v) = 0;
    };

  }
}

#endif  // _QI_SERIALIZATION_SERIALIZABLE_HPP_
