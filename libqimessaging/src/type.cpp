/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qimessaging/type.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/genericvalue.hpp>
#include <qimessaging/genericvaluespecialized.hpp>
#include <qimessaging/genericobject.hpp>

namespace qi {




  typedef std::map<detail::TypeInfoKey, Type*> TypeFactory;
  static TypeFactory& typeFactory()
  {
    static TypeFactory* res = 0;
    if (!res)
      res = new TypeFactory;
    return *res;
  }

  QIMESSAGING_API Type* getType(const std::type_info& type)
  {
    // We create-if-not-exist on purpose: to detect access that occur before
    // registration
    return typeFactory()[detail::TypeInfoKey(type)];
  }

  /// Type factory setter
  QIMESSAGING_API bool registerType(const std::type_info& typeId, Type* type)
  {
    qiLogDebug("qi.meta") << "registerType "  << typeId.name() << " "
     << type->kind() <<" " << (void*)type;
    TypeFactory::iterator i = typeFactory().find(detail::TypeInfoKey(typeId));
    if (i != typeFactory().end())
    {
      if (i->second)
        qiLogWarning("qi.meta") << "registerType: previous registration present for "
          << typeId.name()<< " " << (void*)i->second << " " << i->second->kind();
      else
        qiLogWarning("qi.meta") << "registerType: access to type factory before"
          " registration detected for type " << typeId.name();
    }
    typeFactory()[detail::TypeInfoKey(typeId)] = type;
    return true;
  }

  class TypeSignature
  {
  public:
    void visitVoid(Type*)
    {
      result = Signature::fromType(Signature::Type_Void).toString();
    }
    void visitInt(TypeInt* type, int64_t, bool isSigned, int byteSize)
    {
      switch((isSigned?1:-1)*byteSize)
      {
        case 1:  result = Signature::fromType(Signature::Type_Int8).toString();  break;
        case -1: result = Signature::fromType(Signature::Type_UInt8).toString(); break;
        case 2:  result = Signature::fromType(Signature::Type_Int16).toString(); break;
        case -2: result = Signature::fromType(Signature::Type_UInt16).toString();break;
        case 4:  result = Signature::fromType(Signature::Type_Int32).toString(); break;
        case -4: result = Signature::fromType(Signature::Type_UInt32).toString();break;
        case 8:  result = Signature::fromType(Signature::Type_Int64).toString(); break;
        case -8: result = Signature::fromType(Signature::Type_UInt64).toString();break;
        default:
          result =  Signature::fromType(Signature::Type_Unknown).toString();
      }
    }
   void visitFloat(TypeFloat* type, double, int byteSize)
   {
     if (byteSize == 4)
       result = Signature::fromType(Signature::Type_Float).toString();
     else
       result = Signature::fromType(Signature::Type_Double).toString();
   }
   void visitString(TypeString* type, void* storage)
   {
     result = Signature::fromType(Signature::Type_String).toString();
   }
   void visitList(GenericList value)
   {
     result = std::string()
        + (char)Signature::Type_List
        + value.elementType()->signature()
        + (char)Signature::Type_List_End;
   }
   void visitMap(GenericMap value)
   {
     result = std::string()
        + (char)Signature::Type_Map
        + value.keyType()->signature()
        + value.elementType()->signature()
        + (char)Signature::Type_Map_End;
   }
   void visitObject(GenericObject )
   {
     result = Signature::fromType(Signature::Type_Unknown).toString();
   }
   void visitPointer(TypePointer* type, void* , GenericValue )
   {
     result = Signature::fromType(Signature::Type_Unknown).toString();
   }
   void visitUnknown(Type* type, void*)
   {
     result = Signature::fromType(Signature::Type_Unknown).toString();
   }
   void visitTuple(TypeTuple* type, void* storage)
   {
     std::vector<Type*> types = type->memberTypes(storage);
     std::string res;
     res += (char)Signature::Type_Tuple;
     for (unsigned i=0; i<types.size(); ++i)
       res += types[i]->signature();
     res += (char)Signature::Type_Tuple_End;
     result = res;
   }
   void visitDynamic(Type* type, GenericValue pointee)
   {
     result = Signature::fromType(Signature::Type_Dynamic).toString();
   }
   std::string result;
  };

