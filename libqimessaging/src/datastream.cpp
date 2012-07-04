/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qi/log.hpp>
#include <vector>
#include <cstring>



#if 0

#include <qimessaging/signature.hpp>

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

#define QI_SIMPLE_SERIALIZER_IMPL(Type)                                 \
  DataStream& DataStream::operator>>(Type &b)                           \
  {                                                                     \
    int ret;                                                            \
    ret = _buffer.read((void *)&b, sizeof(Type));                       \
    if (ret != sizeof(Type))                                            \
      setStatus(Status_ReadPastEnd)                                     \
    __QI_DEBUG_SERIALIZATION_DATA_R(Type, b);                           \
    return *this;                                                       \
  }                                                                     \
                                                                        \
  DataStream& DataStream::operator<<(Type b)                            \
  {                                                                     \
    if (_ro) {                                                          \
      qiLogError("datastream", "cant write to a readonly buffer");      \
      setStatus(Status_WriteOnReadOnlyStream);                          \
      return *this;                                                     \
    }                                                                   \
    int ret;                                                            \
    ret = _buffer.write((Type *)&b, sizeof(b));                         \
    if (ret == -1)                                                      \
      setStatus(Status_WriteError);                                     \
    __QI_DEBUG_SERIALIZATION_DATA_W(Type, b);                           \
    return *this;                                                       \
  }


namespace qi {

  QI_SIMPLE_SERIALIZER_IMPL(bool);
  QI_SIMPLE_SERIALIZER_IMPL(char);
  QI_SIMPLE_SERIALIZER_IMPL(int8_t);
  QI_SIMPLE_SERIALIZER_IMPL(uint8_t);
  QI_SIMPLE_SERIALIZER_IMPL(int16_t);
  QI_SIMPLE_SERIALIZER_IMPL(uint16_t);
  QI_SIMPLE_SERIALIZER_IMPL(int32_t);
  QI_SIMPLE_SERIALIZER_IMPL(uint32_t);
  QI_SIMPLE_SERIALIZER_IMPL(int64_t);
  QI_SIMPLE_SERIALIZER_IMPL(uint64_t);
  QI_SIMPLE_SERIALIZER_IMPL(float);
  QI_SIMPLE_SERIALIZER_IMPL(double);

  DataStream::DataStream(const qi::Buffer &buffer)
    : _buffer(buffer),
      _ro(true),
      _status(Status_Ok)
  {
  }

  DataStream::DataStream(qi::Buffer &buffer)
    : _buffer(buffer),
      _ro(false),
      _status(Status_Ok)
  {
  }


  // string
  size_t DataStream::read(void *data, size_t len)
  {
    return _buffer.read(data, len);
  }

  void DataStream::write(const char *str, size_t len)
  {
    if (_ro) {
      qiLogError("datastream", "cant write to a readonly buffer");
      setStatus(Status_WriteOnReadOnlyStream);
      return;
    }
    if (len) {
      if (_buffer.write(str, len) != (int)len)
        setStatus(Status_WriteError);
      __QI_DEBUG_SERIALIZATION_DATA_W(std::string, str);
    }
  }

  void DataStream::writeString(const char *str, size_t len)
  {
    if (_ro) {
      qiLogError("datastream", "cant write to a readonly buffer");
      setStatus(Status_WriteOnReadOnlyStream);
      return;
    }
    *this << (uint32_t)len;
    if (len) {
      if (_buffer.write(str, len) != (int)len)
        setStatus(Status_WriteError);
      __QI_DEBUG_SERIALIZATION_DATA_W(std::string, str);
    }
  }

  // string
  DataStream& DataStream::operator>>(std::string &s)
  {
    uint32_t sz;
    *this >> sz;

    s.clear();
    if (sz) {
      char *data = static_cast<char *>(_buffer.read(sz));
      if (!data) {
        qiLogError("datastream", "buffer empty");
        setStatus(Status_ReadPastEnd);
        return *this;
      }
      s.append(data, sz);
      __QI_DEBUG_SERIALIZATION_DATA_R(std::string, s);
    }

    return *this;
  }

