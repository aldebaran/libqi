/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/genericvalue.hpp>
#include <qimessaging/message.hpp>

#include <qimessaging/binaryencoder.hpp>
#include <qimessaging/binarydecoder.hpp>
#include "binarycoder_p.hpp"
#include "boundobject.hpp"
#include "remoteobject_p.hpp"
#include <qi/log.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/typedispatcher.hpp>
#include <qi/types.hpp>
#include <vector>
#include <cstring>

qiLogCategory("qimessaging.binarycoder");

namespace qi {
  template <typename T, typename T2, char S>
  static inline qi::BinaryDecoder& deserialize(qi::BinaryDecoder* ds, T &b)
  {
    T2 res;
    int ret = ds->readRaw((void *)&res, sizeof(res));
    if (ret != sizeof(res))
      ds->setStatus(ds->Status_ReadPastEnd);
    b = static_cast<T>(res);
    return *ds;
  }

  template <typename T, typename T2, char S>
  static inline qi::BinaryEncoder& serialize(qi::BinaryEncoder* ds, T &b, bool inner)
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
  void BinaryDecoder::read(Type &b)                                            \
  {                                                                            \
    deserialize<Type, TypeCast, Signature>(this, b);                           \
  }                                                                            \
  void BinaryEncoder::write(Type b)                                            \
  {                                                                            \
    bool sig = (_p->_innerSerialization != 0);                                 \
    ++(_p->_innerSerialization);                                               \
    serialize<Type, TypeCast, Signature>(this, b, sig);                        \
    --(_p->_innerSerialization);                                               \
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

  BinaryDecoder::BinaryDecoder(const qi::Buffer& buffer)
    : _p(new BinaryDecoderPrivate(buffer))
  {
  }

  BinaryDecoder::~BinaryDecoder()
  {
    delete _p;
  }

  size_t BinaryDecoder::readRaw(void *data, size_t size)
  {
    return _p->_reader.read(data, size);
  }

  void* BinaryDecoder::readRaw(size_t size)
  {
    return _p->_reader.read(size);
  }

  BinaryDecoder::Status BinaryDecoder::status() const
  {
    return _p->_status;
  }

  void BinaryDecoder::setStatus(Status status)
  {
    _p->_status = status;
  }

  BufferReader& BinaryDecoder::bufferReader()
  {
    return _p->_reader;
  }

  BinaryDecoderPrivate::BinaryDecoderPrivate(const qi::Buffer& buffer)
    : _status(BinaryDecoder::Status_Ok), _reader(BufferReader(buffer))
  {
  }

  BinaryDecoderPrivate::~BinaryDecoderPrivate()
  {
  }

  void BinaryDecoder::read(std::string &s)
  {
    qi::uint32_t sz = 0;
    read(sz);

    s.clear();
    if (sz) {
      char *data = static_cast<char *>(readRaw(sz));
      if (!data) {
        qiLogError();
        setStatus(Status_ReadPastEnd);
        return;
      }
      s.append(data, sz);
    }
  }

  void BinaryDecoder::read(qi::Buffer &meta) {
    BufferReader& reader = bufferReader();
    if (reader.hasSubBuffer())
    {
      meta = reader.subBuffer();
    }
    else
    {
      uint32_t sz;
      read(sz);
      qiLogDebug() << "Extracting buffer of size " << sz <<" at " << reader.position();
      meta.clear();
      void* ptr = meta.reserve(sz);
      memcpy(ptr, readRaw(sz), sz);
    }
  }

  void BinaryDecoder::read(GenericValuePtr &value)
  {
    std::string signature;
    read(signature);
    Type* type = 0; // Type::getCompatibleTypeWithSignature(signature);
    if (!type)
      qiLogError() << "Could not find metatype for signature " << signature;
    else
    {
      value.type = type;
      value.value = 0; // value.type->deserialize(*this);
    }
  }

  // Output
  BinaryEncoder::BinaryEncoder(qi::Buffer &buffer)
    : _p(new BinaryEncoderPrivate(buffer))
  {
  }

  BinaryEncoder::~BinaryEncoder()
  {
    delete _p;
  }

  int BinaryEncoder::write(const char *str, size_t len)
  {
    if (len) {
      if (!_p->_innerSerialization)
      {
        signature() += 's';
      }
      if (_p->_buffer.write(str, len) == false)
      {
        setStatus(Status_WriteError);
      }
    }
    return len;
  }

  void BinaryEncoder::writeString(const char *str, size_t len)
  {
    ++_p->_innerSerialization;
    write((qi::uint32_t)len);
    --_p->_innerSerialization;
    if (!_p->_innerSerialization)
    {
      signature() += 's';
    }
    if (len) {
      if (_p->_buffer.write(str, len) == false)
        setStatus(Status_WriteError);
    }
  }

  void BinaryEncoder::write(const std::string &s)
  {
    writeString(s.c_str(), s.length());
  }

  void BinaryEncoder::write(const char *s)
  {
    qi::uint32_t len = strlen(s);
    writeString(s, len);
  }

  void BinaryEncoder::write(const qi::Buffer &meta) {
    if (!_p->_innerSerialization)
    {
      signature() += "r";
    }

    buffer().addSubBuffer(meta);

    // qiLogDebug() << "Serializing buffer " << meta.size()
    //                         << " at " << buffer().size();
  }

  void BinaryEncoder::write(const GenericValuePtr &value,
    boost::function<void()> recurse)
  {
    if (!_p->_innerSerialization)
      signature() += "m";
    ++_p->_innerSerialization;
    std::string sig = value.signature();
    if (sig.empty())
      sig = "v";
    if (Signature(sig).size() != 1)
      qiLogWarning() << "Weird GenericValuePtr signature: " << sig;
    write(sig);
    if (!recurse)
      qi::details::serialize(value, *this);
    else
      recurse();
    --_p->_innerSerialization;
  }

  void BinaryEncoder::beginList(uint32_t size, std::string elementSignature)
  {
    if (!_p->_innerSerialization)
      signature() += "[" + elementSignature;
    ++_p->_innerSerialization;
    write(size);
  }

  void BinaryEncoder::endList()
  {
    --_p->_innerSerialization;
    if (!_p->_innerSerialization)
      signature() += "]";
  }

  void BinaryEncoder::beginMap(uint32_t size, std::string keySignature, std::string valueSignature)
  {
    if (!_p->_innerSerialization)
      signature() += "{" + keySignature + valueSignature + "}";
    ++_p->_innerSerialization;
    write(size);
  }

  void BinaryEncoder::endMap()
  {
    --_p->_innerSerialization;
  }

  void BinaryEncoder::beginTuple(std::string sig)
  {
    if (!_p->_innerSerialization)
      signature() += "(" + sig + ")";
    ++_p->_innerSerialization;
  }

  void BinaryEncoder::endTuple()
  {
    --_p->_innerSerialization;
  }

  BinaryEncoder::Status BinaryEncoder::status() const
  {
    return _p->_status;
  }

  void BinaryEncoder::setStatus(Status status)
  {
    _p->_status = status;
  }

  Buffer& BinaryEncoder::buffer()
  {
    return _p->_buffer;
  }

  std::string& BinaryEncoder::signature()
  {
    return _p->_signature;
  }

  BinaryEncoderPrivate::BinaryEncoderPrivate(qi::Buffer &buffer)
    : _status(BinaryEncoder::Status_Ok)
    , _buffer(buffer)
    , _innerSerialization(0)
  {
  }

  BinaryEncoderPrivate::~BinaryEncoderPrivate()
  {
  }

  namespace details {

    void serializeObject(qi::BinaryEncoder& out,
      ObjectPtr object,
      ObjectHost* context)
    {
      if (!context)
        throw std::runtime_error("Unable to serialize object without a valid ObajectHost");
      unsigned int sid = context->service();
      unsigned int oid = context->nextId();
      ServiceBoundObject* sbo = new ServiceBoundObject(sid, oid, object, MetaCallType_Queued, true, context);
      boost::shared_ptr<BoundObject> bo(sbo);
      context->addObject(bo, oid);
      qiLogDebug() << "Hooking " << oid <<" on " << context;
      qiLogDebug() << "sbo " << sbo << "obj " << object.get();
      // Transmit the metaObject augmented by ServiceBoundObject.
      out.write(sbo->metaObject(oid));
      out.write((unsigned int)sid);
      out.write((unsigned int)oid);
    }

    void onProxyLost(GenericObject* ptr)
    {
      qiLogDebug() << "Proxy on argument object lost, invoking terminate...";
      DynamicObject* dobj = reinterpret_cast<DynamicObject*>(ptr->value);
      // dobj is a RemoteObject
      //FIXME: use post()
      ptr->call<void>("terminate", static_cast<RemoteObject*>(dobj)->service()).async();
    }

    GenericValuePtr deserializeObject(qi::BinaryDecoder& in,
      TransportSocketPtr context)
    {
      if (!context)
        throw std::runtime_error("Unable to deserialize object without a valid TransportSocket");
      MetaObject mo;
      in.read(mo);
      int sid, oid;
      in.read(sid);
      in.read(oid);
      qiLogDebug() << "Creating unregistered object " << sid << '/' << oid << " on " << context.get();
      RemoteObject* ro = new RemoteObject(sid, oid, mo, context);
      ObjectPtr o = makeDynamicObjectPtr(ro, true, &onProxyLost);
      qiLogDebug() << "New object is " << o.get() << "on ro " << ro;
      assert(o);
      assert(GenericValueRef(o).as<ObjectPtr>());
      return GenericValueRef(o).clone();
    }

    class SerializeTypeVisitor
    {
    public:
      SerializeTypeVisitor(BinaryEncoder& out, ObjectHost* context, GenericValuePtr value)
        : out(out)
        , context(context)
        , value(value)
      {}
      void visitUnknown(GenericValuePtr value)
      {
        qiLogError() << "Type " << value.type->infoString() <<" not serializable";
      }

      void visitVoid()
      {
        // Not an error, makes sense if encapsulated in a Dynamic for instance
      }

      void visitInt(int64_t value, bool isSigned, int byteSize)
      {
        switch((isSigned ? 1 : -1) * byteSize)
        {
          case 0:  out.write((bool)value);    break;
          case 1:  out.write((int8_t)value);  break;
          case -1: out.write((uint8_t)value); break;
          case 2:  out.write((int16_t)value); break;
          case -2: out.write((uint16_t)value);break;
          case 4:  out.write((int32_t)value); break;
          case -4: out.write((uint32_t)value);break;
          case 8:  out.write((int64_t)value); break;
          case -8: out.write((uint64_t)value);break;
          default:
            qiLogError() << "Unknown integer type " << isSigned << " " << byteSize;
        }
      }

      void visitFloat(double value, int byteSize)
      {
        if (byteSize == 4)
          out.write((float)value);
        else if (byteSize == 8)
          out.write((double)value);
        else
          qiLogError() << "serialize on unknown float type " << byteSize;
      }

      void visitString(char* data, size_t len)
      {
        out.writeString(data, len);
      }

      void visitList(GenericIterator it, GenericIterator end)
      {
        out.beginList(value.size(), static_cast<TypeList*>(value.type)->elementType()->signature());
        for (; it != end; ++it)
          qi::details::serialize(*it, out, context);
        out.endList();
      }

      void visitMap(GenericIterator it, GenericIterator end)
      {
        TypeMap* type = static_cast<TypeMap*>(value.type);
        out.beginMap(value.size(), type->keyType()->signature(), type->elementType()->signature());
        for(; it != end; ++it)
        {
          GenericValuePtr v = *it;
          qi::details::serialize(v[0], out, context);
          qi::details::serialize(v[1], out, context);
        }
        out.endMap();
      }

      void visitObject(GenericObject value)
      {
        // No refcount, user called us with some kind of Object, not ObjectPtr
        qiLogWarning() << "Serializing an object without a shared pointer";
        serializeObject(out, ObjectPtr(new GenericObject(value)), context);
      }

      void visitPointer(GenericValuePtr pointee)
      {
        TypePointer* type = static_cast<TypePointer*>(value.type);
        if (type->pointerKind() == TypePointer::Shared
          && pointee.kind() == Type::Object)
        {
          out.write("o");
          GenericValuePtr shared_ptr = value.clone();
          serializeObject(out,
            ObjectPtr(new GenericObject(static_cast<ObjectType*>(pointee.type), pointee.value),
              boost::bind(&GenericValuePtr::destroy, shared_ptr)),
            context);
        }
        else
          qiLogError() << "Pointer serialization not implemented";
      }

      void visitTuple(const std::vector<GenericValuePtr>& vals)
      {
        std::string tsig;
        for (unsigned i=0; i<vals.size(); ++i)
          tsig += vals[i].type->signature();
        out.beginTuple(tsig);
        for (unsigned i=0; i<vals.size(); ++i)
          qi::details::serialize(vals[i], out, context);
        out.endTuple();
      }

      void visitDynamic(GenericValuePtr pointee)
      {
        if (value.type->info() == typeOf<ObjectPtr>()->info())
        {
          out.write("o");
          ObjectPtr* obj = value.ptr<ObjectPtr>();
          serializeObject(out, *obj, context);
        }
        else
        out.write(pointee,
          boost::bind(&typeDispatch<SerializeTypeVisitor>,
            SerializeTypeVisitor(out, context, pointee),
            pointee));
      }

      void visitRaw(GenericValuePtr raw)
      {
        out.write(raw.as<Buffer>());
      }
      void visitIterator(GenericValuePtr)
      {
        qiLogError() << "Type " << value.type->infoString() <<" not serializable";
      }
      BinaryEncoder& out;
      ObjectHost* context;
      GenericValuePtr value;
    };

    class DeserializeTypeVisitor
    {
      /* Value passed to visitor functions should not be used, only type
    * information should.
    */
    public:
      DeserializeTypeVisitor(BinaryDecoder& in, TransportSocketPtr context)
        : in(in)
        , context(context)
      {}

      template<typename T>
      void deserialize()
      {
        in.read(result.as<T>());
      }

      void visitUnknown(GenericValuePtr)
      {
        qiLogError() << "Type " << result.type->infoString() <<" not deserializable";
      }

      void visitVoid()
      {
        result.type = typeOf<void>();
        result.value = 0;
      }

      void visitInt(int64_t value, bool isSigned, int byteSize)
      {
        switch((isSigned?1:-1)*byteSize)
        {
          case 0:  deserialize<bool>();  break;
          case 1:  deserialize<int8_t>();  break;
          case -1: deserialize<uint8_t>(); break;
          case 2:  deserialize<int16_t>(); break;
          case -2: deserialize<uint16_t>();break;
          case 4:  deserialize<int32_t>(); break;
          case -4: deserialize<uint32_t>();break;
          case 8:  deserialize<int64_t>(); break;
          case -8: deserialize<uint64_t>();break;
          default:
            qiLogError() << "Unknown integer type " << isSigned << " " << byteSize;
        }
      }

      void visitFloat(double value, int byteSize)
      {
        if (byteSize == 4)
          deserialize<float>();
        else if (byteSize == 8)
          deserialize<double>();
        else
          qiLogError() << "Unknown float type " << byteSize;
      }

      void visitString(char*, size_t)
      {
        std::string s;
        in.read(s);
        std::swap(s, result.as<std::string>());
      }

      void visitList(GenericIterator, GenericIterator)
      {
        Type* elementType = static_cast<TypeList*>(result.type)->elementType();
        qi::uint32_t sz = 0;
        in.read(sz);
        if (in.status() != BinaryDecoder::Status_Ok)
          return;
        for (unsigned i = 0; i < sz; ++i)
        {
          GenericValuePtr v = qi::details::deserialize(elementType, in, context);
          result._append(v);
          v.destroy();
        }
      }

      void visitMap(GenericIterator, GenericIterator)
      {
        Type* keyType = static_cast<TypeMap*>(result.type)->keyType();
        Type* elementType = static_cast<TypeMap*>(result.type)->elementType();
        qi::uint32_t sz = 0;
        in.read(sz);
        if (in.status() != BinaryDecoder::Status_Ok)
          return;
        for (unsigned i = 0; i < sz; ++i)
        {
          GenericValuePtr k = qi::details::deserialize(keyType, in, context);
          GenericValuePtr v = qi::details::deserialize(elementType, in, context);
          result._insert(k, v);
          k.destroy();
          v.destroy();
        }
      }

      void visitObject(GenericObject value)
      {
        result = deserializeObject(in, context);
      }

      void visitPointer(GenericValuePtr pointee)
      {
        qiLogError() << " Pointer serialization not implemented";
      }

      void visitTuple(const std::vector<GenericValuePtr>&)
      {
        TypeTuple* type = static_cast<TypeTuple*>(result.type);
        std::vector<Type*> types = type->memberTypes();
        for (unsigned i = 0; i<types.size(); ++i)
        {
          GenericValuePtr val = qi::details::deserialize(types[i], in, context);
          type->set(&result.value, i, val.value);
          val.destroy();
        }
      }

      void visitDynamic(GenericValuePtr pointee)
      {
        if (result.type->info() == typeOf<ObjectPtr>()->info())
        { // Advertise as dynamic, but type was already parsed
          visitObject(GenericObject(0, 0));
        }
        else
        {
          std::string sig;
          in.read(sig);
          DeserializeTypeVisitor dtv(*this);
          dtv.result = GenericValuePtr(Type::fromSignature(sig));;
          typeDispatch<DeserializeTypeVisitor>(dtv, dtv.result);
          static_cast<TypeDynamic*>(result.type)->set(&result.value, dtv.result);
          dtv.result.destroy();
        }
      }
      void visitIterator(GenericValuePtr)
      {
        qiLogError() << "Type " << result.type->infoString() <<" not deserializable";
      }

      void visitRaw(GenericValuePtr)
      {
        Buffer b;
        in.read(b);
        static_cast<TypeRaw*>(result.type)->set(&result.value, b);
      }
      GenericValuePtr result;
      BinaryDecoder& in;
      TransportSocketPtr context;
    }; //class

  } // namespace details

  namespace details {
    void serialize(GenericValuePtr val, BinaryEncoder& out, ObjectHost* context)
    {
      SerializeTypeVisitor stv(out, context, val);
      qi::typeDispatch(stv, val);
      if (out.status() != BinaryEncoder::Status_Ok) {
        qiLogError() << "OSerialization error " << out.status();
      }
    }

    GenericValuePtr deserialize(qi::Type *type, BinaryDecoder& in, TransportSocketPtr context) {
      DeserializeTypeVisitor dtv(in, context);
      dtv.result = GenericValuePtr(type);

      qi::typeDispatch(dtv, dtv.result);
      if (in.status() != BinaryDecoder::Status_Ok) {
        qiLogError() << "ISerialization error " << in.status();
      }
      return dtv.result;
    }



  } // namespace details
}

