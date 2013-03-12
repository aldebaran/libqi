/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>

#include <qitype/type.hpp>
#include <qitype/signature.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/typedispatcher.hpp>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

qiLogCategory("qitype.type");

namespace qi {


  TypeInfo::TypeInfo()
  : stdInfo(0)
  {}

  TypeInfo::TypeInfo(const std::type_info& info)
  : stdInfo(&info)
  {
  }

  TypeInfo::TypeInfo(const std::string& str)
  : stdInfo(0), customInfo(str)
  {

  }

  std::string TypeInfo::asString() const
  {
    if (stdInfo)
      return stdInfo->name();
    else
      return customInfo;
  }

  std::string TypeInfo::asDemangledString() const
  {
#ifdef __GNUC__
    if (stdInfo) {
      std::string tmp;
      int status;
      char *demangled = abi::__cxa_demangle(stdInfo->name(), NULL, NULL, &status);
      if (status == 0) {
        tmp = demangled;
        free(demangled);
        return tmp;
      } else {
        return stdInfo->name();
      }
    } else {
      return customInfo;
    }
#else
    return asString();
#endif
  }

  const char* TypeInfo::asCString() const
  {
    if (stdInfo)
      return stdInfo->name();
    else
      return customInfo.c_str();
  }

  bool TypeInfo::operator==(const TypeInfo& b) const
  {
    if (!! stdInfo != !! b.stdInfo)
      return false;
    if (stdInfo)
      return *stdInfo == *b.stdInfo;
    else
      return customInfo == b.customInfo;
  }

  bool TypeInfo::operator!=(const TypeInfo& b) const
  {
    return ! (*this == b);
  }

  bool TypeInfo::operator< (const TypeInfo& b) const
  {
    if (!! stdInfo != !! b.stdInfo)
      return stdInfo;
    else
    {
      if (stdInfo)
        return (*stdInfo).before(*b.stdInfo);
      else
        return customInfo < b.customInfo;
    }
  }

  typedef std::map<TypeInfo, Type*> TypeFactory;
  static TypeFactory& typeFactory()
  {
    static TypeFactory* res = 0;
    if (!res)
      res = new TypeFactory;
    return *res;
  }

  QITYPE_API Type* getType(const std::type_info& type)
  {
    static boost::mutex* mutex = 0;
    if (!mutex)
      mutex = new boost::mutex;
    boost::mutex::scoped_lock sl(*mutex);
    // We create-if-not-exist on purpose: to detect access that occur before
    // registration
    return typeFactory()[TypeInfo(type)];
  }

  /// Type factory setter
  QITYPE_API bool registerType(const std::type_info& typeId, Type* type)
  {
    qiLogCategory("qitype.type"); // method can be called at static init
    qiLogDebug() << "registerType "  << typeId.name() << " "
     << type->kind() <<" " << (void*)type;
    TypeFactory::iterator i = typeFactory().find(TypeInfo(typeId));
    if (i != typeFactory().end())
    {
      if (i->second)
        qiLogVerbose() << "registerType: previous registration present for "
          << typeId.name()<< " " << (void*)i->second << " " << i->second->kind();
      else
        qiLogVerbose() << "registerType: access to type factory before"
          " registration detected for type " << typeId.name();
    }
    typeFactory()[TypeInfo(typeId)] = type;
    return true;
  }

