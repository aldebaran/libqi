#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SERIALIZATION_MESSAGE_HPP_
#define _QI_SERIALIZATION_MESSAGE_HPP_

#include <iostream>
#include <qi/signature.hpp>

namespace qi {

  namespace serialization {


    /// serialize a c++ type to a message
    /// \ingroup Serialization
    //Enable is need for protobuf (for conditional template specialization)
    template <typename T, class Enable = void>
    struct serialize {
      static void read(Message &sd, T &t){
        std::cout << "ERROR: this type is not serializable" << std::endl;
        //#error "This type is not serializable"
      }

      static void write(Message &sd, const T &t) {
        std::cout << "ERROR: this type is not serializable" << std::endl;
        //#error "This type is not serializable"
      }
    };

  }
}

#include <qi/serialization/serialize.hxx>

#endif  // _QI_SERIALIZATION_MESSAGE_HPP_
