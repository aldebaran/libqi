#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_SERIALIZATION_DATASTREAM_HPP_
#define _QIMESSAGING_SERIALIZATION_DATASTREAM_HPP_

#include <iostream>
#include <qimessaging/api.hpp>

namespace qi {

class QIMESSAGING_API DataStream {
public:

  /// <summary>Default constructor. </summary>
  DataStream()
    : _index(0)
  {}

  /// <summary>Default constructor.</summary>
  /// <param name="data">The data.</param>
  DataStream(const std::string &data)
    : _data(data),
      _index(0)
  {}

  const char *readString(size_t &len);
  void writeString(const char *str, size_t len);

  DataStream& operator<<(bool i);
  DataStream& operator<<(char i);
  DataStream& operator<<(int i);
  DataStream& operator<<(float i);
  DataStream& operator<<(double i);
  DataStream& operator<<(std::string i);

  DataStream& operator>>(const bool& i);
  DataStream& operator>>(const char& i);
  DataStream& operator>>(const int& i);
  DataStream& operator>>(const float& i);
  DataStream& operator>>(const double& i);
  DataStream& operator>>(const std::string& i);

  /// <summary>Gets the string. </summary>
  /// <returns> The string representation of the serialized message</returns>
  std::string str()const {
    return _data;
  }

  /// <summary>Gets the string.</summary>
  /// <param name="str">The result string.</param>
  void str(const std::string &str) {
    _data = str;
    _index = 0;
  }

protected:
  /// <summary> The underlying data </summary>
  std::string _data;
  long        _index;
};


}

#endif  // _QIMESSAGING_SERIALIZATION_MESSAGE_HPP_
