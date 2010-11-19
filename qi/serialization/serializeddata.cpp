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

#if 0
#define __QI_DEBUG_SERIALIZATION_DATA_R(x, d) {            \
  std::string sig = qi::signature< x >::value();           \
  std::cout << "read (" << sig << "): " << d << std::endl;  \
}

#define __QI_DEBUG_SERIALIZATION_DATA_W(x, d) {            \
  std::string sig = qi::signature< x >::value();           \
  std::cout << "write(" << sig << "): " << d << std::endl; \
}
#else
# define __QI_DEBUG_SERIALIZATION_DATA_R(x, d)
# define __QI_DEBUG_SERIALIZATION_DATA_W(x, d)
#endif

#define SIMPLE_SERIALIZER_IMPL(Name, Type)                   \
    void SerializedData::read##Name(Type& b)                 \
    {                                                        \
      b = *((Type *) fData.data());                          \
      fData.erase(0, sizeof(Type));                          \
      __QI_DEBUG_SERIALIZATION_DATA_R(Type, b);              \
    }                                                        \
                                                             \
    void SerializedData::write##Name(const Type& b)          \
    {                                                        \
      fData.append((char *)&b, sizeof(b));                   \
      __QI_DEBUG_SERIALIZATION_DATA_W(Type, b);              \
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
      __QI_DEBUG_SERIALIZATION_DATA_R(double, d);
    }

    void SerializedData::writeDouble(const double& d)
    {
      fData.append((char *)&d, sizeof(d));
      __QI_DEBUG_SERIALIZATION_DATA_W(double, d);
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
        __QI_DEBUG_SERIALIZATION_DATA_R(std::string, s);
      }
    }

    void SerializedData::writeString(const std::string &s)
    {
      writeInt(s.size());
      if (s.size()) {
        fData.append(s.data(), s.size());
        __QI_DEBUG_SERIALIZATION_DATA_W(std::string, s);
      }
    }

  }
}

