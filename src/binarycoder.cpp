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
        qiLogError("qimessaging.binarycoder", "buffer empty");
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
      qiLogDebug("BinaryCoder") << "Extracting buffer of size " << sz <<" at " << reader.position();
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
      qiLogError("qimessaging.binarycoder") << "Could not find metatype for signature " << signature;
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

    // qiLogDebug("BinaryCoder") << "Serializing buffer " << meta.size()
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
      qiLogWarning("qi.BinaryEncoder") << "Weird GenericValuePtr signature: " << sig;
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

    static void delete_sbo(ServiceBoundObject* sbo)
    {
      qiLogDebug("qi.type") << "Socket lost, destroying unregistered boundobject " << sbo;
      sbo->terminate(0); // be async
    }

    void serializeObject(qi::BinaryEncoder& out,
      ObjectPtr object,
      ObjectHost* context)
    {
      if (!context)
        throw std::runtime_error("Unable to serialize object without a valid ObajectHost");
      unsigned int sid = context->service();
      unsigned int oid = context->nextId();
      ServiceBoundObject* sbo = new ServiceBoundObject(sid, oid, object, MetaCallType_Auto, true);
      context->addObject(sbo, oid);
      qiLogDebug("qi.type") << "Hooking " << oid <<" on " << context;
      qiLogDebug("qi.type") << "sbo " << sbo << "obj " << object.get();
      // Transmit the metaObject augmented by ServiceBoundObject.
      out.write(sbo->metaObject(oid));
      out.write((unsigned int)sid);
      out.write((unsigned int)oid);
      // Disconnect from context on ServiceBoundObject destruction
      sbo->onDestroy.connect(boost::bind(&ObjectHost::removeObject, context, oid));
      // Delete sbo on context destruction
      qi::SignalBase::Link dSub = context->onDestroy.connect(boost::bind(delete_sbo, sbo));
      // unlink deletor-on-objecthost-delete on sbo deletion to prevent double-delete
      sbo->onDestroy.connect(boost::bind(&qi::Signal<void()>::disconnect, boost::ref(context->onDestroy), dSub));
    }

    void onProxyLost(GenericObject* ptr)
    {
      qiLogDebug("qi.type") << "Proxy on argument object last, invoking terminate...";
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
      qiLogDebug("qi.type") << "Creating unregistered object " << sid << '/' << oid << " on " << context.get();
      RemoteObject* ro = new RemoteObject(sid, oid, mo, context);
      ObjectPtr o = makeDynamicObjectPtr(ro, true, &onProxyLost);
      qiLogDebug("qi.type") << "New object is " << o.get() << "on ro " << ro;
      assert(o);
      assert(GenericValuePtr::from(o).as<ObjectPtr>());
      return GenericValuePtr::from(o).clone();
    }

    class SerializeTypeVisitor
    {
    public:
      SerializeTypeVisitor(BinaryEncoder& out, ObjectHost* context)
        : out(out)
        , context(context)
      {}
      void visitUnknown(Type* type, void* storage)
      {
        qiLogError("qi.type") << "Type " << type->infoString() <<" not serializable";
      }

      void visitVoid(Type*)
      {
        // Not an error, makes sense if encapsulated in a Dynamic for instance
      }

      void visitInt(TypeInt* type, int64_t value, bool isSigned, int byteSize)
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
            qiLogError("qi.type") << "Unknown integer type " << isSigned << " " << byteSize;
        }
      }

      void visitFloat(TypeFloat* type, double value, int byteSize)
      {
        if (byteSize == 4)
          out.write((float)value);
        else if (byteSize == 8)
          out.write((double)value);
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
        // No refcount, user called us with some kind of Object, not ObjectPtr
        qiLogWarning("qi.type") << "Serializing an object without a shared pointer";
        serializeObject(out, ObjectPtr(new GenericObject(value)), context);
      }

      void visitPointer(TypePointer* type, void* storage, GenericValuePtr pointee)
      {
        if (type->pointerKind() == TypePointer::Shared
          && pointee.kind() == Type::Object)
        {
          out.write("o");
          GenericValuePtr shared_ptr = GenericValuePtr(type, storage).clone();
          serializeObject(out,
            ObjectPtr(new GenericObject(static_cast<ObjectType*>(pointee.type), pointee.value),
              boost::bind(&GenericValuePtr::destroy, shared_ptr)),
            context);
        }
        else
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

      void visitDynamic(GenericValuePtr source, GenericValuePtr pointee)
      {
        if (source.type->info() == typeOf<ObjectPtr>()->info())
        {
          out.write("o");
          ObjectPtr* obj = source.ptr<ObjectPtr>();
          serializeObject(out, *obj, context);
        }
        else
        out.write(pointee,
          boost::bind(&typeDispatch<SerializeTypeVisitor>, boost::ref(*this),
            pointee.type, &pointee.value));
      }

      void visitRaw(TypeRaw* type, Buffer* buffer)
      {
        out.write(*buffer);
      }
      BinaryEncoder& out;
      ObjectHost* context;
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

      template<typename T, typename TYPE>
      void deserialize(TYPE* type)
      {
        T val;
        in.read(val);
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
          case 0:  deserialize<bool>(type);  break;
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
        in.read(s);
        type->set(&result.value, s);
      }

      void visitList(GenericListPtr value)
      {
        result.type = value.type;
        result.value = value.type->initializeStorage();
        GenericListPtr res(result);
        Type* elementType = res.elementType();
        qi::uint32_t sz = 0;
        in.read(sz);
        if (in.status() != BinaryDecoder::Status_Ok)
          return;
        for (unsigned i = 0; i < sz; ++i)
        {
          GenericValuePtr v = qi::details::deserialize(elementType, in);
          res.pushBack(v);
          v.destroy();
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
        in.read(sz);
        if (in.status() != BinaryDecoder::Status_Ok)
          return;
        for (unsigned i = 0; i < sz; ++i)
        {
          GenericValuePtr k = qi::details::deserialize(keyType, in);
          GenericValuePtr v = qi::details::deserialize(elementType, in);
          res.insert(k, v);
          k.destroy();
          v.destroy();
        }
      }

      void visitObject(GenericObject value)
      {
        result = deserializeObject(in, context);
      }

      void visitPointer(TypePointer* type, void* storage, GenericValuePtr pointee)
      {
        qiLogError("qi.type") << " Pointer serialization not implemented";
      }

      void visitTuple(TypeTuple* type, void* storage)
      {
        std::vector<Type*> types = type->memberTypes();
        result.type = type;
        result.value = type->initializeStorage();
        for (unsigned i = 0; i<types.size(); ++i)
        {
          GenericValuePtr val = qi::details::deserialize(types[i], in);
          type->set(&result.value, i, val.value);
          val.destroy();
        }
      }

      void visitDynamic(GenericValuePtr source, GenericValuePtr pointee)
      {
        if (source.type->info() == typeOf<ObjectPtr>()->info())
        { // Advertise as dynamic, but type was already parsed
          visitObject(GenericObject(0, 0));
        }
        else
        {
          std::string sig;
          in.read(sig);
          GenericValuePtr val;
          val.type = Type::fromSignature(sig);
          DeserializeTypeVisitor dtv(*this);
          typeDispatch<DeserializeTypeVisitor>(dtv, val.type, &val.value);
          val = dtv.result;
          result.type = source.type;
          result.value = result.type->initializeStorage();
          static_cast<TypeDynamic*>(source.type)->set(&result.value, val);
          val.destroy();
        }
      }

      void visitRaw(TypeRaw* type, Buffer*)
      {
        Buffer b;
        in.read(b);
        void* s = type->initializeStorage();
        type->set(&s, b);
        result.type = type;
        result.value = s;
      }
      GenericValuePtr result;
      BinaryDecoder& in;
      TransportSocketPtr context;
    }; //class

  } // namespace details

  namespace details {
    void serialize(GenericValuePtr val, BinaryEncoder& out, ObjectHost* context)
    {
      SerializeTypeVisitor stv(out, context);
      qi::typeDispatch(stv, val.type, &val.value);
      if (out.status() != BinaryEncoder::Status_Ok) {
        qiLogError("qimessaging.binarycoder") << "OSerialization error " << out.status();
      }
    }

    GenericValuePtr deserialize(qi::Type *type, BinaryDecoder& in, TransportSocketPtr context) {
      void* storage = 0;
      DeserializeTypeVisitor dtv(in, context);
      qi::typeDispatch(dtv, type, &storage);
      if (in.status() != BinaryDecoder::Status_Ok) {
        qiLogError("qimessaging.binarycoder") << "ISerialization error " << in.status();
      }
      return dtv.result;
    }



  } // namespace details
}

