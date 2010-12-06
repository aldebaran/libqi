#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SERIALIZATION_BOOST_BOOST_TEXT_SERIALIZER_HXX_
#define _QI_SERIALIZATION_BOOST_BOOST_TEXT_SERIALIZER_HXX_

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

// argument types
#include <string>

namespace qi {
  namespace serialization {

    template<class T>
    std::string BoostTextSerializer::serialize(const T& item)
    {
      std::stringstream stream;
      boost::archive::text_oarchive oa(stream, boost::archive::no_header);
      oa << item;
      return stream.str();
    }

    template<class T>
    T BoostTextSerializer::deserialize(const std::string & buffer)
    {
      T ret;
      deserialize((char*)buffer.c_str(), buffer.size(), ret);
      return ret;
    }

    template<class T>
    T BoostTextSerializer::deserialize(char* chars, const int size)
    {
      T ret;
      deserialize(chars, size, ret);
      return ret;
    }

    template<class T>
    void BoostTextSerializer::deserialize(const std::string& buffer, T& ret)
    {
      deserialize((char*)buffer.c_str(), buffer.size(), ret);
    }

    template<class T>
    void BoostTextSerializer::deserialize(char* chars, const int size, T& ret)
    {
      boost::interprocess::bufferstream buff(chars, size);
      boost::archive::text_iarchive ia(buff, boost::archive::no_header);
      ia >> ret;
    }
  }
}

#endif  // _QI_SERIALIZATION_BOOST_BOOST_TEXT_SERIALIZER_HXX_
