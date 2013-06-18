/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/binarycodec.hpp>
#include <qitype/genericvalue.hpp>

#include "binaryencoder.hpp"
#include "binarydecoder.hpp"

#include <qi/log.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/typedispatcher.hpp>
#include <qi/types.hpp>
#include <vector>
#include <cstring>

qiLogCategory("qitype.binarycoder");

namespace qi {
  namespace details
  {
    void serialize(GenericValuePtr val, BinaryEncoder& out, SerializeObjectCallback context=SerializeObjectCallback());
    void deserialize(GenericValuePtr what, BinaryDecoder& in, DeserializeObjectCallback context=DeserializeObjectCallback());
    GenericValuePtr deserialize(qi::Type *type, BinaryDecoder& in, DeserializeObjectCallback context=DeserializeObjectCallback());
  }
  class BinaryDecoder;
  class BinaryEncoder;

  class BinaryDecoderPrivate {
    public:
      BinaryDecoderPrivate(qi::BufferReader* buffer);
      ~BinaryDecoderPrivate();

      BinaryDecoder::Status _status;
      BufferReader *_reader;
  };

  class BinaryEncoderPrivate {
    public:
      BinaryEncoderPrivate(qi::Buffer& buffer);
      ~BinaryEncoderPrivate();

      BinaryEncoder::Status _status;
      Buffer _buffer;
      std::string _signature;
      unsigned int _innerSerialization;
  };

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
  QI_SIMPLE_SERIALIZER_IMPL(long, long, 'l')
  QI_SIMPLE_SERIALIZER_IMPL(unsigned long, unsigned long, 'L')
  QI_SIMPLE_SERIALIZER_IMPL(long long, long long, 'l')
  QI_SIMPLE_SERIALIZER_IMPL(unsigned long long, unsigned long long, 'L')
  QI_SIMPLE_SERIALIZER_IMPL(float, float, 'f')
  QI_SIMPLE_SERIALIZER_IMPL(double, double, 'd')

#undef QI_SIMPLE_SERIALIZER_IMPL

  BinaryDecoder::BinaryDecoder(BufferReader *buffer)
    : _p(new BinaryDecoderPrivate(buffer))
  {
  }

  BinaryDecoder::~BinaryDecoder()
  {
    delete _p;
  }

  size_t BinaryDecoder::readRaw(void *data, size_t size)
  {
    return _p->_reader->read(data, size);
  }

  void* BinaryDecoder::readRaw(size_t size)
  {
    return _p->_reader->read(size);
  }

  BinaryDecoder::Status BinaryDecoder::status() const
  {
    return _p->_status;
  }

  void BinaryDecoder::setStatus(Status status)
  {
    _p->_status = status;
  }

  const char* BinaryDecoder::statusToStr(Status status)
  {
    const char* StatusStr[] = {
      "Status OK",
      "Status Read Error",
      "Status Read Past End"
    };
    return StatusStr[status];
  }

  BufferReader& BinaryDecoder::bufferReader()
  {
    return *_p->_reader;
  }

  BinaryDecoderPrivate::BinaryDecoderPrivate(qi::BufferReader *buffer)
    : _status(BinaryDecoder::Status_Ok), _reader(buffer)
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

  void BinaryEncoder::writeRaw(const qi::Buffer &meta) {
    if (!_p->_innerSerialization)
    {
      signature() += "r";
    }

    buffer().addSubBuffer(meta);

    // qiLogDebug() << "Serializing buffer " << meta.size()
    //                         << " at " << buffer().size();
  }

  void BinaryEncoder::writeValue(const GenericValuePtr &value, boost::function<void()> recurse)
  {
    qi::Signature sig = value.signature();
    beginDynamic(sig);

    if (sig.isValid()) {
      assert(value.type);
      if (!recurse)
        details::serialize(value, *this);
      else
        recurse();
    } else {
      assert(!value.type);
    }
    endDynamic();
  }

  void BinaryEncoder::beginDynamic(const qi::Signature &elementSignature)
  {
    if (!_p->_innerSerialization)
      signature() += "m";
    ++_p->_innerSerialization;
    write(elementSignature);
  }

  void BinaryEncoder::endDynamic()
  {
    --_p->_innerSerialization;
  }

  void BinaryEncoder::beginList(uint32_t size, const qi::Signature &elementSignature)
  {
    if (!_p->_innerSerialization)
      signature() += "[" + elementSignature.toString();
    ++_p->_innerSerialization;
    write(size);
  }

  void BinaryEncoder::endList()
  {
    --_p->_innerSerialization;
    if (!_p->_innerSerialization)
      signature() += "]";
  }

  void BinaryEncoder::beginMap(uint32_t size, const qi::Signature &keySignature, const qi::Signature &valueSignature)
  {
    if (!_p->_innerSerialization)
      signature() += "{" + keySignature.toString() + valueSignature.toString() + "}";
    ++_p->_innerSerialization;
    write(size);
  }

  void BinaryEncoder::endMap()
  {
    --_p->_innerSerialization;
  }

