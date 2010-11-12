#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef SERIALIZATION_BOOST_BINARY_SERIALIZER_HPP_
#define SERIALIZATION_BOOST_BINARY_SERIALIZER_HPP_

#include <string>

namespace qi {
  namespace serialization {
    class BoostBinarySerializer {
    public:
      /// <summary>
      /// Serializes a type to a string containing
      /// a serialization of the item
      /// Any boost serializable types are accepted.
      /// </summary>
      /// <param name="item">The Item you wish to serialize</param>
      /// <returns>The object, serialized as a string</returns>
      template<class T>
      static std::string serialize(const T& item);

      /// <summary>
      /// DeSerializes a type from a string buffer
      /// Any boost serializable types are accepted.
      /// </summary>
      /// <param name="buffer">The text buffer containing the serialized object</param>
      /// <returns>The object of type T</returns>
      template<class T>
      static T deserialize(const std::string & buffer);

      /// <summary>
      /// DeSerializes a type from a string buffer
      /// Any boost serializable types are accepted.
      /// </summary>
      /// <param name="chars">A pointer the start of the text buffer containing the serialized object</param>
      /// <param name="size">The size of the buffer</param>
      /// <returns>The object of type T</returns>
      template<class T>
      static T deserialize(char* chars, const int size);

      /// <summary>
      /// DeSerializes a type from a string buffer
      /// Any boost serializable types are accepted.
      /// </summary>
      /// <param name="chars">A pointer the start of the text buffer containing the serialized object</param>
      /// <param name="buffer">The text buffer containing the serialized object</param>
      /// <returns>The object of type T</returns>
      template<class T>
      static void deserialize(const std::string& buffer, T& ret);

      /// <summary>
      /// DeSerializes a type from a string buffer
      /// Any boost serializable types are accepted.
      /// </summary>
      /// <param name="chars">A pointer the start of the text buffer containing the serialized object</param>
      /// <param name="size">The size of the buffer</param>
      /// <param name="ret">A reference to the output object of type T</returns>
      template<class T>
      static void deserialize(char* chars, const int size, T& ret);
    };
  }
}

// implementation
#include <qi/serialization/boost/boost_binary_serializer.hxx>

#endif  // SERIALIZATION_BOOST_BINARY_SERIALIZER_HPP_
