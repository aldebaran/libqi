#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_SERIALIZATION_MESSAGE_HPP_
#define _QIMESSAGING_SERIALIZATION_MESSAGE_HPP_

#include <iostream>
#include <qimessaging/api.hpp>
//#include <qimessaging/signature.hpp>

namespace qi {

  /// <summary>A serialized message</summary>
  /// \ingroup Serialization
  class QIMESSAGING_API Message {
  public:

    /// <summary>Default constructor. </summary>
    Message()
      : _index(0)
    {}

    /// <summary>Default constructor.</summary>
    /// <param name="data">The data.</param>
    Message(const std::string &data)
      : _data(data),
        _index(0)
    {}

    /// <summary>Reads a bool. </summary>
    /// <param name="s">The result</param>
    void readBool(bool& s);

    /// <summary>Writes a bool. </summary>
    /// <param name="t">The bool</param>
    void writeBool(const bool& t);

    /// <summary>Reads a character. </summary>
    /// <param name="s">The result.</param>
    void readChar(char& s);

    /// <summary>Writes a character. </summary>
    /// <param name="t">The character.</param>
    void writeChar(const char& t);

    /// <summary>Reads an int. </summary>
    /// <param name="s">The result.</param>
    void readInt(int& s);

    /// <summary>Writes an int. </summary>
    /// <param name="t">The int.</param>
    void writeInt(const int& t);

    /// <summary>Reads a float. </summary>
    /// <param name="s">The result.</param>
    void readFloat(float& s);

    /// <summary>Writes a float. </summary>
    /// <param name="t">The float.</param>
    void writeFloat(const float& t);

    /// <summary>Reads a string. </summary>
    /// <param name="s">[in,out] The result.</param>
    void readString(std::string& s);

    /// <summary>Writes a string. </summary>
    /// <param name="t">The string.</param>
    void writeString(const std::string& t);

    const char *readString(size_t &len);
    void writeString(const char *str, size_t len);


    /// <summary>Reads a double. </summary>
    /// <param name="d">The result</param>
    void readDouble(double& d);

    /// <summary>Writes a double. </summary>
    /// <param name="d">The double</param>
    void writeDouble(const double& d);

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
