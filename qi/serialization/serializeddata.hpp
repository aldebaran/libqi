/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef     QI_SERIALIZATION_SERIALIZE_HPP_
# define     QI_SERIALIZATION_SERIALIZE_HPP_

#include <iostream>
#include <qi/signature.hpp>

namespace qi {

  namespace serialization {

    class SerializedData {
    public:
      SerializedData() {}
      SerializedData(const std::string &data) : fData(data) {}

      template<typename T>
      void read(T& t)
      {
        std::string sig = qi::signature<T>::value();

        std::cout << "read(" << fData << "):" << sig << std::endl;
      }

      template<typename T>
      void write(const T& t)
      {
        std::string sig = qi::signature<T>::value();
        std::cout << "write(" << fData << "):" << sig << std::endl;
      }

//      void read(std::string& s);
//      void write(const std::string& t);

//      void read(int& s);
//      void write(const int& t);

      std::string str()const {
        return fData;
      }

      void str(const std::string &str) {
        fData = str;
      }

    protected:
      std::string fData;
    };

    typedef SerializedData BinarySerializer;
  }
}

#include <qi/serialization/serializeddata.hxx>

#endif  // QI_SERIALIZATION_SERIALIZE_HPP_
