#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SERIALIZATION_SERIALIZER_HXX_
#define _QI_SERIALIZATION_SERIALIZER_HXX_

namespace qi {
  namespace serialization {

    template <typename T>
    void Serializer::visit(std::vector<T>& v) {
      if (_action == ACTION_SERIALIZE) {
        serialize<std::vector<T> >::write(_message, v);
      } else {
        serialize<std::vector<T> >::read(_message, v);
      }
    }

    template <typename K, typename V>
    void Serializer::visit(std::map<K, V>& m) {
      if (_action == ACTION_SERIALIZE) {
        serialize<std::map<K, V> >::write(_message, m);
      } else {
        serialize<std::map<K, V> >::read(_message, m);
      }
    }

  }
}

#endif  // _QI_SERIALIZATION_SERIALIZER_HXX_
