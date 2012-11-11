/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qitype/genericvalue.hpp>
#include <qimessaging/message.hpp>

#include <qimessaging/datastream.hpp>
#include "datastream_p.hpp"
#include <qi/log.hpp>
#include <qi/types.hpp>
#include <vector>
#include <cstring>

namespace qi {

  // Input
  template <typename T, typename T2, char S>
  static inline qi::IDataStream& deserialize(qi::IDataStream* ds, T &b)
  {
    T2 res;
    int ret = ds->read((void *)&res, sizeof(res));
    if (ret != sizeof(res))
      ds->setStatus(ds->Status_ReadPastEnd);
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
      ds->signature() += S;
    }
    if (ret == -1)
      ds->setStatus(ds->Status_WriteError);
    return *ds;
  }

#define QI_SIMPLE_SERIALIZER_IMPL(Type, TypeCast, Signature)                   \
  IDataStream& IDataStream::operator>>(Type &b)                                \
  {                                                                            \
    return deserialize<Type, TypeCast, Signature>(this, b);                    \
  }                                                                            \
  ODataStream& ODataStream::operator<<(Type b)                                 \
  {                                                                            \
    bool sig = (_p->_innerSerialization != 0);                                 \
    ++(_p->_innerSerialization);                                               \
    serialize<Type, TypeCast, Signature>(this, b, sig);                        \
    --(_p->_innerSerialization);                                               \
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
  : _p(new IDataStreamPrivate(buffer))
  {
  }

  IDataStream::~IDataStream()
  {
    delete _p;
  }

  size_t IDataStream::read(void *data, size_t size)
  {
    return _p->_reader.read(data, size);
  }

  void* IDataStream::read(size_t size)
  {
    return _p->_reader.read(size);
  }

  IDataStream::Status IDataStream::status() const
  {
    return _p->_status;
  }

  void IDataStream::setStatus(Status status)
  {
    _p->_status = status;
  }

  BufferReader& IDataStream::bufferReader()
  {
    return _p->_reader;
  }

  IDataStreamPrivate::IDataStreamPrivate(const qi::Buffer& buffer)
    : _status(IDataStream::Status_Ok), _reader(BufferReader(buffer))
  {
  }

  IDataStreamPrivate::~IDataStreamPrivate()
  {
  }

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
    }

    return *this;
  }

  IDataStream &IDataStream::operator>>(qi::Buffer &meta) {
    BufferReader& reader = bufferReader();
    uint32_t sz;
    *this >> sz;
    if (reader.hasSubBuffer())
    {
      meta = reader.subBuffer();
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

  IDataStream &IDataStream::operator>>(GenericValuePtr &value)
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

  // Output
  ODataStream::ODataStream(qi::Buffer &buffer)
    : _p(new ODataStreamPrivate(buffer))
  {
  }

  ODataStream::~ODataStream()
  {
    delete _p;
  }

  int ODataStream::write(const char *str, size_t len)
  {
    if (len) {
      if (!_p->_innerSerialization)
      {
        signature() += 's';
      }
      if (_p->_buffer.write(str, len) == false)
      {
        setStatus(Status_WriteError);
        return -1;
      }
    }
    return len;
  }

  void ODataStream::writeString(const char *str, size_t len)
  {
    ++_p->_innerSerialization;
    *this << (qi::uint32_t)len;
    --_p->_innerSerialization;
    if (len) {
      if (!_p->_innerSerialization)
      {
        signature() += 's';
      }
      if (_p->_buffer.write(str, len) == false)
        setStatus(Status_WriteError);
    }
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
    return *this;
  }

  ODataStream &ODataStream::operator<<(const qi::Buffer &meta) {
    if (!_p->_innerSerialization)
    {
      signature() += "r";
    }

    ++_p->_innerSerialization;

    *this << (uint32_t)meta.size();
    buffer().addSubBuffer(meta);

    --_p->_innerSerialization;

    qiLogDebug("DataStream") << "Serializing buffer " << meta.size()
                             << " at " << buffer().size();
    return *this;
  }

  ODataStream &ODataStream::operator<<(const GenericValuePtr &value)
  {
    if (!_p->_innerSerialization)
      signature() += "m";
    ++_p->_innerSerialization;
    *this << value.signature();
    qi::details::serialize(value, *this);
    --_p->_innerSerialization;
    return *this;
  }

  void ODataStream::beginList(uint32_t size, std::string elementSignature)
  {
    if (!_p->_innerSerialization)
      signature() += "[" + elementSignature;
    ++_p->_innerSerialization;
    *this << size;
  }

  void ODataStream::endList()
  {
    --_p->_innerSerialization;
    if (!_p->_innerSerialization)
      signature() += "]";
  }

  void ODataStream::beginMap(uint32_t size, std::string keySignature, std::string valueSignature)
  {
    if (!_p->_innerSerialization)
      signature() += "{" + keySignature + valueSignature + "}";
    ++_p->_innerSerialization;
     *this << size;
  }

  void ODataStream::endMap()
  {
    --_p->_innerSerialization;
    if (!_p->_innerSerialization)
      signature() += "}";
  }

  void ODataStream::beginTuple(std::string sig)
  {
    if (!_p->_innerSerialization)
       signature() += "(" + sig + ")";
    ++_p->_innerSerialization;
  }

  void ODataStream::endTuple()
  {
    --_p->_innerSerialization;
  }

  ODataStream::Status ODataStream::status() const
  {
    return _p->_status;
  }

  void ODataStream::setStatus(Status status)
  {
    _p->_status = status;
  }

  Buffer& ODataStream::buffer()
  {
    return _p->_buffer;
  }

  std::string& ODataStream::signature()
  {
    return _p->_signature;
  }

  ODataStreamPrivate::ODataStreamPrivate(qi::Buffer &buffer)
    : _status(ODataStream::Status_Ok), _innerSerialization(0)
  {
    _buffer = buffer;
  }

  ODataStreamPrivate::~ODataStreamPrivate()
  {
  }

  namespace details {
    void serialize(GenericValuePtr val, ODataStream& out)
    {
      SerializeTypeVisitor stv(out);
      typeDispatch(stv, val.type, &val.value);
    }

    GenericValuePtr deserialize(qi::Type *type, IDataStream& in) {
      void* storage = 0;
      DeserializeTypeVisitor dtv(in);
      typeDispatch(dtv, type, &storage);
      return dtv.result;
    }
  }

}

