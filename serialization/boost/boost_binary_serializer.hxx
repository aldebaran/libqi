#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef SERIALIZATION_BOOST_BINARY_SERIALIZATION_HXX_
#define SERIALIZATION_BOOST_BINARY_SERIALIZATION_HXX_

// internal type (fixed size buffer)
#include <boost/interprocess/streams/bufferstream.hpp>
#include <sstream>

// special types
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

// binary archive
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

// argument types
#include <string>

// temporary
# include <alcommon-ng/serialization/call_definition_serialization.hxx>

namespace AL {
  namespace Serialization {

    template<class T>
    std::string BoostBinarySerializer::serialize(const T& item)
    {
      std::stringstream stream;
      boost::archive::binary_oarchive oa(stream, boost::archive::no_header);
      oa << item;
      return stream.str();
    }

    template<class T>
    T BoostBinarySerializer::deserialize(const std::string & buffer)
    {
      T ret;
      deserialize<T>((char*)buffer.c_str(), buffer.size(), ret);
      return ret;
    }

    template<class T>
    T BoostBinarySerializer::deserialize(char* chars, const int size)
    {
      T ret;
      deserialize<T>(chars, size, ret);
      return ret;
    }

    template<class T>
    void BoostBinarySerializer::deserialize(const std::string& buffer, T& ret)
    {
      deserialize<T>((char*)buffer.c_str(), buffer.size(), ret);
    }

    template<class T>
    void BoostBinarySerializer::deserialize(char* chars, const int size, T& ret)
    {
      boost::interprocess::bufferstream buff(chars, size);
      boost::archive::binary_iarchive ia(buff, boost::archive::no_header);
      ia >> ret;
    }

  }
}

#endif  // SERIALIZATION_BOOST_BINARY_SERIALIZATION_HXX_