  void BinaryEncoder::beginTuple(const Signature &signatu)
  {
    if (!_p->_innerSerialization)
      _p->_signature += signatu.toString();
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

  const char* BinaryEncoder::statusToStr(Status status)
  {
    const char* StatusStr[] = {
      "Status OK",
      "Status Write Error"
    };
    return StatusStr[status];
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

    class SerializeTypeVisitor
    {
    public:
      SerializeTypeVisitor(BinaryEncoder& out, SerializeObjectCallback context, GenericValuePtr value)
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
          case 0:  out.write((bool)!!value);    break;
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
          serialize(*it, out, context);
        out.endList();
      }

      void visitMap(GenericIterator it, GenericIterator end)
      {
        MapTypeInterface* type = static_cast<MapTypeInterface*>(value.type);
        out.beginMap(value.size(), type->keyType()->signature(), type->elementType()->signature());
        for(; it != end; ++it)
        {
          GenericValuePtr v = *it;
          serialize(v[0], out, context);
          serialize(v[1], out, context);
        }
        out.endMap();
      }

      void visitObject(GenericObject value)
      {
        // No refcount, user called us with some kind of Object, not ObjectPtr
        ObjectPtr o(new GenericObject(value));
        visitObjectPtr(o);
      }

      void visitPointer(GenericValuePtr pointee)
      {
        qiLogError() << "Pointer serialization not implemented";
      }

      void visitObjectPtr(ObjectPtr& ptr)
      {
        if (!context)
          throw std::runtime_error("Object serialization callback required but not provided");
        ObjectSerializationInfo osi = context(ptr);
        out.write(osi.metaObject);
        out.write(osi.serviceId);
        out.write(osi.objectId);
      }

      void visitTuple(const std::string &name, const std::vector<GenericValuePtr>& vals, const std::vector<std::string>& annotations)
      {
        out.beginTuple(qi::makeTupleSignature(vals));
        for (unsigned i=0; i<vals.size(); ++i)
          serialize(vals[i], out, context);
        out.endTuple();
      }

      void visitDynamic(GenericValuePtr pointee)
      {
        //Remaining types
        out.writeValue(pointee, boost::bind(&typeDispatch<SerializeTypeVisitor>,
                                            SerializeTypeVisitor(out, context, pointee), pointee));
      }

      void visitRaw(GenericValuePtr raw)
      {
        out.writeRaw(raw.to<Buffer>());
      }

      void visitIterator(GenericValuePtr)
      {
        qiLogError() << "Type " << value.type->infoString() <<" not serializable";
      }

      BinaryEncoder& out;
      SerializeObjectCallback context;
      GenericValuePtr value;
    };

    class DeserializeTypeVisitor
    {
      /*
      * result *must* be modified in place, not changed
      */
    public:
      DeserializeTypeVisitor(BinaryDecoder& in, DeserializeObjectCallback context)
        : in(in)
        , context(context)
      {}

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
        case 0: {
          bool b; in.read(b); result.setInt(b);
        } break;
        case 1: {
          int8_t b; in.read(b); result.setInt(b);
        } break;
        case -1: {
          uint8_t b; in.read(b); result.setUInt(b);
        } break;
        case 2: {
          int16_t b; in.read(b); result.setInt(b);
        } break;
        case -2: {
          uint16_t b; in.read(b); result.setUInt(b);
        } break;
        case 4: {
          int32_t b; in.read(b); result.setInt(b);
        } break;
        case -4: {
          uint32_t b; in.read(b); result.setUInt(b);
        } break;
        case 8: {
          int64_t b; in.read(b); result.setInt(b);
        } break;
        case -8: {
          uint64_t b; in.read(b); result.setUInt(b);
        } break;
        default:
          qiLogError() << "Unknown integer type " << isSigned << " " << byteSize;
        }
      }

      void visitFloat(double value, int byteSize)
      {
        if (byteSize == 4) {
          float t;
          in.read(t);
          result.setFloat(t);
        } else if (byteSize == 8) {
          double t;
          in.read(t);
          result.setDouble(t);
        }
        else
          qiLogError() << "Unknown float type " << byteSize;
      }

