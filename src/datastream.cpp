/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qitype/genericvalue.hpp>
#include <qimessaging/message.hpp>

#include <qimessaging/idatastream.hpp>
#include <qimessaging/odatastream.hpp>
#include "datastream_p.hpp"
#include <qi/log.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/typedispatcher.hpp>
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

#undef QI_SIMPLE_SERIALIZER_IMPL

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
    class SerializeTypeVisitor
    {
    public:
      SerializeTypeVisitor(ODataStream& out)
        : out(out)
      {}

      void visitUnknown(Type* type, void* storage)
      {
        qiLogError("qi.type") << "Type " << type->infoString() <<" not serializable";
      }

      void visitVoid(Type*)
      {
        qiLogError("qi.type") << "serialize() called on void";
      }

      void visitInt(TypeInt* type, int64_t value, bool isSigned, int byteSize)
      {
        switch((isSigned?1:-1)*byteSize)
        {
          case 1:	 out <<(int8_t)value;  break;
          case -1: out <<(uint8_t)value; break;
          case 2:	 out <<(int16_t)value; break;
          case -2: out <<(uint16_t)value;break;
          case 4:	 out <<(int32_t)value; break;
          case -4: out <<(uint32_t)value;break;
          case 8:	 out <<(int64_t)value; break;
          case -8: out <<(uint64_t)value;break;
          default:
            qiLogError("qi.type") << "Unknown integer type " << isSigned << " " << byteSize;
        }
      }

      void visitFloat(TypeFloat* type, double value, int byteSize)
      {
        if (byteSize == 4)
          out << (float)value;
        else if (byteSize == 8)
          out << (double)value;
        else
          qiLogError("qi.type") << "serialize on unknown float type " << byteSize;
      }

      void visitString(TypeString* type, void* storage)
      {
        std::pair<char*, size_t> data = type->get(storage);
        out.writeString(data.first, data.second);
      }

      void visitList(GenericListPtr value)
      {
        out.beginList(value.size(), value.elementType()->signature());
        GenericListIteratorPtr it, end;
        it = value.begin();
        end = value.end();
        for (; it != end; ++it)
          qi::details::serialize(*it, out);
        it.destroy();
        end.destroy();
        out.endList();
      }

      void visitMap(GenericMapPtr value)
      {
        out.beginMap(value.size(), value.keyType()->signature(), value.elementType()->signature());
        GenericMapIteratorPtr it, end;
        it = value.begin();
        end = value.end();
        for(; it != end; ++it)
        {
          std::pair<GenericValuePtr, GenericValuePtr> v = *it;
          qi::details::serialize(v.first, out);
          qi::details::serialize(v.second, out);
        }
        it.destroy();
        end.destroy();
        out.endMap();
      }

      void visitObject(GenericObject value)
      {
        qiLogError("qi.type") << "Object serialization not implemented";
      }

      void visitPointer(TypePointer* type, void* storage, GenericValuePtr pointee)
      {
        qiLogError("qi.type") << "Pointer serialization not implemented";
      }

      void visitTuple(TypeTuple* type, void* storage)
      {
        std::vector<GenericValuePtr> vals = type->getValues(storage);
        std::string tsig;
        for (unsigned i=0; i<vals.size(); ++i)
          tsig += vals[i].type->signature();
        out.beginTuple(tsig);
        for (unsigned i=0; i<vals.size(); ++i)
          qi::details::serialize(vals[i], out);
        out.endTuple();
      }

      void visitDynamic(Type* type, GenericValuePtr pointee)
      {
        out << pointee;
      }

      void visitRaw(TypeRaw* type, Buffer* buffer)
      {
        out << *buffer;
      }
      ODataStream& out;
    };

    class DeserializeTypeVisitor
    {
      /* Value passed to visitor functions should not be used, only type
    * information should.
    */
    public:
      DeserializeTypeVisitor(IDataStream& in)
        : in(in)
      {}

      template<typename T, typename TYPE>
      void deserialize(TYPE* type)
      {
        T val;
        in >> val;
        assert(typeOf<T>()->info() == type->info());
        result.type = type;
        result.value = type->initializeStorage();
        type->set(&result.value, val);
      }

      void visitUnknown(Type* type, void* storage)
      {
        qiLogError("qi.type") << "Type " << type->infoString() <<" not deserializable";
      }

      void visitVoid(Type*)
      {
        result.type = typeOf<void>();
        result.value = 0;
      }

      void visitInt(TypeInt* type, int64_t value, bool isSigned, int byteSize)
      {
        switch((isSigned?1:-1)*byteSize)
        {
          case 1:  deserialize<int8_t>(type);  break;
          case -1: deserialize<uint8_t>(type); break;
          case 2:  deserialize<int16_t>(type); break;
          case -2: deserialize<uint16_t>(type);break;
          case 4:  deserialize<int32_t>(type); break;
          case -4: deserialize<uint32_t>(type);break;
          case 8:  deserialize<int64_t>(type); break;
          case -8: deserialize<uint64_t>(type);break;
          default:
            qiLogError("qi.type") << "Unknown integer type " << isSigned << " " << byteSize;
        }
      }

      void visitFloat(TypeFloat* type, double value, int byteSize)
      {
        if (byteSize == 4)
          deserialize<float>(type);
        else if (byteSize == 8)
          deserialize<double>(type);
        else
          qiLogError("qi.type") << "Unknown float type " << byteSize;
      }

      void visitString(TypeString* type, const void* storage)
      {
        result.type = type;
        result.value = result.type->initializeStorage();
        std::string s;
        in >> s;
        type->set(&result.value, s);
      }

      void visitList(GenericListPtr value)
      {
        result.type = value.type;
        result.value = value.type->initializeStorage();
        GenericListPtr res(result);
        Type* elementType = res.elementType();
        qi::uint32_t sz = 0;
        in >> sz;
        if (in.status() != IDataStream::Status_Ok)
          return;
        for (unsigned i = 0; i < sz; ++i)
        {
          res.pushBack(qi::details::deserialize(elementType, in));
        }
      }

      void visitMap(GenericMapPtr value)
      {
        result.type = value.type;
        result.value = value.type->initializeStorage();
        GenericMapPtr res(result);
        Type* keyType = res.keyType();
        Type* elementType = res.elementType();
        qi::uint32_t sz = 0;
        in >> sz;
        if (in.status() != IDataStream::Status_Ok)
          return;
        for (unsigned i = 0; i < sz; ++i)
        {
          GenericValuePtr k = qi::details::deserialize(keyType, in);
          GenericValuePtr v = qi::details::deserialize(elementType, in);
          res.insert(k, v);
        }
      }

      void visitObject(GenericObject value)
      {
        qiLogError("qi.type") << " Object serialization not implemented";
      }

      void visitPointer(TypePointer* type, void* storage, GenericValuePtr pointee)
      {
        qiLogError("qi.type") << " Pointer serialization not implemented";
      }

      void visitTuple(TypeTuple* type, void* storage)
      {
        std::vector<Type*> types = type->memberTypes(storage);
        result.type = type;
        result.value = type->initializeStorage();
        for (unsigned i = 0; i<types.size(); ++i)
        {
          GenericValuePtr val = qi::details::deserialize(types[i], in);
          type->set(&result.value, i, val.value);
          val.destroy();
        }
      }

      void visitDynamic(Type* type, GenericValuePtr pointee)
      {
        std::string sig;
        in >> sig;
        GenericValuePtr val;
        val.type = Type::fromSignature(sig);
        val = qi::details::deserialize(val.type, in);
        result.type = type;
        result.value = result.type->initializeStorage();
        static_cast<TypeDynamic*>(type)->set(&result.value, val);
        val.destroy();
      }

      void visitRaw(TypeRaw* type, Buffer*)
      {
        Buffer b;
        in >> b;
        void* s = type->initializeStorage();
        type->set(&s, b);
        result.type = type;
        result.value = s;
      }
      GenericValuePtr result;
      IDataStream& in;
    }; //class

  } // namespace details

  namespace details {
    void serialize(GenericValuePtr val, ODataStream& out)
    {
      SerializeTypeVisitor stv(out);
      qi::typeDispatch(stv, val.type, &val.value);
      if (out.status() != ODataStream::Status_Ok) {
        qiLogError("qi.datastream") << "OSerialization error";
      }
    }

    GenericValuePtr deserialize(qi::Type *type, IDataStream& in) {
      void* storage = 0;
      DeserializeTypeVisitor dtv(in);
      qi::typeDispatch(dtv, type, &storage);
      if (in.status() != IDataStream::Status_Ok) {
        qiLogError("qi.datastream") << "ISerialization error";
      }
      return dtv.result;
    }
  }



}

