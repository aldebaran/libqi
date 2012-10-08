/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/genericvalue.hpp>
#include <qimessaging/message.hpp>

#include <qimessaging/datastream.hpp>
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

namespace qi {

  template <typename T, typename T2, char S>
  static inline qi::IDataStream& deserialize(qi::IDataStream* ds, T &b)
  {
    T2 res;
    int ret = ds->read((void *)&res, sizeof(res));
    if (ret != sizeof(res))
      ds->setStatus(ds->Status_ReadPastEnd);
    __QI_DEBUG_SERIALIZATION_DATA_R(Type, b);
    b = res;
    return *ds;
  }

  template <typename T, typename T2, char S>
  static inline qi::ODataStream& serialize(qi::ODataStream* ds, T &b, bool inner)
  {
    T2 val = b;
    int ret = ds->write((const char*)&val, sizeof(val));
    if (!inner)
    {
      ds->buffer().signature() << S;
    }
    if (ret == -1)
      ds->setStatus(ds->Status_WriteError);
    __QI_DEBUG_SERIALIZATION_DATA_W(Type, b);
    return *ds;
  }

#define QI_SIMPLE_SERIALIZER_IMPL(Type, TypeCast, Signature)                   \
  IDataStream& IDataStream::operator>>(Type &b)                                \
  {                                                                            \
    return deserialize<Type, TypeCast, Signature>(this, b);                    \
  }                                                                            \
  ODataStream& ODataStream::operator<<(Type b)                                 \
  {                                                                            \
    bool sig = _innerSerialization;                                            \
    ++_innerSerialization;                                                     \
    serialize<Type, TypeCast, Signature>(this, b, sig);                        \
    --_innerSerialization;                                                     \
    return *this;                                                              \
  }

  QI_SIMPLE_SERIALIZER_IMPL(bool, bool, 'b')
  QI_SIMPLE_SERIALIZER_IMPL(char, char, 'c')
  QI_SIMPLE_SERIALIZER_IMPL(signed char, signed char, 'c')
  QI_SIMPLE_SERIALIZER_IMPL(unsigned char, unsigned char, 'C')
  QI_SIMPLE_SERIALIZER_IMPL(short, short, 'w')
  QI_SIMPLE_SERIALIZER_IMPL(unsigned short, unsigned short, 'W')
  QI_SIMPLE_SERIALIZER_IMPL(int, int, 'i')
  QI_SIMPLE_SERIALIZER_IMPL(unsigned int, unsigned int, 'I')
  QI_SIMPLE_SERIALIZER_IMPL(long, qi::int64_t, 'l')
  QI_SIMPLE_SERIALIZER_IMPL(unsigned long, qi::uint64_t, 'L')
  QI_SIMPLE_SERIALIZER_IMPL(long long, long long, 'l')
  QI_SIMPLE_SERIALIZER_IMPL(unsigned long long, unsigned long long, 'L')
  QI_SIMPLE_SERIALIZER_IMPL(float, float, 'f')
  QI_SIMPLE_SERIALIZER_IMPL(double, double, 'd')

  IDataStream::IDataStream(const qi::Buffer& buffer)
  : _status(Status_Ok)
  , _reader(BufferReader(buffer))
  {
  }

  ODataStream::ODataStream(qi::Buffer &buffer)
  : _status(Status_Ok),
    _innerSerialization(0)
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
      if (!_innerSerialization)
      {
        buffer().signature() << 's';
      }
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
    ++_innerSerialization;
    *this << (qi::uint32_t)len;
    --_innerSerialization;
    if (len) {
      if (!_innerSerialization)
      {
        buffer().signature() << 's';
      }
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

  ODataStream &ODataStream::operator<<(const qi::Buffer &meta) {
    if (!_innerSerialization)
    {
      buffer().signature() << "s";
    }

    ++_innerSerialization;

    *this << (uint32_t)meta.size();
    buffer().subBuffers().push_back(std::make_pair(buffer().size(), meta));

    --_innerSerialization;

    qiLogDebug("DataStream") << "Serializing buffer " << meta.size()
                             << " at " << buffer().size();
    return *this;
  }

  IDataStream &IDataStream::operator>>(qi::Buffer &meta) {
    BufferReader& reader = bufferReader();
    uint32_t sz;
    *this >> sz;
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
      memcpy(ptr, read(sz), sz);
    }
    return *this;
  }

  size_t IDataStream::read(void *data, size_t size)
  {
    return _reader.read(data, size);
  }

  void* IDataStream::read(size_t size)
  {
    return _reader.read(size);
  }

  IDataStream &IDataStream::operator>>(GenericValue &value)
  {
    std::string signature;
    *this >> signature;
    Type* type = 0; // Type::getCompatibleTypeWithSignature(signature);
    if (!type)
      qiLogError("qi.datastream") << "Could not find metatype for signature " << signature;
    else
    {
      value.type = type;
      value.value = 0; // value.type->deserialize(*this);
    }
    return *this;
  }

  ODataStream &ODataStream::operator<<(const GenericValue &value)
  {
    if (!_innerSerialization)
      buffer().signature() << "m";
    ++_innerSerialization;
    *this << value.signature();
    value.serialize(*this);
    --_innerSerialization;
    return *this;
  }

  void ODataStream::beginList(uint32_t size, std::string elementSignature)
  {
    if (!_innerSerialization)
      buffer().signature() << "[" << elementSignature;
    ++_innerSerialization;
    *this << size;
  }

  void ODataStream::endList()
  {
    --_innerSerialization;
    if (!_innerSerialization)
      buffer().signature() << "]";
  }

  void ODataStream::beginMap(uint32_t size, std::string keySignature, std::string valueSignature)
  {
    if (!_innerSerialization)
      buffer().signature() << "{" << keySignature << valueSignature << "}";
    ++_innerSerialization;
     *this << size;
  }

  void ODataStream::endMap()
  {
    --_innerSerialization;
    if (!_innerSerialization)
      buffer().signature() << "}";
  }

  void ODataStream::beginTuple(std::string sig)
  {
    if (!_innerSerialization)
       buffer().signature() << "(" << sig << ")";
    ++_innerSerialization;
  }
  void ODataStream::endTuple()
  {
    --_innerSerialization;
  }
}

