/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <qimessaging/metavalue.hpp>
#include <qimessaging/message.hpp>

#include <qimessaging/datastream.hpp>
#include <qimessaging/details/value.hpp>
#include <qi/log.hpp>
#include <qi/types.hpp>
#include <vector>
#include <cstring>
#include "src/buffer_p.hpp"


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


#define QI_SIMPLE_SERIALIZER_IMPL_FORCE_TYPE(Type, TypeCast)  \
  IDataStream& IDataStream::operator>>(Type &b)               \
  {                                                           \
    TypeCast res;                                             \
    int ret;                                                  \
    ret = read((void *)&res, sizeof(res));                    \
    if (ret != sizeof(res))                                   \
      setStatus(Status_ReadPastEnd)                           \
    __QI_DEBUG_SERIALIZATION_DATA_R(Type, b);                 \
    b = res;                                                  \
    return *this;                                             \
  }                                                           \
  ODataStream& ODataStream::operator<<(Type b)                \
  {                                                           \
    int ret;                                                  \
    TypeCast val = b;                                         \
    ret = write((const char*)&val, sizeof(val));              \
    if (ret == -1)                                            \
      setStatus(Status_WriteError);                           \
    __QI_DEBUG_SERIALIZATION_DATA_W(Type, b);                 \
    return *this;                                             \
  }


#define QI_SIMPLE_SERIALIZER_IMPL(Type)                                 \
    IDataStream& IDataStream::operator>>(Type &b)                       \
  {                                                                     \
    int ret;                                                            \
    ret = read((void *)&b, sizeof(Type));                               \
    if (ret != sizeof(Type))                                            \
      setStatus(Status_ReadPastEnd)                                     \
    __QI_DEBUG_SERIALIZATION_DATA_R(Type, b);                           \
    return *this;                                                       \
  }                                                                     \
                                                                        \
  ODataStream& ODataStream::operator<<(Type b)                          \
  {                                                                     \
    int ret;                                                            \
    ret = write((const char*)(const void *)&b, sizeof(b));              \
    if (ret == -1)                                                      \
      setStatus(Status_WriteError);                                     \
    __QI_DEBUG_SERIALIZATION_DATA_W(Type, b);                           \
    return *this;                                                       \
  }







namespace qi {

  QI_SIMPLE_SERIALIZER_IMPL(bool);
  QI_SIMPLE_SERIALIZER_IMPL(char);
  QI_SIMPLE_SERIALIZER_IMPL(signed char);
  QI_SIMPLE_SERIALIZER_IMPL(unsigned char);
  QI_SIMPLE_SERIALIZER_IMPL(short);
  QI_SIMPLE_SERIALIZER_IMPL(unsigned short);
  QI_SIMPLE_SERIALIZER_IMPL(int);
  QI_SIMPLE_SERIALIZER_IMPL(unsigned int);
  QI_SIMPLE_SERIALIZER_IMPL_FORCE_TYPE(long         , qi::int64_t);
  QI_SIMPLE_SERIALIZER_IMPL_FORCE_TYPE(unsigned long, qi::uint64_t);
  QI_SIMPLE_SERIALIZER_IMPL(long long);
  QI_SIMPLE_SERIALIZER_IMPL(unsigned long long);
  QI_SIMPLE_SERIALIZER_IMPL(float);
  QI_SIMPLE_SERIALIZER_IMPL(double);

  IDataStream::IDataStream(const qi::Buffer& buffer)
  : _status(Status_Ok)
  , _reader(BufferReader(buffer))
  {
  }

  ODataStream::ODataStream(qi::Buffer &buffer)
  : _status(Status_Ok)
  {
    if (!buffer._p)
      buffer._p = boost::shared_ptr<BufferPrivate>(new BufferPrivate());
    _buffer = buffer;
    ++_buffer._p->nWriters;
  }


  ODataStream::~ODataStream()
  {
    --_buffer._p->nWriters;
  }
  IDataStream::~IDataStream()
  {
  }

  int ODataStream::write(const char *str, size_t len)
  {
    if (len) {
      if (_buffer.write(str, len) < 0)
      {
        setStatus(Status_WriteError);
        __QI_DEBUG_SERIALIZATION_DATA_W(std::string, str);
        return -1;
      }
    }
    return len;
  }

