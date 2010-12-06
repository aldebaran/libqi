#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SERIALIZATION_BOOST_BOOST_XML_SERIALIZER_HXX_
#define _QI_SERIALIZATION_BOOST_BOOST_XML_SERIALIZER_HXX_

// internal type (fixed size buffer)
#include <boost/interprocess/streams/bufferstream.hpp>
#include <sstream>

// special types
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

// binary archive
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

// argument types
#include <string>

namespace qi {
  namespace serialization {

    template<class T>
    std::string BoostXmlSerializer::serialize(const T& item)
    {
      std::stringstream stream;
      boost::archive::xml_oarchive oa(stream, boost::archive::no_header);
      oa << boost::serialization::make_nvp("t", item);
      return stream.str();
    }

    template<class T>
    T BoostXmlSerializer::deserialize(const std::string & buffer)
    {
      T ret;
      deserialize<T>((char*)buffer.c_str(), buffer.size(), ret);
      return ret;
    }

    template<class T>
    T BoostXmlSerializer::deserialize(char* chars, const int size)
    {
      T ret;
      deserialize<T>(chars, size, ret);
      return ret;
    }

    template<class T>
    void BoostXmlSerializer::deserialize(const std::string& buffer, T& ret)
    {
      deserialize<T>((char*)buffer.c_str(), buffer.size(), ret);
    }

    template<class T>
    void BoostXmlSerializer::deserialize(char* chars, const int size, T& ret)
    {
      boost::interprocess::bufferstream buff(chars, size);
      boost::archive::xml_iarchive ia(buff, boost::archive::no_header);
      ia >> boost::serialization::make_nvp("t", ret);
    }
  }
}

#endif  // _QI_SERIALIZATION_BOOST_BOOST_XML_SERIALIZER_HXX_
