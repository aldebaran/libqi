#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef SERIALIZATION_BOOST_SERIALIZATION_H_
#define SERIALIZATION_BOOST_SERIALIZATION_H_

// internal type (fixed size buffer)
#include <boost/interprocess/streams/bufferstream.hpp>
#include <sstream>

// special types
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

// text archive
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

// binary archive
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

// argument types
#include <string>

namespace AL {
  namespace Serialization {

    template<
      typename INPUT_ARCHIVE = boost::archive::binary_iarchive,
      typename OUTPUT_ARCHIVE = boost::archive::binary_oarchive
    >
    class BoostGenericSerializer {
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


    private:
      static const char boost_serialization_flags = boost::archive::no_header;
  };

    // TODO move to another file
    // =============  implementation ============= 
    template<class T, typename INPUT_ARCHIVE, typename OUTPUT_ARCHIVE>
    static std::string BoostGenericSerializer<INPUT_ARCHIVE, OUTPUT_ARCHIVE>::serialize(const T& item)
    {
      std::stringstream stream;
      OUTPUT_ARCHIVE oa(stream, boost_serialization_flags);
      oa << item;
      return stream.str();
    }

    template<class T, typename INPUT_ARCHIVE, typename OUTPUT_ARCHIVE>
    static T BoostGenericSerializer<INPUT_ARCHIVE, OUTPUT_ARCHIVE>::deserialize(const std::string & buffer)
    {
      T ret;
      deserialize((char*)buffer.c_str(), buffer.size(), T& ret);
      return ret;
    }

    template<class T, typename INPUT_ARCHIVE, typename OUTPUT_ARCHIVE>
    static T BoostGenericSerializer<INPUT_ARCHIVE, OUTPUT_ARCHIVE>::deserialize(char* chars, const int size)
    {
      T ret;
      deserialize(char* chars, const int size, T& ret)
      return ret;
    }

    template<class T, typename INPUT_ARCHIVE, typename OUTPUT_ARCHIVE>
    static void BoostGenericSerializer<INPUT_ARCHIVE, OUTPUT_ARCHIVE>::deserialize(const std::string& buffer, T& ret)
    {
      deserialize((char*)buffer.c_str(), buffer.size(), ret);
    }

    template<class T, typename INPUT_ARCHIVE, typename OUTPUT_ARCHIVE>
    static void BoostGenericSerializer<INPUT_ARCHIVE, OUTPUT_ARCHIVE>::deserialize(char* chars, const int size, T& ret)
    {
      boost::interprocess::bufferstream buff(chars, size);
      INPUT_ARCHIVE ia(buff, boost_serialization_flags);
      ia >> ret;
    }
    // =========================================
  }
}

#endif  // SERIALIZATION_BOOST_SERIALIZATION_H_
