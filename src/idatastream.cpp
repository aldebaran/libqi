/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/typedispatcher.hpp>
#include <qimessaging/idatastream.hpp>
#include "idatastream_p.hpp"

namespace qi {
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

#define QI_SIMPLE_SERIALIZER_IMPL(Type, TypeCast, Signature)                   \
  IDataStream& IDataStream::operator>>(Type &b)                                \
  {                                                                            \
    return deserialize<Type, TypeCast, Signature>(this, b);                    \
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

  namespace details {
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
    };

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
