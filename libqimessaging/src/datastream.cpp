/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <qimessaging/serialization/message.hpp>
#include <qimessaging/datastream.hpp>
#include <vector>


namespace qi {

#if 0
#define __QI_DEBUG_SERIALIZATION_DATA_R(x, d) {            \
  std::string sig = qi::signature< x >::value();           \
  std::cout << "read (" << sig << "): " << d << std::endl; \
}

#define __QI_DEBUG_SERIALIZATION_DATA_W(x, d) {            \
  std::string sig = qi::signature< x >::value();           \
  std::cout << "write(" << sig << "): " << d << std::endl; \
}
#else
# define __QI_DEBUG_SERIALIZATION_DATA_R(x, d)
# define __QI_DEBUG_SERIALIZATION_DATA_W(x, d)
#endif

#define QI_SIMPLE_SERIALIZER_IMPL(Type)                    \
  DataStream& DataStream::operator>>(Type &b)               \
  {                                                        \
    b = *((Type *) (_data.data() + _index));               \
    _index += sizeof(Type);                                \
    __QI_DEBUG_SERIALIZATION_DATA_R(Type, b);              \
    return *this;                                          \
  }                                                        \
                                                           \
  DataStream& DataStream::operator<<(Type b)        \
  {                                                        \
    _data.append((char *)&b, sizeof(b));                   \
    __QI_DEBUG_SERIALIZATION_DATA_W(Type, b);              \
    return *this;                                          \
  }

  QI_SIMPLE_SERIALIZER_IMPL(bool);
  QI_SIMPLE_SERIALIZER_IMPL(char);
  QI_SIMPLE_SERIALIZER_IMPL(int);
  QI_SIMPLE_SERIALIZER_IMPL(float);
  QI_SIMPLE_SERIALIZER_IMPL(double);

  // string
  const char *DataStream::readString(size_t &len)
  {
    int sz;
    *this >> sz;
    len = sz;
    return _data.data();
  }

  void DataStream::writeString(const char *str, size_t len)
  {
    *this << (int)len;
    if (len) {
      _data.append(str, len);
      __QI_DEBUG_SERIALIZATION_DATA_W(std::string, str);
    }
  }

  // string
  DataStream& DataStream::operator>>(std::string &s)
  {
    int sz;
    *this >> sz;
    s.clear();
    if (sz) {
      s.append(_data.data() + _index, sz);
      _index += sz;
      __QI_DEBUG_SERIALIZATION_DATA_R(std::string, s);
    }

    return *this;
  }

  DataStream& DataStream::operator<<(const std::string &s)
  {
    *this << (int)s.size();
    if (!s.empty()) {
      _data.append(s.data(), s.size());
      __QI_DEBUG_SERIALIZATION_DATA_W(std::string, s);
    }

    return *this;
  }

  qi::DataStream &operator>>(qi::DataStream &sd, qi::Value &value)
  {
  }

  qi::DataStream &operator<<(qi::DataStream &sd, const qi::Value &value)
  {

  }

}