      void visitString(char*, size_t)
      {
        std::string s;
        in.read(s);

        //optimise when result is of type std::string
        static Type* tstring = qi::typeOf<std::string>();
        if ((result.type == tstring) || (result.type->info() == tstring->info())) {
          std::swap(s, result.as<std::string>());
          return;
        }
        //result is compatible with string
        result.setString(s);
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
          GenericValuePtr v = deserialize(elementType, in, context);
          result._append(v);
          v.destroy();
        }
      }

      void visitMap(GenericIterator, GenericIterator)
      {
        Type* keyType = static_cast<MapTypeInterface*>(result.type)->keyType();
        Type* elementType = static_cast<MapTypeInterface*>(result.type)->elementType();
        qi::uint32_t sz = 0;
        in.read(sz);
        if (in.status() != BinaryDecoder::Status_Ok)
          return;
        for (unsigned i = 0; i < sz; ++i)
        {
          GenericValuePtr k = deserialize(keyType, in, context);
          GenericValuePtr v = deserialize(elementType, in, context);
          result._insert(k, v);
          k.destroy();
          v.destroy();
        }
      }
      void visitObjectPtr(ObjectPtr& o)
      {
        ObjectSerializationInfo osi;
        in.read(osi.metaObject);
        in.read(osi.serviceId);
        in.read(osi.objectId);
        if (context)
          o = context(osi);
        // else leave result default-initilaized
      }

      void visitObject(GenericObject value)
      {
        qiLogError() << "No signature deserializes to object";
      }

      void visitPointer(GenericValuePtr pointee)
      {
        qiLogError() << " Pointer serialization not implemented";
      }

      void visitTuple(const std::string &, const std::vector<GenericValuePtr>&, const std::vector<std::string>&)

      {
        StructTypeInterface* type = static_cast<StructTypeInterface*>(result.type);
        std::vector<Type*> types = type->memberTypes();
        // Be safe, do not assume deserialize will give us the type we asked.
        std::vector<void*> vals;
        std::vector<Type*> valstypes;
        vals.resize(types.size());
        valstypes.resize(types.size());
        for (unsigned i = 0; i<types.size(); ++i)
        {
          GenericValuePtr val = deserialize(types[i], in, context);
          if (!val.type)
            throw std::runtime_error("Deserialization of tuple field failed");
          vals[i] = val.value;
          valstypes[i] = val.type;
        }
        type->set(&result.value, vals);
        for (unsigned i = 0; i<types.size(); ++i)
        {
          valstypes[i]->destroy(vals[i]);
        }
      }

      void visitDynamic(GenericValuePtr pointee)
      {
        std::string sig;
        in.read(sig);
        //empty gv: nothing to do
        if (sig.empty()) {
          return;
        }
        Type* type = Type::fromSignature(qi::Signature(sig));
        if (!type)
        {
          qiLogError() << "Cannot find a type to deserialize signature " << sig << " within a dynamic value.";
          result.destroy();
          return;
        }

        DeserializeTypeVisitor dtv(*this);
        dtv.result = GenericValuePtr(type);
        typeDispatch<DeserializeTypeVisitor>(dtv, dtv.result);
        static_cast<DynamicTypeInterface*>(result.type)->set(&result.value, dtv.result);
        dtv.result.destroy();
      }
      void visitIterator(GenericValuePtr)
      {
        qiLogError() << "Type " << result.type->infoString() <<" not deserializable";
      }

      void visitRaw(GenericValuePtr)
      {
        Buffer b;
        in.read(b);
        static_cast<RawTypeInterface*>(result.type)->set(&result.value, (char*)b.data(), b.size());
      }
      GenericValuePtr result;
      BinaryDecoder& in;
      DeserializeObjectCallback context;
    }; //class

     void serialize(GenericValuePtr val, BinaryEncoder& out, SerializeObjectCallback context)
    {
      details::SerializeTypeVisitor stv(out, context, val);
      qi::typeDispatch(stv, val);
      if (out.status() != BinaryEncoder::Status_Ok) {
        qiLogError() << "OSerialization error " << BinaryEncoder::statusToStr(out.status());
      }
    }

    void deserialize(GenericValuePtr what, BinaryDecoder& in, DeserializeObjectCallback context)
    {
      details::DeserializeTypeVisitor dtv(in, context);
      dtv.result = what;
      qi::typeDispatch(dtv, dtv.result);
      if (in.status() != BinaryDecoder::Status_Ok) {
        qiLogError() << "ISerialization error " << BinaryDecoder::statusToStr(in.status());
      }
      what = dtv.result;
    }

    GenericValuePtr deserialize(qi::Type *type, BinaryDecoder& in, DeserializeObjectCallback context)
    {
      GenericValuePtr res(type);
      deserialize(res, in, context);
      return res;
    }

  } // namespace details

  void encodeBinary(qi::Buffer *buf, const qi::AutoGenericValuePtr &gvp, SerializeObjectCallback onObject) {
    BinaryEncoder be(*buf);
    details::SerializeTypeVisitor stv(be, onObject, gvp);
    qi::typeDispatch(stv, gvp);
    if (be.status() != BinaryEncoder::Status_Ok) {
      std::stringstream ss;
      ss << "OSerialization error " << be.status();
      qiLogError() << ss.str();
      throw std::runtime_error(ss.str());
    }
  }

  void decodeBinary(qi::BufferReader *buf, qi::GenericValuePtr gvp,
    DeserializeObjectCallback onObject) {
    BinaryDecoder in(buf);
    details::DeserializeTypeVisitor dtv(in, onObject);
    dtv.result = gvp;
    qi::typeDispatch(dtv, dtv.result);
    if (in.status() != BinaryDecoder::Status_Ok) {
      std::stringstream ss;
      ss << "ISerialization error " << in.status();
      qiLogError() << ss.str();
      throw std::runtime_error(ss.str());
    }
  }

}