  void ODataStream::writeString(const char *str, size_t len)
  {
    *this << (qi::uint32_t)len;
    if (len) {
      if (_buffer.write(str, len) != (int)len)
        setStatus(Status_WriteError);
      __QI_DEBUG_SERIALIZATION_DATA_W(std::string, str);
    }
  }

  // string
  IDataStream& IDataStream::operator>>(std::string &s)
  {
    qi::uint32_t sz = 0;
    *this >> sz;

    s.clear();
    if (sz) {
      char *data = static_cast<char *>(read(sz));
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

  ODataStream& ODataStream::operator<<(const std::string &s)
  {
    writeString(s.c_str(), s.length());
    return *this;
  }

  ODataStream& ODataStream::operator<<(const char *s)
  {
    qi::uint32_t len = strlen(s);
    writeString(s, len);
    __QI_DEBUG_SERIALIZATION_DATA_W(char *, s);
    return *this;
  }

  IDataStream &operator>>(qi::IDataStream &sd, qi::detail::Value &val)
  {
    std::string sig;
    qi::uint32_t type;
    val.clear();
    sd >> sig;
    sd >> type;
    switch(type) {
      case qi::detail::Value::Double:
        double d;
        sd >> d;
        val.setDouble(d);
        return sd;
      case qi::detail::Value::String:
        {
        std::string s;
        sd >> s;
        val.setString(s);
        return sd;
        }
      case qi::detail::Value::List:
        val.setList(detail::Value::ValueList());
        sd >> *val.data.list;
        return sd;
      case qi::detail::Value::Map:
        val.setMap(detail::Value::ValueMap());
        sd >> *val.data.map;
        return sd;
    };
    return sd;
  }

  qi::ODataStream &operator<<(qi::ODataStream &sd, const qi::detail::Value &val)
  {
    switch(val.type) {
      case qi::detail::Value::Double:
        sd << "Id";
        sd << (qi::uint32_t)val.type;
        sd << val.data.d;
        return sd;
      case qi::detail::Value::String:{
        sd << "Is";
        sd << (qi::uint32_t)val.type;
        sd << val.toString();
        return sd;
      }
      case qi::detail::Value::List:{
        sd << "I[m]";
        sd << (qi::uint32_t)val.type;
        sd << *val.data.list;
        return sd;
      }
      case qi::detail::Value::Map: {
        sd << "I{sm}";
        sd << (qi::uint32_t)val.type;
        sd << *val.data.map;
        return sd;
      }
      default:
        return sd;
    };
    return sd;
  }

  qi::SignatureStream &operator&(qi::SignatureStream &sd, const qi::detail::Value &value) {
    sd.write(qi::Signature::Type_Dynamic);
    return sd;
  }

  qi::ODataStream &operator<<(qi::ODataStream &stream, const qi::Buffer &meta) {
    qi::Buffer& parent = stream.getBuffer();
    stream << (uint32_t)meta.size();
    parent.subBuffers().push_back(std::make_pair(stream.getBuffer().size(),
      meta));
    qiLogDebug("DataStream") << "Serializing buffer " << meta.size() <<" at " << stream.getBuffer().size();
    return stream;
  }

  qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::Buffer &buffer) {
    os.write(qi::Signature::Type_Raw);
    return os;
  }

  qi::IDataStream &operator>>(qi::IDataStream &stream, qi::Buffer &meta) {
    BufferReader& reader = stream.getBufferReader();
    uint32_t sz;
    stream >> sz;
    if (reader.hasSubBuffer())
    {
      meta = reader.getSubBuffer();
      if (meta.size() != sz)
        qiLogWarning("DataStream") << "Buffer size mismatch " << sz << " " << meta.size();
    }
    else
    {
      qiLogDebug("DataStream") << "Extracting buffer of size " << sz <<" at " << reader.position();
      meta.clear();
      void* ptr = meta.reserve(sz);
      memcpy(ptr, stream.read(sz), sz);
    }
    return stream;
  }

  size_t IDataStream::read(void *data, size_t size)
  {
    return _reader.read(data, size);
  }

  void* IDataStream::read(size_t size)
  {
    return _reader.read(size);
  }

}