  class SignatureTypeVisitor
  {
  public:
    SignatureTypeVisitor(GenericValuePtr value, bool resolveDynamic)
    : _value(value)
    , _resolveDynamic(resolveDynamic)
    {
    }
    void visitVoid()
    {
      result = Signature::fromType(Signature::Type_Void).toString();
    }
    void visitInt(int64_t, bool isSigned, int byteSize)
    {
      switch((isSigned?1:-1)*byteSize)
      {
        case 0:  result = Signature::fromType(Signature::Type_Bool).toString();  break;
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
   void visitFloat(double, int byteSize)
   {
     if (byteSize == 4)
       result = Signature::fromType(Signature::Type_Float).toString();
     else
       result = Signature::fromType(Signature::Type_Double).toString();
   }
   void visitString(char*, size_t)
   {
     result = Signature::fromType(Signature::Type_String).toString();
   }
   void visitList(GenericIterator it, GenericIterator iend)
   {
     std::string esig = static_cast<TypeList*>(_value.type)->elementType()->signature();
     if (_resolveDynamic)
     {
       if (it == iend)
       { // Empty list, TODO have a 'whatever signature entry
         result =  std::string()
         + (char)Signature::Type_List
         + (char)Signature::Type_None
         + (char)Signature::Type_List_End;
       }
       else
       {
         std::string sigFirst = (*it).signature(true);
         ++it;
         for (;it != iend && !sigFirst.empty(); ++it)
         {
           std::string sig = (*it).signature(true);
           if (sig != sigFirst)
           {
             if (Signature(sig).isConvertibleTo(Signature(sigFirst)))
               {} // keep sigFirst
             else if (Signature(sigFirst).isConvertibleTo(Signature(sig)))
               sigFirst = sig; // keep sig
             else
             {
               qiLogDebug() << "Heterogeneous elements "
               << sigFirst << ' ' << sig;
             sigFirst.clear();
             }
           }
         }
         // qiLogDebug() << "List effective " << sigFirst;
         result =  std::string()
         + (char)Signature::Type_List
         + (sigFirst.empty()?esig:sigFirst)
         + (char)Signature::Type_List_End;
       }
     }
     else
       result = std::string()
        + (char)Signature::Type_List
        + esig
        + (char)Signature::Type_List_End;
   }
   void visitMap(GenericIterator it, GenericIterator iend)
   {
     TypeMap* type =  static_cast<TypeMap*>(_value.type);
     if (_resolveDynamic)
     {
       if (it == iend)
         result = std::string()
           + (char)Signature::Type_Map
           + (char)Signature::Type_None
           + (char)Signature::Type_None
           + (char)Signature::Type_Map_End;
       else
       {
         GenericValuePtr e = *it;
         std::string ksig = e[0].signature(true);
         std::string vsig = e[1].signature(true);
         // Check that ksig/vsig is always the same, set to empty if not
         ++it;
         for(; it!= iend; ++it)
         {
           GenericValuePtr e = *it;
           std::string k = e[0].signature(true);
           std::string v = e[1].signature(true);
           if (!ksig.empty() && ksig != k)
           {
             if (Signature(k).isConvertibleTo(Signature(ksig)))
               {}
             else if (Signature(ksig).isConvertibleTo(Signature(k)))
               ksig = k;
             else
             {
               qiLogDebug() << "Heterogeneous keys " << ksig << e[0].signature(true);
               ksig.clear();
             }
           }
           if (!vsig.empty() && vsig != v)
           {
              if (Signature(v).isConvertibleTo(Signature(vsig)))
               {}
             else if (Signature(vsig).isConvertibleTo(Signature(v)))
               vsig = v;
             else
             {
               qiLogDebug() << "Heterogeneous value " << vsig << e[1].signature(true);
               vsig.clear();
             }
           }
         }
         // qiLogDebug() << "Map effective: " << ksig << " , " << vsig;
         result = std::string()
           + (char)Signature::Type_Map
           + (ksig.empty()? type->keyType()->signature(): ksig)
           + (vsig.empty()? type->elementType()->signature(): vsig)
           + (char)Signature::Type_Map_End;
       }
     }
     else
       result = std::string()
        + (char)Signature::Type_Map
        + type->keyType()->signature()
        + type->elementType()->signature()
        + (char)Signature::Type_Map_End;
   }
   void visitObject(GenericObject )
   {
     result = Signature::fromType(Signature::Type_Object).toString();
   }
   void visitPointer(GenericValuePtr)
   {
     // Shared-ptr of Object can be serialized as type dynamic
     TypePointer* type = static_cast<TypePointer*>(_value.type);
     if (type->pointerKind() == TypePointer::Shared
       && type->pointedType()->kind() == Type::Object)
       result = Signature::fromType(Signature::Type_Dynamic).toString();
     else
       result = Signature::fromType(Signature::Type_Unknown).toString();
   }
   void visitUnknown(GenericValuePtr)
   {
     result = Signature::fromType(Signature::Type_Unknown).toString();
   }
   void visitTuple(const std::vector<GenericValuePtr>& tuple)
   {
     std::string res;
     res += (char)Signature::Type_Tuple;
     for (unsigned i=0; i<tuple.size(); ++i)
       res += tuple[i].signature(_resolveDynamic);
     res += (char)Signature::Type_Tuple_End;
     result = res;
   }
   void visitDynamic(GenericValuePtr pointee)
   {
     if (_resolveDynamic)
     {
       result = pointee.signature(true);
     }
     else
       result = Signature::fromType(Signature::Type_Dynamic).toString();
   }
   void visitRaw(GenericValuePtr)
   {
     result = Signature::fromType(Signature::Type_Raw).toString();
   }
   void visitIterator(GenericValuePtr v)
   {
     visitUnknown(v);
   }
   std::string result;
   GenericValuePtr _value;
   bool _resolveDynamic;
  };


  std::string Type::signature(void* storage, bool resolveDynamic)
  {
    if (resolveDynamic)
    {
      GenericValuePtr value(this, storage);
      SignatureTypeVisitor ts(value, resolveDynamic);
      typeDispatch(ts, value);
      return ts.result;
    }
    else
    { // We might be called without a valid storage in that mode, which
      // is not supported by typeDispatch()
      // Still reuse methods from SignatureTypeVisitor to avoid duplication
      GenericValuePtr value(this, storage);
      SignatureTypeVisitor v(value, resolveDynamic);
      switch(kind())
      {
      case Type::Void:
        v.result = Signature::fromType(Signature::Type_Void).toString();
        break;
      case Type::Int:
      {
        TypeInt* tint = static_cast<TypeInt*>(value.type);
        v.visitInt(0, tint->isSigned(), tint->size());
        break;
      }
      case Type::Float:
      {
        TypeFloat* tfloat = static_cast<TypeFloat*>(value.type);
        v.visitFloat(0, tfloat->size());
        break;
      }
      case Type::String:
        v.result = Signature::fromType(Signature::Type_String).toString();
        break;
      case Type::List:
        v.visitList(GenericIterator(), GenericIterator());
        break;
      case Type::Map:
        v.visitMap(GenericIterator(), GenericIterator());
        break;
      case Type::Object:
        v.result = Signature::fromType(Signature::Type_Object).toString();
        break;
      case Type::Pointer:
        v.visitPointer(GenericValuePtr());
        break;
      case Type::Tuple:
        v.result += (char)Signature::Type_Tuple;
        {
          std::vector<Type*> memberTypes = static_cast<TypeTuple*>(this)->memberTypes();
          for (unsigned i=0; i<memberTypes.size(); ++i)
            v.result += memberTypes[i]->signature();
        }
        v.result += (char)Signature::Type_Tuple_End;
        break;
      case Type::Dynamic:
        v.result = Signature::fromType(Signature::Type_Dynamic).toString();
        break;
      case Type::Raw:
        v.result = Signature::fromType(Signature::Type_Raw).toString();
        break;
      case Type::Unknown:
      case Type::Iterator:
         v.result = Signature::fromType(Signature::Type_Unknown).toString();
         break;
      }
      return v.result;
    }
  }


  static Type* fromSignature(const qi::Signature::iterator & i)
  {
    switch(i.type())
    {
    case Signature::Type_None:
    case Signature::Type_Void:
      return typeOf<void>();
    case Signature::Type_Bool:
      return typeOf<bool>();
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
    case Signature::Type_Int64:
      return typeOf<int64_t>();
    case Signature::Type_UInt64:
      return typeOf<uint64_t>();
    case Signature::Type_Float:
      return typeOf<float>();
    case Signature::Type_Double:
      return typeOf<double>();
    case Signature::Type_String:
      return typeOf<std::string>();
    case Signature::Type_List:
      {
        Type* el = fromSignature(i.children().begin());
        if (!el)
        {
          qiLogError() << "Cannot get type from list of unknown type.";
          return 0;
        }
      return makeListType(el);
      }
    case Signature::Type_Map:
      {
        Type* k = fromSignature(i.children().begin());
        Type* e = fromSignature(++i.children().begin());
        if (!k || !e)
        {
          qiLogError() <<" Cannot get type from map of unknown "
          << (k?"element":"key") << " type";
          return 0;
        }
        return makeMapType(k, e);
      }
    case Signature::Type_Tuple:
      {
        std::vector<Type*> types;
        Signature c = i.children();
        for (Signature::iterator child = c.begin(); child != c.end(); child++)
        {
          Type* t = fromSignature(child);
          if (!t)
          {
            qiLogError() << "Cannot get type from tuple of unknown element type";
            return 0;
          }
          // qiLogDebug() << "tuple element " << child.signature() << " " << t->infoString();
          types.push_back(t);
        }
        Type* res = makeTupleType(types);
        qiLogDebug() <<"Resulting tuple " << i.signature() << " " << res->infoString();
        return res;
      }
    case Signature::Type_Dynamic:
      return typeOf<GenericValue>();
    case Signature::Type_Raw:
      return typeOf<Buffer>();
    case Signature::Type_Object:
      return typeOf<ObjectPtr>();
    default:
      qiLogWarning() << "Cannot get type from signature " << i.signature();
      return 0;
    }
  }

  Type* Type::fromSignature(const qi::Signature& sig)
  {
    if (sig.size() != 1)
      qiLogWarning() << "fromSignature(): signature has more than one element: " << sig.toString();
    Signature::iterator i = sig.begin();
    Type* result = ::qi::fromSignature(i);
    // qiLogDebug() << "fromSignature() " << i.signature() << " -> " << (result?result->infoString():"NULL");
    return result;
  }

  // Default list
  static Type* makeListIteratorType(Type* element);

  class DefaultListIteratorType: public TypeIteratorImpl<std::vector<void*> >
  {
  public:
  private:
    DefaultListIteratorType(Type* elementType)
    : _elementType(elementType)
    {
      // We need an unique name, but elementType->nifo().aString() is not
      // garanteed unique. So use our address. The factory system ensures
      // non-duplication.
      _name = "DefaultListIteratorType<"
        + _elementType->info().asString()
        + ">(" + boost::lexical_cast<std::string>(this);
      _info = TypeInfo(_name);
    }
    friend Type* makeListIteratorType(Type*);
  public:
    GenericValueRef dereference(void* storage)
    {
      std::vector<void*>::iterator& ptr = *(std::vector<void*>::iterator*)
        ptrFromStorage(&storage);
      GenericValuePtr res;
      res.type = _elementType;
      res.value = *ptr;
      return res;
    }
    const TypeInfo& info()
    {
      return _info;
    }
    Type* _elementType;
    std::string _name;
    TypeInfo _info;
  };

  // We want exactly one instance per element type
  static Type* makeListIteratorType(Type* element)
  {
    static std::map<TypeInfo, Type*>* map = 0;
    if (!map)
      map = new std::map<TypeInfo, Type*>();
    TypeInfo key = element->info();
    std::map<TypeInfo, Type*>::iterator it;
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

  class DefaultListType: public TypeListImpl<std::vector<void*> >
  {
  public:
  private:
    DefaultListType(Type* elementType)
    : _elementType(elementType)
    {
       _name = "DefaultListType<"
        + _elementType->info().asString()
        + ">(" + boost::lexical_cast<std::string>(this);
        _info = TypeInfo(_name);
    }
    friend Type* makeListType(Type* element);
  public:

    Type* elementType() const
    {
      return _elementType;
    }
    GenericIterator begin(void* storage)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      std::vector<void*>::iterator it = ptr.begin();
      GenericValuePtr v = GenericValuePtr::ref(it);
      // Hugly type swap, works because we know backend storage matches
      v.type = makeListIteratorType(_elementType);
      return GenericIterator(v);
    }
    GenericIterator end(void* storage)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      std::vector<void*>::iterator it = ptr.end();
      GenericValuePtr v = GenericValuePtr::ref(it);
      // Hugly type swap, works because we know backend storage matches
      v.type = makeListIteratorType(_elementType);
      return GenericIterator(v);
    }
    void* clone(void* storage)
    {
      std::vector<void*>& src = *(std::vector<void*>*)ptrFromStorage(&storage);
      void* result = initializeStorage();
      std::vector<void*>& dst = *(std::vector<void*>*)ptrFromStorage(&result);
      for (unsigned i=0; i<src.size(); ++i)
        dst.push_back(_elementType->clone(src[i]));
      return result;
    }
    void destroy(void* storage)
    {
      std::vector<void*>& src = *(std::vector<void*>*)ptrFromStorage(&storage);
      for (unsigned i=0; i<src.size(); ++i)
        _elementType->destroy(src[i]);
      Methods::destroy(storage);
    }

    void pushBack(void** storage, void* valueStorage)
    {
      std::vector<void*>& src = *(std::vector<void*>*)ptrFromStorage(storage);
      src.push_back(_elementType->clone(valueStorage));
    }

    void* element(void* storage, int key)
    {
      std::vector<void*>& src = *(std::vector<void*>*)ptrFromStorage(&storage);
      return src[key];
    }
    const TypeInfo& info()
    {
      return _info;
    }
    typedef DefaultTypeImplMethods<std::vector<void*> > Methods;
    void* initializeStorage(void* ptr=0) { return Methods::initializeStorage(ptr);} \
    void* ptrFromStorage(void**s) { return Methods::ptrFromStorage(s);}
    Type* _elementType;
    std::string _name;
    TypeInfo _info;

  };

