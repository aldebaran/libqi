#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SERIALIZATION_SERIALIZER_HPP_
#define _QI_SERIALIZATION_SERIALIZER_HPP_

#include <qi/serialization/message.hpp>

namespace qi {
  namespace serialization {

    class Serializable;

    enum SerializeAction {
      ACTION_SERIALIZE,
      ACTION_DESERIALIZE
    };

    /// <summary>
    /// Serializer class. Provides methods that can visit fields of a struct of class
    /// and either serialize or deserialize into the reference of a message provided in
    /// the constructor. Supports recursive POD types, vectors, maps and classes derived
    /// from Serializable.
    /// </summary>
    class Serializer {
    private:
      qi::serialization::SerializeAction _action;
      qi::serialization::Message& _message;
    public:

      /// <summary> Constructs a Serializer object </summary>
      /// <param name="action"> The SerializeAction, either ACTION_SERIALIZE or ACTION_DESERIALIZE. </param>
      /// <param name="message"> [in,out] A reference to the message to serialize to or from </param>
      Serializer(SerializeAction action, qi::serialization::Message& message);

      /// <summary> Visits a bool </summary>
      /// <param name="b"> [in,out] The bool </param>
      void visit(bool& b);

      /// <summary> Visits a char </summary>
      /// <param name="c"> [in,out] The char </param>
      void visit(char& c);

      /// <summary> Visits an int </summary>
      /// <param name="i"> [in,out] The int </param>
      void visit(int& i);

      /// <summary> Visits a float </summary>
      /// <param name="f"> [in,out] The float </param>
      void visit(float& f);

      /// <summary> Visits a std::string </summary>
      /// <param name="s"> [in,out] The std::string </param>
      void visit(std::string& s);

      /// <summary> Visits a Serializable class </summary>
      /// <param name="v"> [in,out] The Serializable class. </param>
      void visit(Serializable& v);

      /// <summary> Visits a std::vector </summary>
      /// <param name="v"> [in,out] The std::vector. </param>
      template <typename T>
      void visit(std::vector<T>& v);

      /// <summary> Visits a std::map </summary>
      /// <param name="m"> [in,out] The std::map </param>
      template <typename K, typename V>
      void visit(std::map<K, V>& m);

      /// <summary> Helper method: Visits two parameters in one call </summary>
      template <typename P0, typename P1>
      void visit(P0& p0, P1& p1);

      /// <summary> Helper method: Visits three parameters in one call </summary>
      template <typename P0, typename P1, typename P2>
      void visit(P0& p0, P1& p1, P2& p2);

      /// <summary> Helper method: Visits four parameters in one call </summary>
      template <typename P0, typename P1, typename P2, typename P3>
      void visit(P0& p0, P1& p1, P2& p2, P3& p3);

      /// <summary> Helper method: Visits five parameters in one call </summary>
      template <typename P0, typename P1, typename P2, typename P3, typename P4>
      void visit(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4);

      /// <summary> Helper method: Visits six parameters in one call </summary>
      template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
      void visit(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5);

      /// <summary> Helper method: Visits seven parameters in one call </summary>
      template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
      void visit(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6);

      /// <summary> Helper method: Visits eight parameters in one call </summary>
      template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
      void visit(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7);

      /// <summary> Helper method: Visits nine parameters in one call </summary>
      template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
      void visit(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7, P8& p8);
    };
  }
}
#include <qi/serialization/serialize.hpp>

#include <qi/serialization/serializer.hxx>

#endif  // _QI_SERIALIZATION_SERIALIZER_HPP_
