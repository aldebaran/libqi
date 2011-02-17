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
#include <qi/serialization/serialize.hpp>

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
    };
  }
}
#include <qi/serialization/serializer.hxx>

#endif  // _QI_SERIALIZATION_SERIALIZER_HPP_
