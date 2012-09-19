/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <qimessaging/genericvalue.hpp>
#include <qimessaging/message.hpp>

#include <qimessaging/datastream.hpp>
#include <qimessaging/details/dynamicvalue.hpp>
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
      ds->getBuffer().signature() << S;
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
    return serialize<Type, TypeCast, Signature>(this, b, _innerSerialization); \
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
    _innerSerialization(false)
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
        getBuffer().signature() << 'r';
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
    *this << (qi::uint32_t)len;
    if (len) {
      if (!_innerSerialization)
      {
        getBuffer().signature() << 's';
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

  IDataStream &operator>>(qi::IDataStream &sd, qi::detail::DynamicValue &val)
  {
    std::string sig;
    qi::uint32_t type;
    val.clear();
    sd >> sig;
    sd >> type;
    switch(type) {
      case qi::detail::DynamicValue::Double:
        double d;
        sd >> d;
        val.setDouble(d);
        return sd;
      case qi::detail::DynamicValue::String:
        {
        std::string s;
        sd >> s;
        val.setString(s);
        return sd;
        }
      case qi::detail::DynamicValue::List:
        val.setList(detail::DynamicValue::DynamicValueList());
        sd >> *val.data.list;
        return sd;
      case qi::detail::DynamicValue::Map:
        val.setMap(detail::DynamicValue::DynamicValueMap());
        sd >> *val.data.map;
        return sd;
    };
    return sd;
  }

  ODataStream &ODataStream::operator<<(const qi::detail::DynamicValue &val)
  {
    switch(val.type) {
      case qi::detail::DynamicValue::Double:
        if (!_innerSerialization)
        {
          getBuffer().signature() << "Id";
        }
        *this << "Id" << (qi::uint32_t)val.type << val.data.d;
        break;
      case qi::detail::DynamicValue::String:
        if (!_innerSerialization)
        {
          getBuffer().signature() << "Is";
        }
        *this << "Is" << (qi::uint32_t)val.type << val.toString();
        break;
      case qi::detail::DynamicValue::List:
        if (!_innerSerialization)
        {
          getBuffer().signature() << "I[m]";
        }
        *this << "I[m]" << (qi::uint32_t)val.type << *val.data.list;
        break;
      case qi::detail::DynamicValue::Map:
        if (!_innerSerialization)
        {
          getBuffer().signature() << "I{sm}";
        }
        *this << "I{sm}" << (qi::uint32_t)val.type << *val.data.map;
        break;
      default:
        break;
    };
    return *this;
  }

  qi::SignatureStream &operator&(qi::SignatureStream &sd, const qi::detail::DynamicValue &value) {
    sd.write(qi::Signature::Type_Dynamic);
    return sd;
  }

  ODataStream &ODataStream::operator<<(const qi::Buffer &meta) {
    if (!_innerSerialization)
    {
      getBuffer().signature() << "r";
    }

    bool wasInnerSerialization = _innerSerialization;
    _innerSerialization = true;

    *this << (uint32_t)meta.size();
    getBuffer().subBuffers().push_back(std::make_pair(getBuffer().size(), meta));

    _innerSerialization = wasInnerSerialization;

    qiLogDebug("DataStream") << "Serializing buffer " << meta.size()
                             << " at " << getBuffer().size();
    return *this;
  }

  qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::Buffer &buffer) {
    os.write(qi::Signature::Type_Raw);
    return os;
  }

  IDataStream &IDataStream::operator>>(qi::Buffer &meta) {
    BufferReader& reader = getBufferReader();
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

  qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::GenericValue &value)
  {
    os.write(Signature::Type_Dynamic);
    return os;
  }

  IDataStream &IDataStream::operator>>(GenericValue &value)
  {
    std::string signature;
    *this >> signature;
    Type* type = Type::getCompatibleTypeWithSignature(signature);
    if (!type)
      qiLogError("qi.datastream") << "Could not find metatype for signature " << signature;
    else
    {
      value.type = type;
      value.value = value.type->deserialize(*this);
    }
    return *this;
  }

  ODataStream &ODataStream::operator<<(const GenericValue &value)
  {
    getBuffer().signature() << "m";
    *this << value.signature();
    value.serialize(*this);
    return *this;
  }

  void ODataStream::beginList(uint32_t size, std::string elementSignature)
  {
    getBuffer().signature() << "[" << elementSignature;
    *this << size;
  }

  void ODataStream::endList()
  {
    getBuffer().signature() << "]";
  }

  void ODataStream::beginMap(uint32_t size, std::string keySignature, std::string valueSignature)
  {
    getBuffer().signature() << "{" << keySignature << valueSignature << "}";
    *this << size;
  }

  void ODataStream::endMap()
  {
    getBuffer().signature() << "}";
  }

}