    // We want exactly one instance per element type
  Type* makeListType(Type* element)
  {
    static std::map<TypeInfo, Type*>* map = 0;
    if (!map)
      map = new std::map<TypeInfo, Type*>();
    TypeInfo key(element->info());
    std::map<TypeInfo, Type*>::iterator it;
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



  class DefaultTupleType: public TypeTuple
  {
  private:
    DefaultTupleType(std::vector<Type*> types)
    : types(types)
    {
      _name = "DefaultTupleType<";
      for (unsigned i=0; i<types.size(); ++i)
        _name += types[i]->info().asString() + ",";
      _name += ">(" + boost::lexical_cast<std::string>(this) + ")";
      qiLogDebug() << "Instanciating tuple " << _name;
      _info = TypeInfo(_name);
    }
    friend Type* makeTupleType(std::vector<Type*> types);
  public:
    virtual std::vector<Type*> memberTypes() { return types;}
    virtual void* get(void* storage, unsigned int index)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      if (ptr.size() < index +1)
        ptr.resize(index + 1, 0);
      return ptr[index];
    }
    virtual void set(void** storage, unsigned int index, void* valStorage)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(storage);
      if (ptr.size() < index +1)
        ptr.resize(index + 1, 0);
      if (ptr[index])
        types[index]->destroy(ptr[index]);
      ptr[index] = types[index]->clone(valStorage);
    }
    const TypeInfo& info()
    {
      return _info;
    }
    virtual void* clone(void* storage)
    {
      std::vector<void*>& src = *(std::vector<void*>*)ptrFromStorage(&storage);
      void* result = initializeStorage();
      for (unsigned i=0; i<src.size(); ++i)
        set(&result, i, src[i]); // set will clone
      return result;
    }
    virtual void destroy(void* storage)
    { // destroy elements that have been set
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      for (unsigned i=0; i<ptr.size(); ++i)
      {
        types[i]->destroy(ptr[i]);
      }
      Methods::destroy(storage);
    }

