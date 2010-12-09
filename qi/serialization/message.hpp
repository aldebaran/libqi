#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SERIALIZATION_MESSAGE_HPP_
#define _QI_SERIALIZATION_MESSAGE_HPP_

#include <iostream>
#include <qi/signature.hpp>

namespace qi {

  namespace serialization {

    /// <summary>A serialized message</summary>
    /// \ingroup Serialization
    class Message {
    public:

      /// <summary>Default constructor. </summary>
      Message() {}

      /// <summary>Default constructor.</summary>
      /// <param name="data">The data.</param>
      Message(const std::string &data) : fData(data) {}

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

      /// <summary>Reads a double. </summary>
      /// <param name="s">The result</param>
      void readDouble(double& d);

      /// <summary>Writes a double. </summary>
      /// <param name="d">The double</param>
      void writeDouble(const double& d);

      /// <summary>Gets the string. </summary>
      /// <returns> The string representation of the serialized message</returns>
      std::string str()const {
        return fData;
      }

      /// <summary>Gets the string.</summary>
      /// <param name="str">The result string.</param>
      void str(const std::string &str) {
        fData = str;
      }

    protected:
      /// <summary> The underlying data </summary>
      std::string fData;
    };

    //Enable is need for protobuf (for conditional template specialization)
    template <typename T, class Enable = void>
    struct serialize {
      static void read(Message &sd, T &t){
        std::cout << "ERROR: this type is not serializable" << std::endl;
        //#error "This type is not serializable"
      }

      static void write(Message &sd, const T &t) {
        std::cout << "ERROR: this type is not serializable" << std::endl;
        //#error "This type is not serializable"
      }
    };

  }
}

#include <qi/serialization/message.hxx>

#endif  // _QI_SERIALIZATION_MESSAGE_HPP_
