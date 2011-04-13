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

    template <typename P0, typename P1>
    void Serializer::visit(P0& p0, P1& p1) {
      visit(p0);
      visit(p1);
    }

    template <typename P0, typename P1, typename P2>
    void Serializer::visit(P0& p0, P1& p1, P2& p2) {
      visit(p0);
      visit(p1);
      visit(p2);
    }

    template <typename P0, typename P1, typename P2, typename P3>
    void Serializer::visit(P0& p0, P1& p1, P2& p2, P3& p3) {
      visit(p0);
      visit(p1);
      visit(p2);
      visit(p3);
    }

    template <typename P0, typename P1, typename P2, typename P3, typename P4>
    void Serializer::visit(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4) {
      visit(p0);
      visit(p1);
      visit(p2);
      visit(p3);
      visit(p4);
    }

    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
    void Serializer::visit(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5) {
      visit(p0);
      visit(p1);
      visit(p2);
      visit(p3);
      visit(p4);
      visit(p5);
    }

    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    void Serializer::visit(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6) {
      visit(p0);
      visit(p1);
      visit(p2);
      visit(p3);
      visit(p4);
      visit(p5);
      visit(p6);
    }

    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    void Serializer::visit(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7) {
      visit(p0);
      visit(p1);
      visit(p2);
      visit(p3);
      visit(p4);
      visit(p5);
      visit(p6);
      visit(p7);
    }


    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    void Serializer::visit(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7, P8& p8) {
      visit(p0);
      visit(p1);
      visit(p2);
      visit(p3);
      visit(p4);
      visit(p5);
      visit(p6);
      visit(p7);
      visit(p8);
    }

  }
}

#endif  // _QI_SERIALIZATION_SERIALIZER_HXX_