    void* initializeStorage(void* ptr=0) {
      std::vector<void*> *ret = (std::vector<void*>*)Methods::initializeStorage(ptr);
      ret->resize(types.size());
      for (unsigned i=0; i < ret->size(); ++i) {
        (*ret)[i] = types[i]->initializeStorage();
      }
      return ret;
    }

    virtual void* ptrFromStorage(void**s) { return Methods::ptrFromStorage(s);}

    std::vector<void*>& backend(void* storage)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      return ptr;
    }

    bool less(void* a, void* b) { return Methods::less(a, b);}
    std::vector<Type*> types;
    std::string _name;
    TypeInfo _info;
    typedef DefaultTypeImplMethods<std::vector<void*> > Methods;
  };

  GenericValuePtr makeGenericTuple(std::vector<GenericValuePtr> values)
  {
    std::vector<Type*> types;
    types.reserve(values.size());
    for (unsigned i=0; i<values.size(); ++i)
      types.push_back(values[i].type);

    TypeTuple* tupleType = static_cast<TypeTuple*>(makeTupleType(types));
    GenericValuePtr result;
    result.type = tupleType;
    result.value = tupleType->initializeStorage();
    std::vector<void*> storages;
    for (unsigned i=0; i<values.size(); ++i)
      storages.push_back(values[i].value);
    tupleType->set(&result.value, storages);
    return result;
  }




  // element of map is of type _pairType, see below
  typedef std::map<GenericValuePtr, void*> DefaultMapStorage;

  // Default map, using a vector<pair<void*, void*> > as storage
  static Type* makeMapIteratorType(Type* kt);

  class DefaultMapIteratorType: public TypeIterator
  {
  public:
  private:
    DefaultMapIteratorType(Type* elementType)
    : _elementType(elementType)
    {
      _name = "DefaultMapIteratorType<"
      + elementType->info().asString()
      + "(" + boost::lexical_cast<std::string>(this) + ")";
      _info = TypeInfo(_name);
    }
    friend Type* makeMapIteratorType(Type* kt);
  public:
    GenericValueRef dereference(void* storage)
    {
      /* Result is a pair<GV, void*>
       * and we must return something we store, pretending it is of
       * type pair<K&, V&>
       * The *pair itself* must be somehow stored
       *
       *
      */
      DefaultMapStorage::iterator& it = *(DefaultMapStorage::iterator*)
        ptrFromStorage(&storage);
      return GenericValuePtr(_elementType, it->second);
    }
    void next(void** storage)
    {
      DefaultMapStorage::iterator& ptr = *(DefaultMapStorage::iterator*)
        ptrFromStorage(storage);
      ++ptr;
    }
    bool equals(void* s1, void* s2)
    {
      DefaultMapStorage::iterator& p1 = *(DefaultMapStorage::iterator*)
        ptrFromStorage(&s1);
      DefaultMapStorage::iterator& p2 = *(DefaultMapStorage::iterator*)
        ptrFromStorage(&s2);
      return p1 == p2;
    }
    const TypeInfo& info()
    {
      return _info;
    }
    _QI_BOUNCE_TYPE_METHODS_NOINFO(DefaultTypeImplMethods<DefaultMapStorage::iterator>);
    Type* _elementType;
    std::string _name;
    TypeInfo _info;
  };

  // We want exactly one instance per element type
  static Type* makeMapIteratorType(Type* te)
  {
    typedef std::map<TypeInfo, Type*> Map;
    static Map * map = 0;
    if (!map)
      map = new Map();
    TypeInfo ti(te->info());
    Map::key_type key(ti);
    Map::iterator it;
    Type* result;
    it = map->find(key);
    if (it == map->end())
    {
      result = new DefaultMapIteratorType(te);
      (*map)[key] = result;
    }
    else
      result = it->second;
    return result;
  }

  class DefaultMapType: public TypeMap
  {
  public:
  private:
    DefaultMapType(Type* keyType, Type* elementType)
    : _keyType(keyType)
    , _elementType(elementType)
    {
      _name = "DefaultMapType<"
      + keyType->info().asString() + ", "
      + elementType->info().asString()
      + "(" + boost::lexical_cast<std::string>(this) + ")";
      _info = TypeInfo(_name);
      std::vector<Type*> kvtype;
      kvtype.push_back(_keyType);
      kvtype.push_back(_elementType);
      _pairType = static_cast<DefaultTupleType*>(makeTupleType(kvtype));
      assert(dynamic_cast<DefaultTupleType*>(_pairType));
    }
    friend Type* makeMapType(Type* kt, Type* et);
  public:
    Type* elementType() const
    {
      return _elementType;
    }
    Type* keyType () const
    {
      return _keyType;
    }
    GenericIterator begin(void* storage)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*)ptrFromStorage(&storage);
      DefaultMapStorage::iterator it = ptr.begin();
      GenericValuePtr val = GenericValuePtr::ref(it);
      val.type = makeMapIteratorType(_pairType);
      return GenericIterator(val);
    }
    GenericIterator end(void* storage)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*)ptrFromStorage(&storage);
      DefaultMapStorage::iterator it = ptr.end();
      GenericValuePtr val = GenericValuePtr::ref(it);
      val.type = makeMapIteratorType(_pairType);
      return GenericIterator(val);

    }

    // Unconditional insert, assumes key is not present, return value
    GenericValuePtr _insert(DefaultMapStorage& ptr, void* keyStorage, void* valueStorage, bool copyValue)
    {
      // key is referenced in map key, and map value for the pair
      GenericValuePtr key(_keyType, keyStorage);
      key = key.clone();
      GenericValuePtr value(_elementType, valueStorage);
      if (copyValue)
        value = value.clone();
      // must cast or wrong GenericValuePtr ctor is called
      // We know that _pairType is a DefaultTupleType, so optimize:
      // if we construct a value from _pairType it will allocate the pair content
      void* pairPtr = DefaultTupleType::Methods::initializeStorage();
      std::vector<void*>&pair = *(std::vector<void*>*) pairPtr;
      pair.resize(2);
      pair[0] = key.value;
      pair[1] = value.value;
      ptr[key] = pairPtr;
      return value;
    }

    void insert(void** storage, void* keyStorage, void* valueStorage)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*)ptrFromStorage(storage);
      DefaultMapStorage::iterator i = ptr.find(GenericValuePtr(_keyType, keyStorage));
      if (i != ptr.end())
      {// Replace: clear previous storage
        // Now, normally tuple (_pairType is one) only have inplace set
        // But this is not any tuple, we know it's a DefaultTuple
        // So we need to hack.
        std::vector<void*>& elem = _pairType->backend(i->second);
        assert(elem.size() == 2);
        _elementType->destroy(elem[1]);
        elem[1] = GenericValuePtr(_elementType, valueStorage).clone().value;
      }
      else
      {
        _insert(ptr, keyStorage, valueStorage, true);
      }
    }

    GenericValuePtr element(void** pstorage, void* keyStorage, bool autoInsert)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*) ptrFromStorage(pstorage);
      DefaultMapStorage::iterator i = ptr.find(GenericValuePtr(_keyType, keyStorage));
      if (i != ptr.end())
      {
        GenericValuePtr elem(_pairType, i->second);
        return elem[1];
      }
      if (!autoInsert)
        return GenericValuePtr();
      return _insert(ptr, keyStorage, _elementType->initializeStorage(), false);
    }

    size_t size(void* storage)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*) ptrFromStorage(&storage);
      return ptr.size();
    }
    void destroy(void* storage)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*)ptrFromStorage(&storage);
      for (DefaultMapStorage::iterator it = ptr.begin(); it != ptr.end(); ++it)
      {
        // destroying the pair will destroy key and value
        _pairType->destroy(it->second);
      }
      Methods::destroy(storage);
    }
    void* clone(void* storage)
    {
      void* result = initializeStorage();
      DefaultMapStorage& src = *(DefaultMapStorage*)ptrFromStorage(&storage);
      DefaultMapStorage& dst = *(DefaultMapStorage*)ptrFromStorage(&result);
      // must clone content
      for (DefaultMapStorage::iterator it = src.begin(); it != src.end(); ++it)
      {
        // do not double-clone the key, which is in the pair also
        GenericValuePtr clonedPair(_pairType, _pairType->clone(it->second));
        dst[clonedPair[0]] = clonedPair.value;
      }
      return result;
    }
    const TypeInfo& info()
    {
      return _info;
    }
    typedef DefaultTypeImplMethods<DefaultMapStorage> Methods;
    void* initializeStorage(void* ptr=0) { return Methods::initializeStorage(ptr);}   \
    virtual void* ptrFromStorage(void**s) { return Methods::ptrFromStorage(s);}
    bool less(void* a, void* b) { return Methods::less(a, b);}
    Type* _keyType;
    Type* _elementType;
    DefaultTupleType* _pairType;
    TypeInfo _info;
    std::string _name;
  };

  // We want exactly one instance per element type
  Type* makeMapType(Type* kt, Type* et)
  {
    typedef std::map<std::pair<TypeInfo, TypeInfo>, TypeMap*> Map;
    static Map * map = 0;
    if (!map)
      map = new Map();
    TypeInfo kk(kt->info());
    TypeInfo ek(et->info());
    Map::key_type key(kk, ek);
    Map::iterator it;
    TypeMap* result;
    it = map->find(key);
    if (it == map->end())
    {
      result = new DefaultMapType(kt, et);
      (*map)[key] = result;
    }
    else
    {
      result = it->second;
    }
    return result;
  }

  struct InfosKey: public std::vector<Type*>
  {
  public:
    InfosKey(const std::vector<Type*>& b)
    : std::vector<Type*>(b) {}
    bool operator < (const InfosKey& b) const
    {
      if (size() != b.size())
        return size() < b.size();
      for (unsigned i=0; i<size(); ++i)
      {
        if ( (*this)[i]->info() != b[i]->info())
          return (*this)[i]->info() < b[i]->info();
      }
      return false;
    }
  };
  Type* makeTupleType(std::vector<Type*> types)
  {
    typedef std::map<InfosKey, TypeTuple*> Map;
    static Map* map = 0;
    if (!map)
      map = new Map;
    InfosKey key(types);
    Map::iterator it = map->find(key);
    if (it == map->end())
    {
      TypeTuple* result = new DefaultTupleType(types);
      (*map)[key] = result;
      return result;
    }
    else
    {
      TypeTuple* res = it->second;
      assert(res->memberTypes().size() == types.size());
      return res;
    }
  }

  void* TypeList::element(void* storage, int index)
  {
    // Default implementation using iteration
    GenericValuePtr self(this, storage);
    GenericIterator it = self.begin();
    GenericIterator iend = self.end();
    int p = 0;
    while (p!= index && it!= iend)
    {
      ++p;
      ++it;
    }
    if (p > index)
      throw std::runtime_error("Index out of range");
    return (*it).value;
  }

  namespace detail
  {
    void typeFail(const char* typeName, const char* operation)
    {
      /* Use an internal map and be untemplated to avoid generating zillions
      * of symbols
      */
      static std::set<std::string>* once = 0;
      if (!once)
        once = new std::set<std::string>();
      if (once->find(typeName)!=once->end())
        return;
      once->insert(typeName);
      qiLogError() << "The following operation failed on data type "
      << typeName << " :" << operation;
    }
  }
}

namespace
{
  // trick to avoid wrapping in a function: setCategory returns void so can't
  // be used in expr.
  static bool __unused_debug = qi::os::getenv("QITYPE_DEBUG").empty()
    && (::qi::log::setCategory("qitype.*", ::qi::log::info), false);
}