  DataStream& DataStream::operator<<(const std::string &s)
  {
    if (_ro) {
      qiLogError("datastream", "cant write to a readonly buffer");
      setStatus(Status_WriteOnReadOnlyStream);
      return *this;
    }
    *this << (uint32_t)s.size();
    if (!s.empty()) {
      if (_buffer.write(s.data(), s.size()) != (int)s.size())
        setStatus(Status_WriteError);
      __QI_DEBUG_SERIALIZATION_DATA_W(std::string, s);
    }

    return *this;
  }

  DataStream& DataStream::operator<<(const char *s)
  {
    if (_ro) {
      qiLogError("datastream", "cant write to a readonly buffer");
      setStatus(Status_WriteOnReadOnlyStream);
      return *this;
    }
    uint32_t len = strlen(s);
    writeString(s, len);
    __QI_DEBUG_SERIALIZATION_DATA_W(char *, s);
    return *this;
  }


  qi::DataStream &operator>>(qi::DataStream &sd, qi::Value &val)
  {
    std::string sig;
    uint32_t type;
    val.clear();
    sd >> type;
    sd >> sig;
    switch(type) {
      case qi::Value::Bool:
        val._private.type = qi::Value::Bool;
        sd >> val._private.data.b;
        return sd;
      case qi::Value::Char:
        val._private.type = qi::Value::Char;
        sd >> val._private.data.c;
        return sd;
      case qi::Value::Int32:
        val._private.type = qi::Value::Int32;
        sd >> val._private.data.i;
        return sd;
      case Value::UInt32:
      case Value::Int64:
      case Value::UInt64:
        qiLogError("datasteam") << "not implemented";
        sd.setStatus(DataStream::Status_ReadError);
        return sd;
      case qi::Value::Float:
        val._private.type = qi::Value::Float;
        sd >> val._private.data.f;
        return sd;
      case qi::Value::Double:
        val._private.type = qi::Value::Double;
        sd >> val._private.data.d;
        return sd;
      case qi::Value::String:
        val.setType(qi::Value::String);
        sd >> *val.value<std::string>();
        return sd;
      case qi::Value::List:
        val.setType(qi::Value::List);
        sd >> *val.value< std::list<qi::Value> >();
        return sd;
      case qi::Value::Vector:
        val.setType(qi::Value::Vector);
        sd >> *val.value< std::vector<qi::Value> >();
        return sd;
      case qi::Value::Map:
        val.setType(qi::Value::Map);
        sd >> *val.value< std::map<std::string, qi::Value> >();
        return sd;
    };
    return sd;
  }

  qi::DataStream &operator<<(qi::DataStream &sd, const qi::Value &val)
  {
    sd << (uint32_t)val.type();
    switch(val.type()) {
      case qi::Value::Bool:
        sd << "b";
        sd << val._private.data.b;
        return sd;
      case qi::Value::Char:
        sd << "c";
        sd << val._private.data.c;
        return sd;
      case qi::Value::Int32:
        sd << "i";
        sd<< val._private.data.i;
        return sd;
      case Value::UInt32:
      case Value::Int64:
      case Value::UInt64:
        qiLogError("datasteam") << "not implemented";
        sd.setStatus(DataStream::Status_WriteError);
        return sd;
      case qi::Value::Float:
        sd << "f";
        sd << val._private.data.f;
        return sd;
      case qi::Value::Double:
        sd << "d";
        sd << val._private.data.d;
        return sd;
      case qi::Value::String:{
        sd << "s";
        sd << *val.value< std::string >();
        return sd;
      }
      case qi::Value::List:{
        sd << "[m]";
        const std::list<qi::Value> *le = val.value< std::list<qi::Value> >();
        sd << *le;
        return sd;
      }
      case qi::Value::Vector: {
        sd << "[m]";
        const std::vector<qi::Value> *ve = val.value< std::vector<qi::Value> >();
        sd << *ve;
        return sd;
      }
      case qi::Value::Map: {
        sd << "{sm}";
        const std::map<std::string, qi::Value> *me = val.value< std::map<std::string, qi::Value> >();
        sd << *me;
        return sd;
      }
      default:
        return sd;
    };
    return sd;
  }

  qi::DataStream &operator<<(qi::DataStream &stream, const qi::Buffer &meta) {
    stream << (uint32_t)meta.size();
    stream.write((const char *)meta.data(), meta.size());
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, qi::Buffer &meta) {
    uint32_t sz;
    stream >> sz;
    meta.reserve(sz);
    stream.read(meta.data(), sz);
    return stream;
  }



}

