#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_SERIALIZATION_SERIALIZE_POD_HXX_
#define _QI_SERIALIZATION_SERIALIZE_POD_HXX_

namespace qi {
  namespace serialization {

    QI_SIMPLE_SERIALIZER(Bool, bool);
    QI_SIMPLE_SERIALIZER(Char, char);
    QI_SIMPLE_SERIALIZER(Int, int);
    QI_SIMPLE_SERIALIZER(Float, float);

    template <typename T>
    struct serialize<T&> {
      static inline void write(Message &sd, const T &val) {
        __QI_DEBUG_SERIALIZATION_W(T, "&");
        serialize<T>::write(sd, val);
      }
      static inline void read(Message &sd, T &val) {
        __QI_DEBUG_SERIALIZATION_R(T, "&");
        serialize<T>::read(sd, val);
      }
    };

    template <typename T>
    struct serialize<const T> {
      static inline void write(Message &sd, const T &val) {
        __QI_DEBUG_SERIALIZATION_W(T, "Const");
        serialize<T>::write(sd, val);
      }
      static inline void read(Message &sd, T &val) {
        __QI_DEBUG_SERIALIZATION_R(T, "Const");
        serialize<T>::read(sd, val);
      }
    };

  }
}

#endif  // _QI_SERIALIZATION_SERIALIZE_HXX_
