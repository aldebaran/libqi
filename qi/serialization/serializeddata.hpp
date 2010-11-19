/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_SERIALIZATION_SERIALIZEDDATA_HPP__
#define   __QI_SERIALIZATION_SERIALIZEDDATA_HPP__

#include <iostream>
#include <qi/signature.hpp>

namespace qi {

  namespace serialization {

    class SerializedData {
    public:
      SerializedData() {}
      SerializedData(const std::string &data) : fData(data) {}

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

    template <typename T>
    struct serialize {
      static void read(SerializedData &sd, T &t){
        std::cout << "BAM: read" << std::endl;
        //#error "This type is not serializable"
      }

      static void write(SerializedData &sd, const T &t) {
        std::cout << "BAM: write" << std::endl;
        //#error "This type is not serializable"
      }
    };

    typedef SerializedData BinarySerializer;
  }
}

#include <qi/serialization/serializeddata.hxx>

#endif // __QI_SERIALIZATION_SERIALIZEDDATA_HPP__
