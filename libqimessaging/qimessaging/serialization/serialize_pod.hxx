#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_SERIALIZATION_SERIALIZE_POD_HXX_
#define _QIMESSAGING_SERIALIZATION_SERIALIZE_POD_HXX_

#include <qimessaging/serialization/datastream.hpp>

namespace qi {
  namespace serialization {

    QI_SIMPLE_SERIALIZER(bool);
    QI_SIMPLE_SERIALIZER(char);
    QI_SIMPLE_SERIALIZER(int);
    QI_SIMPLE_SERIALIZER(float);

    template <typename T>
    struct serialize<T&> {
      static inline void write(qi::DataStream &sd, const T &val) {
        __QI_DEBUG_SERIALIZATION_W(T, "&");
        serialize<T>::write(sd, val);
      }
      static inline void read(qi::DataStream &sd, T &val) {
        __QI_DEBUG_SERIALIZATION_R(T, "&");
        serialize<T>::read(sd, val);
      }
    };

    template <typename T>
    struct serialize<const T> {
      static inline void write(qi::DataStream &sd, const T &val) {
        __QI_DEBUG_SERIALIZATION_W(T, "Const");
        serialize<T>::write(sd, val);
      }
      static inline void read(qi::DataStream &sd, T &val) {
        __QI_DEBUG_SERIALIZATION_R(T, "Const");
        serialize<T>::read(sd, val);
      }
    };

  }
}

#endif  // _QIMESSAGING_SERIALIZATION_SERIALIZE_POD_HXX_