  class TypeSerialize
  {
  public:
    TypeSerialize(ODataStream& out)
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
        case 1:  out <<(int8_t)value;  break;
        case -1: out <<(uint8_t)value; break;
        case 2:  out <<(int16_t)value; break;
        case -2: out <<(uint16_t)value;break;
        case 4:  out <<(int32_t)value; break;
        case -4: out <<(uint32_t)value;break;
        case 8:  out <<(int64_t)value; break;
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
      Buffer* buf = type->asBuffer(storage);
      if (buf)
        out << *buf;
      else
        out << type->getString(storage);
    }
    void visitList(GenericList value)
    {
      out.beginList(value.size(), value.elementType()->signature());
      GenericListIterator it, end;
      it = value.begin();
      end = value.end();
      for (; it != end; ++it)
        (*it).serialize(out);
      it.destroy();
      end.destroy();
      out.endList();
    }
    void visitMap(GenericMap value)
    {
      out.beginMap(value.size(), value.keyType()->signature(), value.elementType()->signature());
      GenericMapIterator it, end;
      it = value.begin();
      end = value.end();
      for(; it != end; ++it)
      {
        std::pair<GenericValue, GenericValue> v = *it;
        v.first.serialize(out);
        v.second.serialize(out);
      }
      it.destroy();
      end.destroy();
      out.endMap();
    }
    void visitObject(GenericObject value)
    {
      qiLogError("qi.type") << "Object serialization not implemented";
    }
    void visitPointer(TypePointer* type, void* storage, GenericValue pointee)
    {
       qiLogError("qi.type") << "Pointer serialization not implemented";
    }
    void visitTuple(TypeTuple* type, void* storage)
    {
      std::vector<GenericValue> vals = type->getValues(storage);
      std::string tsig;
      for (unsigned i=0; i<vals.size(); ++i)
        tsig += vals[i].type->signature();
      out.beginTuple(tsig);
      for (unsigned i=0; i<vals.size(); ++i)
        vals[i].serialize(out);
      out.endTuple();
    }
    void visitDynamic(Type* type, GenericValue pointee)
    {
      out << pointee.type->signature();
      pointee.type->serialize(out, pointee.value);
    }

