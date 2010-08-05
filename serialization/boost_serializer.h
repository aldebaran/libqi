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

// xml archive
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

// text archive
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

// text archive
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

// special types
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

// argument types
#include <boost/shared_ptr.hpp>
#include <string>

// enum of serialization types
#include "serialization_types.h"

namespace AL {
  namespace Serialization {

    class BoostSerializer {
    private:
      static const char boost_serialization_flags = boost::archive::no_header;

    public:
      /// <summary>
      /// Serializes from a Shared Pointer to a string containing
      /// a binary, text or xml representation.
      /// Any boost serializable types are accepted.
      /// </summary>
      /// <param name="item"> The Item you wish to serialize </param>
      /// <param name="type">
      /// OPTIONAL: The type of serialization. Default BINARY</param>
      /// <param name="hint">
      /// OPTIONAL: The hint used when serializing to XML
      /// </param>
      /// <returns>
      /// The object in the shared ptr, serialized as a string
      /// </returns>
      template<class T>
      static std::string serialize(
        boost::shared_ptr<T> item,
        SERIALIZATION_TYPE type = BOOST_BINARY,
        char* xml_serialization_hint = "type")
      {
        return serialize(*item, type, xml_serialization_hint);
      }

      /// <summary>
      /// Serializes a type to a string containing
      /// a binary, text or xml representation.
      /// Any boost serializable types are accepted.
      /// </summary>
      /// <param name="item"> The Item you wish to serialize </param>
      /// <param name="type">
      /// OPTIONAL: The type of serialization. Default BINARY
      /// </param>
      /// <param name="hint">
      /// OPTIONAL: The hint used when serializing to XML
      /// </param>
      /// <returns> The object, serialized as a string </returns>
      template<class T>
      static std::string serialize(
        const T& item,
        SERIALIZATION_TYPE type = BOOST_BINARY,
        char* xml_serialization_hint = "type")
      {
        std::stringstream stream;
        if (type == BOOST_BINARY) {
          boost::archive::binary_oarchive oa(stream, boost_serialization_flags);
          oa << item;
        } else if (type == BOOST_XML) {
          boost::archive::xml_oarchive oa(stream, boost_serialization_flags);
          oa << boost::serialization::make_nvp(xml_serialization_hint, item);
        } else {
          boost::archive::text_oarchive oa(stream, boost_serialization_flags);
          oa << item;
        }
        return stream.str();
      }

      template<class T>
      static T deserialize(
        const std::string & buffer,
        SERIALIZATION_TYPE type = BOOST_BINARY,
        char* xml_serialization_hint = "type")
      {
        return deserialize<T>(
          (char*)buffer.c_str(),
          buffer.size(),
          type,
          xml_serialization_hint);
      }

      template<class T>
      static T deserialize(
        char* chars,
        const int size,
        SERIALIZATION_TYPE type = BOOST_BINARY,
        char* xml_serialization_hint = "type")
      {
        boost::interprocess::bufferstream buff(chars, size);
        T ret;

        if (type == BOOST_BINARY) {
          boost::archive::binary_iarchive ia(buff, boost_serialization_flags);
          ia >> ret;
        } else if (type == BOOST_XML) {
          boost::archive::xml_iarchive ia(buff, boost_serialization_flags);
          ia >> boost::serialization::make_nvp(xml_serialization_hint, ret);
        } else {
          boost::archive::text_iarchive ia(buff, boost_serialization_flags);
          ia >> ret;
        }
        return ret;
      }


      template<class T>
      static boost::shared_ptr<T> deserializeToPtr(
        const std::string & buffer,
        SERIALIZATION_TYPE type = BOOST_BINARY,
        char* xml_serialization_hint = "type")
      {
        return deserializeToPtr<T>( (char*)buffer.c_str(),
          buffer.size(),
          type,
          xml_serialization_hint);
      }

      template<class T>
      static boost::shared_ptr<T> deserializeToPtr(
        char* chars,
        const int size,
        SERIALIZATION_TYPE type = BOOST_BINARY,
        char* xml_serialization_hint = "type")
      {
        boost::interprocess::bufferstream buff(chars, size);
        boost::shared_ptr<T> ret = boost::shared_ptr<T> (new T());

        if (type == BOOST_BINARY) {
          boost::archive::binary_iarchive ia(buff, boost_serialization_flags);
          ia >> *ret;
        } else if (type == BOOST_XML) {
          boost::archive::xml_iarchive ia(buff, boost_serialization_flags);
          ia >> boost::serialization::make_nvp(xml_serialization_hint, *ret);
        } else {
          boost::archive::text_iarchive ia(buff, boost_serialization_flags);
          ia >> *ret;
        }
        return ret;
      }
    };
  }
}

#endif  // SERIALIZATION_BOOST_SERIALIZATION_H_
