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

    class Message {
    public:
      Message() {}
      Message(const std::string &data) : fData(data) {}

      void readBool(bool& s);
      void writeBool(const bool& t);

      void readChar(char& s);
      void writeChar(const char& t);

      void readInt(int& s);
      void writeInt(const int& t);

      void readFloat(float& s);
      void writeFloat(const float& t);

      void readString(std::string& s);
      void writeString(const std::string& t);

      void readDouble(double& d);
      void writeDouble(const double& d);


      std::string str()const {
        return fData;
      }

      void str(const std::string &str) {
        fData = str;
      }

    protected:
      std::string fData;
    };

    //Enable is need for protobuf (for conditional template specialization)
    template <typename T, class Enable = void>
    struct serialize {
      static void read(Message &sd, T &t){
        std::cout << "BAM: read" << std::endl;
        //#error "This type is not serializable"
      }

      static void write(Message &sd, const T &t) {
        std::cout << "BAM: write" << std::endl;
        //#error "This type is not serializable"
      }
    };

  }
}

#include <qi/serialization/message.hxx>

#endif  // _QI_SERIALIZATION_MESSAGE_HPP_