    ODataStream& out;
  };

  class TypeDeserialize
  {
  public:
    TypeDeserialize(IDataStream& in)
    : in(in) {}
    template<typename T, typename TYPE> void get(TYPE* type)
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
      qiLogWarning("qi.type") << "Void cannot be deserialized.";
    }
    void visitInt(TypeInt* type, int64_t value, bool isSigned, int byteSize)
    {
      switch((isSigned?1:-1)*byteSize)
      {
      case 1:  get<int8_t>(type);  break;
      case -1: get<uint8_t>(type); break;
      case 2:  get<int16_t>(type); break;
      case -2: get<uint16_t>(type);break;
      case 4:  get<int32_t>(type); break;
      case -4: get<uint32_t>(type);break;
      case 8:  get<int64_t>(type); break;
      case -8: get<uint64_t>(type);break;
      default:
        qiLogError("qi.type") << "Unknown integer type " << isSigned << " " << byteSize;
      }
    }
    void visitFloat(TypeFloat* type, double value, int byteSize)
    {
      if (byteSize == 4)
        get<float>(type);
      else if (byteSize == 8)
        get<double>(type);
      else
        qiLogError("qi.type") << "Unknown float type " << byteSize;
    }
    void visitString(TypeString* type, const void* storage)
    {
      result.type = type;
      result.value = result.type->initializeStorage();
      Buffer* buf = type->asBuffer(result.value);
      if (buf)
        in >> *buf;
      else
      {
        std::string s;
        in >> s;
        type->set(&result.value, s);
      }
    }
    void visitList(GenericList value)
    {
      result.type = value.type;
      result.value = value.type->initializeStorage();
      GenericList res(result);
      Type* elementType = res.elementType();
      qi::uint32_t sz = 0;
      in >> sz;
      for (unsigned i=0; i<sz; ++i)
      {
        res.pushBack(elementType->deserialize(in));
      }
    }
    void visitMap(GenericMap value)
    {
      result.type = value.type;
      result.value = value.type->initializeStorage();
      GenericMap res(result);
      Type* keyType = res.keyType();
      Type* elementType = res.elementType();
      qi::uint32_t sz = 0;
      in >> sz;
      for (unsigned i=0; i<sz; ++i)
      {
        GenericValue k = keyType->deserialize(in);
        GenericValue v = elementType->deserialize(in);
        res.insert(k, v);
      }
    }
    void visitObject(GenericObject value)
    {
      qiLogError("qi.type") << " Object serialization not implemented";
    }
    void visitPointer(TypePointer* type, void* storage, GenericValue pointee)
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
        GenericValue val = types[i]->deserialize(in);
        type->set(&result.value, i, val.value);
        val.destroy();
      }
    }
    void visitDynamic(Type* type, GenericValue pointee)
    {
      std::string sig;
      in >> sig;
      result.type = Type::fromSignature(sig);
      result = result.type->deserialize(in);
    }
    GenericValue result;
    IDataStream& in;
  };

  std::string Type::signature()
  {
    void* storage = 0;
    return dispatch(TypeSignature(), &storage).result;
  }

  void Type::serialize(ODataStream& out, void* storage)
  {
    dispatch(TypeSerialize(out), &storage);
  }
  GenericValue Type::deserialize(IDataStream& in)
  {
    void* storage = 0;
    return dispatch(TypeDeserialize(in), &storage).result;
  }

  static Type* fromSignature(const qi::Signature::iterator & i)
  {
    switch(i.type())
    {
    case Signature::Type_None:
    case Signature::Type_Void:
      return typeOf<void>();
    case Signature::Type_Int8:
      return typeOf<int8_t>();
    case Signature::Type_UInt8:
      return typeOf<uint8_t>();
    case Signature::Type_Int16:
      return typeOf<int16_t>();
    case Signature::Type_UInt16:
      return typeOf<uint16_t>();
    case Signature::Type_Int32:
      return typeOf<int32_t>();
    case Signature::Type_UInt32:
      return typeOf<uint32_t>();
    case Signature::Type_Float:
      return typeOf<float>();
    case Signature::Type_Double:
      return typeOf<double>();
    case Signature::Type_String:
      return typeOf<std::string>();
    case Signature::Type_List:
      return defaultListType(fromSignature(i.children().begin()));
    case Signature::Type_Map:
      return defaultMapType(fromSignature(i.children().begin()),
        fromSignature(i.children().begin()++));
    case Signature::Type_Tuple:
      {
        std::vector<Type*> types;
        Signature c = i.children();
        for (Signature::iterator child = c.begin(); child != c.end(); child++)
          types.push_back(fromSignature(child));
        return defaultTupleType(types);
      }
    case Signature::Type_Dynamic:
      return typeOf<GenericValue>();
    default:
      qiLogWarning("qi.type") << "Cannot get type from signature " << i.signature();
      return 0;
    }
  }

  Type* Type::fromSignature(const qi::Signature& sig)
  {
    if (sig.size() != 1)
      qiLogWarning("qi.type") << "fromSignature(): signature has more than one element";
    Signature::iterator i = sig.begin();
    return ::qi::fromSignature(i);
  }

  // Default list

  class DefaultListIteratorType: public TypeListIteratorImpl<std::vector<void*> >
  {
  public:
    DefaultListIteratorType(Type* elementType)
    : _elementType(elementType)
    {}
    GenericValue dereference(void* storage)
    {
      std::vector<void*>::iterator& ptr = *(std::vector<void*>::iterator*)
        ptrFromStorage(&storage);
      GenericValue res;
      res.type = _elementType;
      res.value = *ptr;
      return res;
    }
    Type* _elementType;
  };

  // We want exactly one instance per element type
  static Type* defaultListIteratorType(Type* element)
  {
    static std::map<detail::TypeInfoKey, Type*>* map = 0;
    if (!map)
      map = new std::map<detail::TypeInfoKey, Type*>();
    detail::TypeInfoKey key(element->info());
    std::map<detail::TypeInfoKey, Type*>::iterator it;
    Type* result;
    it = map->find(key);
    if (it == map->end())
    {
      result = new DefaultListIteratorType(element);
      (*map)[key] = result;
    }
    else
      result = it->second;
    return result;
  }

  class DefaultListType: public TypeListImpl<vector1, void*>
  {
  public:
    DefaultListType(Type* elementType)
    : _elementType(elementType)
    {
    }
    Type* elementType(void* storage) const
    {
      return _elementType;
    }
    GenericListIterator begin(void* storage)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      GenericListIterator result;
      result.type = defaultListIteratorType(_elementType);
      std::vector<void*>::iterator it = ptr.begin();
      result.value = result.type->initializeStorage(&it);
      *(GenericValue*)&result = result.clone();
      return result;
    }
    GenericListIterator end(void* storage)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      GenericListIterator result;
      result.type = defaultListIteratorType(_elementType);
      std::vector<void*>::iterator it = ptr.end();
      result.value = result.type->initializeStorage(&it);
       *(GenericValue*)&result = result.clone();
      return result;
    }
    Type* _elementType;
    typedef DefaultTypeImplMethods<std::vector<void*> > Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

    // We want exactly one instance per element type
  Type* defaultListType(Type* element)
  {
    static std::map<detail::TypeInfoKey, Type*>* map = 0;
    if (!map)
      map = new std::map<detail::TypeInfoKey, Type*>();
    detail::TypeInfoKey key(element->info());
    std::map<detail::TypeInfoKey, Type*>::iterator it;
    Type* result;
    it = map->find(key);
    if (it == map->end())
    {
      result = new DefaultListType(element);
      (*map)[key] = result;
    }
    else
      result = it->second;
    return result;
  }


   // Default map

  class DefaultMapIteratorType: public TypeMapIteratorImpl<std::map<void*, void*> >
  {
  public:
    DefaultMapIteratorType(Type* keyType, Type* elementType)
    : _keyType(keyType)
    , _elementType(elementType)
    {}
    std::pair<GenericValue, GenericValue> dereference(void* storage)
    {
      std::map<void*, void*>::iterator& ptr = *(std::map<void*, void*>::iterator*)
        ptrFromStorage(&storage);
      GenericValue k;
      k.type = _keyType;
      k.value = ptr->first;
      GenericValue e;
      e.type = _elementType;
      e.value = ptr->second;
      return std::make_pair(k, e);
    }
    Type* _keyType;
    Type* _elementType;
  };

  // We want exactly one instance per element type
  static Type* defaultMapIteratorType(Type* kt, Type* et)
  {
    typedef std::map<std::pair<detail::TypeInfoKey, detail::TypeInfoKey>, Type*> Map;
    static Map * map = 0;
    if (!map)
      map = new Map();
    detail::TypeInfoKey kk(kt->info());
    detail::TypeInfoKey ek(et->info());
    Map::key_type key(kk, ek);
    Map::iterator it;
    Type* result;
    it = map->find(key);
    if (it == map->end())
    {
      result = new DefaultMapIteratorType(kt, et);
      (*map)[key] = result;
    }
    else
      result = it->second;
    return result;
  }

  class DefaultMapType: public TypeMapImpl<std::map<void*, void*> >
  {
  public:
    DefaultMapType(Type* keyType, Type* elementType)
    : _keyType(keyType)
    , _elementType(elementType)
    {
    }
    Type* elementType(void* storage) const
    {
      return _elementType;
    }
    Type* keyType (void* storage) const
    {
      return _keyType;
    }
    GenericMapIterator begin(void* storage)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      GenericMapIterator result;
      result.type = defaultMapIteratorType(_keyType, _elementType);
      std::vector<void*>::iterator it = ptr.begin();
      result.value = result.type->initializeStorage(&it);
      *(GenericValue*)&result = result.clone();
      return result;
    }
    GenericMapIterator end(void* storage)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      GenericMapIterator result;
      result.type = defaultMapIteratorType(_keyType, _elementType);
      std::vector<void*>::iterator it = ptr.end();
      result.value = result.type->initializeStorage(&it);
       *(GenericValue*)&result = result.clone();
      return result;
    }
    Type* _keyType;
    Type* _elementType;
    typedef DefaultTypeImplMethods<std::vector<void*> > Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

  // We want exactly one instance per element type
  Type* defaultMapType(Type* kt, Type* et)
  {
    typedef std::map<std::pair<detail::TypeInfoKey, detail::TypeInfoKey>, Type*> Map;
    static Map * map = 0;
    if (!map)
      map = new Map();
    detail::TypeInfoKey kk(kt->info());
    detail::TypeInfoKey ek(et->info());
    Map::key_type key(kk, ek);
    Map::iterator it;
    Type* result;
    it = map->find(key);
    if (it == map->end())
    {
      result = new DefaultMapType(kt, et);
      (*map)[key] = result;
    }
    else
      result = it->second;
    return result;
  }
  Type* defaultTupleType(std::vector<Type*> types)
  {
    qiLogError("qi.type") <<"not implemented lol";
    return 0;
  }
}


