/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris KILNER  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/serialization/serializeddata.hpp>
#include <vector>

namespace qi {
  namespace serialization {


#define SIMPLE_SERIALIZER_IMPL(Name, Type)                   \
    void SerializedData::read##Name(Type& b)                 \
    {                                                        \
      b = *((Type *) fData.data());                          \
      fData.erase(0, sizeof(Type));                          \
    }                                                        \
                                                             \
    void SerializedData::write##Name(const Type& b)          \
    {                                                        \
      fData.append((char *)&b, sizeof(b));                   \
    }

    SIMPLE_SERIALIZER_IMPL(Bool, bool);
    SIMPLE_SERIALIZER_IMPL(Char, char);
    SIMPLE_SERIALIZER_IMPL(Int, int);
    SIMPLE_SERIALIZER_IMPL(Float, float);
    //SIMPLE_SERIALIZER(Float, double);

    void SerializedData::readDouble(double& d)
    {
      memcpy(&d, fData.data(), sizeof(double));
      fData.erase(0, sizeof(double));
      //std::string sig = qi::signature<double>::value();
      //std::cout << "readD(" << d << "):" << sig << std::endl;
    }

    void SerializedData::writeDouble(const double& d)
    {
      fData.append((char *)&d, sizeof(d));
      //std::string sig = qi::signature<double>::value();
      //std::cout << "writeD(" << d << "):" << sig << std::endl;
    }

    // string
    void SerializedData::readString(std::string& s)
    {
      int sz;
      readInt(sz);
      s.clear();
      if (sz) {
        s.append(fData.data(), sz);
        fData.erase(0, sz);
      }
      //std::string sig = qi::signature<std::string>::value();
      //std::cout << "readS(" << s << "):" << sig << std::endl;
    }

    void SerializedData::writeString(const std::string &s)
    {
      writeInt(s.size());
      if (s.size())
        fData.append(s.data(), s.size());

      //std::string sig = qi::signature<std::string>::value();
      //std::cout << "writeSc(" << s << "):" << sig << std::endl;
    }

  }
}

