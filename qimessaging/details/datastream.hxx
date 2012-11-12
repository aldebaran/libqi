#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef   	TYPESER_HXX_
# define   	TYPESER_HXX_

#include <qitype/typedispatcher.hpp>
#include <qitype/genericobject.hpp>

namespace qi {

  namespace details {

    QIMESSAGING_API void         serialize(GenericValuePtr val, ODataStream& out);
    QIMESSAGING_API GenericValuePtr deserialize(qi::Type *type, IDataStream& in);

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





  template<typename T>
  ODataStream& operator<<(ODataStream& out, const T &v) {
    Type* type = typeOf<T>();
    GenericValuePtr value = GenericValuePtr::from(v);
    details::SerializeTypeVisitor stv(out);
    typeDispatch(stv, type, &value.value);
    return out;
  }

  template<typename T>
  IDataStream& operator>>(IDataStream& in, T& v)
  {
    Type* type = typeOf<T>();
    GenericValuePtr value = qi::details::deserialize(type, in);;
    T* ptr = (T*)type->ptrFromStorage(&value.value);
    v = *ptr;
    return in;
  }

}


#endif	    /* !TYPESER_HXX_ */
