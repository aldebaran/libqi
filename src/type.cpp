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
    qiLogDebug("qi.meta") << "registerType "  << typeId.name() << " "
     << type->kind() <<" " << (void*)type;
    TypeFactory::iterator i = typeFactory().find(TypeInfo(typeId));
    if (i != typeFactory().end())
    {
      if (i->second)
        qiLogVerbose("qi.meta") << "registerType: previous registration present for "
          << typeId.name()<< " " << (void*)i->second << " " << i->second->kind();
      else
        qiLogVerbose("qi.meta") << "registerType: access to type factory before"
          " registration detected for type " << typeId.name();
    }
    typeFactory()[TypeInfo(typeId)] = type;
    return true;
  }

  class SignatureTypeVisitor
  {
  public:
    SignatureTypeVisitor(bool resolveDynamic)
    : _resolveDynamic(resolveDynamic)
    {
    }
    void visitVoid(Type*)
    {
      result = Signature::fromType(Signature::Type_Void).toString();
    }
    void visitInt(TypeInt* type, int64_t, bool isSigned, int byteSize)
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
   void visitList(GenericListPtr value)
   {
     if (_resolveDynamic)
     {
       if (value.size() == 0)
       { // Empty list, TODO have a 'whatever signature entry
         result =  std::string()
         + (char)Signature::Type_List
         + (char)Signature::Type_None
         + (char)Signature::Type_List_End;
       }
       else
       {
         GenericListIteratorPtr it = value.begin();
         GenericListIteratorPtr iend = value.end();
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
               qiLogDebug("qi.signature") << "Heterogeneous elements "
               << sigFirst << ' ' << sig;
             sigFirst.clear();
             }
           }
         }
         // qiLogDebug("qi.signature") << "List effective " << sigFirst;
         result =  std::string()
         + (char)Signature::Type_List
         + (sigFirst.empty()?value.elementType()->signature():sigFirst)
         + (char)Signature::Type_List_End;
         it.destroy();
         iend.destroy();
       }
     }
     else
       result = std::string()
        + (char)Signature::Type_List
        + value.elementType()->signature()
        + (char)Signature::Type_List_End;
   }
   void visitMap(GenericMapPtr value)
   {
     if (_resolveDynamic)
     {
       if (value.size() == 0)
         result = std::string()
           + (char)Signature::Type_Map
           + (char)Signature::Type_None
           + (char)Signature::Type_None
           + (char)Signature::Type_Map_End;
       else
       {
         GenericMapIteratorPtr it = value.begin();
         GenericMapIteratorPtr iend = value.end();
         std::pair<GenericValuePtr, GenericValuePtr> e = *it;
         std::string ksig = e.first.signature(true);
         std::string vsig = e.second.signature(true);
         // Check that ksig/vsig is always the same, set to empty if not
         ++it;
         for(; it!= iend; ++it)
         {
           std::pair<GenericValuePtr, GenericValuePtr> e = *it;
           std::string k = e.first.signature(true);
           std::string v = e.second.signature(true);
           if (!ksig.empty() && ksig != k)
           {
             if (Signature(k).isConvertibleTo(Signature(ksig)))
               {}
             else if (Signature(ksig).isConvertibleTo(Signature(k)))
               ksig = k;
             else
             {
               qiLogDebug("qi.signature") << "Heterogeneous keys " << ksig << e.first.signature(true);
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
               qiLogDebug("qi.signature") << "Heterogeneous value " << vsig << e.second.signature(true);
               vsig.clear();
             }
           }
         }
         it.destroy();
         iend.destroy();
         // qiLogDebug("qi.signature") << "Map effective: " << ksig << " , " << vsig;
         result = std::string()
           + (char)Signature::Type_Map
           + (ksig.empty()? value.keyType()->signature(): ksig)
           + (vsig.empty()? value.elementType()->signature(): vsig)
           + (char)Signature::Type_Map_End;
       }
     }
     else
       result = std::string()
        + (char)Signature::Type_Map
        + value.keyType()->signature()
        + value.elementType()->signature()
        + (char)Signature::Type_Map_End;
   }
   void visitObject(GenericObject )
   {
     result = Signature::fromType(Signature::Type_Object).toString();
   }
   void visitPointer(TypePointer* type, void* , GenericValuePtr )
   {
     // Shared-ptr of Object can be serialized as type dynamic
     if (type->pointerKind() == TypePointer::Shared
       && type->pointedType()->kind() == Type::Object)
       result = Signature::fromType(Signature::Type_Dynamic).toString();
     else
       result = Signature::fromType(Signature::Type_Unknown).toString();
   }
   void visitUnknown(Type* type, void*)
   {
     result = Signature::fromType(Signature::Type_Unknown).toString();
   }
   void visitTuple(TypeTuple* type, void* storage)
   {
     std::vector<Type*> types = type->memberTypes();
     std::string res;
     res += (char)Signature::Type_Tuple;
     if (_resolveDynamic)
     {
       for (unsigned i=0; i<types.size(); ++i)
       {
         res += types[i]->signature(type->get(storage, i), true);
       }
     }
     else
       for (unsigned i=0; i<types.size(); ++i)
         res += types[i]->signature();
     res += (char)Signature::Type_Tuple_End;
     result = res;
   }
   void visitDynamic(GenericValuePtr source, GenericValuePtr pointee)
   {
     if (_resolveDynamic)
     {
       result = pointee.signature(true);
     }
     else
       result = Signature::fromType(Signature::Type_Dynamic).toString();
   }
   void visitRaw(TypeRaw* type, Buffer* buffer)
   {
     result = Signature::fromType(Signature::Type_Raw).toString();
   }
   std::string result;
   bool _resolveDynamic;
  };


  std::string Type::signature(void* storage, bool resolveDynamic)
  {
    SignatureTypeVisitor ts(resolveDynamic);
    typeDispatch(ts, this, resolveDynamic?&storage:(void**)0);
    return ts.result;
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
          qiLogError("qi.type") << "Cannot get type from list of unknown type.";
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
          qiLogError("qi.type") <<" Cannot get type from map of unknown "
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
            qiLogError("qi.type") << "Cannot get type from tuple of unknown element type";
            return 0;
          }
          // qiLogDebug("qi.type") << "tuple element " << child.signature() << " " << t->infoString();
          types.push_back(t);
        }
        Type* res = makeTupleType(types);
        qiLogDebug("qi.type") <<"Resulting tuple " << i.signature() << " " << res->infoString();
        return res;
      }
    case Signature::Type_Dynamic:
      return typeOf<GenericValue>();
    case Signature::Type_Raw:
      return typeOf<Buffer>();
    case Signature::Type_Object:
      return typeOf<ObjectPtr>();
    default:
      qiLogWarning("qi.type") << "Cannot get type from signature " << i.signature();
      return 0;
    }
  }

  Type* Type::fromSignature(const qi::Signature& sig)
  {
    if (sig.size() != 1)
      qiLogWarning("qi.type") << "fromSignature(): signature has more than one element: " << sig.toString();
    Signature::iterator i = sig.begin();
    Type* result = ::qi::fromSignature(i);
    // qiLogDebug("qi.type") << "fromSignature() " << i.signature() << " -> " << (result?result->infoString():"NULL");
    return result;
  }

  // Default list
  static Type* makeListIteratorType(Type* element);

  class DefaultListIteratorType: public TypeListIteratorImpl<std::vector<void*> >
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
    GenericValuePtr dereference(void* storage)
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
    GenericListIteratorPtr begin(void* storage)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      GenericListIteratorPtr result;
      result.type = makeListIteratorType(_elementType);
      std::vector<void*>::iterator it = ptr.begin();
      result.value = result.type->initializeStorage(&it);
      *(GenericValuePtr*)&result = result.clone();
      return result;
    }
    GenericListIteratorPtr end(void* storage)
    {
      std::vector<void*>& ptr = *(std::vector<void*>*)ptrFromStorage(&storage);
      GenericListIteratorPtr result;
      result.type = makeListIteratorType(_elementType);
      std::vector<void*>::iterator it = ptr.end();
      result.value = result.type->initializeStorage(&it);
       *(GenericValuePtr*)&result = result.clone();
      return result;
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


  /* For performances, our Generic Map uses vector<pair<void*, void*> >
   * as storage, since we do not require Generic Map to provide a map API.
  */
  typedef std::vector<std::pair<void*, void*> > DefaultMapStorage;
  // Default map, using a vector<pair<void*, void*> > as storage
  static Type* makeMapIteratorType(Type* kt, Type* et);

  class DefaultMapIteratorType: public TypeMapIterator
  {
  public:
  private:
    DefaultMapIteratorType(Type* keyType, Type* elementType)
    : _keyType(keyType)
    , _elementType(elementType)
    {
      _name = "DefaultMapIteratorType<"
      + keyType->info().asString() + ", "
      + elementType->info().asString()
      + "(" + boost::lexical_cast<std::string>(this) + ")";
      _info = TypeInfo(_name);
    }
    friend Type* makeMapIteratorType(Type* kt, Type* et);
  public:
    std::pair<GenericValuePtr, GenericValuePtr> dereference(void* storage)
    {
      DefaultMapStorage::iterator& ptr = *(DefaultMapStorage::iterator*)
        ptrFromStorage(&storage);
      GenericValuePtr k;
      k.type = _keyType;
      k.value = ptr->first;
      GenericValuePtr e;
      e.type = _elementType;
      e.value = ptr->second;
      return std::make_pair(k, e);
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
    Type* _keyType;
    Type* _elementType;
    std::string _name;
    TypeInfo _info;
  };

  // We want exactly one instance per element type
  static Type* makeMapIteratorType(Type* kt, Type* et)
  {
    typedef std::map<std::pair<TypeInfo, TypeInfo>, Type*> Map;
    static Map * map = 0;
    if (!map)
      map = new Map();
    TypeInfo kk(kt->info());
    TypeInfo ek(et->info());
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
    GenericMapIteratorPtr begin(void* storage)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*)ptrFromStorage(&storage);
      GenericMapIteratorPtr result;
      result.type = makeMapIteratorType(_keyType, _elementType);
      DefaultMapStorage::iterator it = ptr.begin();
      result.value = result.type->initializeStorage(&it);
      *(GenericValuePtr*)&result = result.clone();
      return result;
    }
    GenericMapIteratorPtr end(void* storage)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*)ptrFromStorage(&storage);
      GenericMapIteratorPtr result;
      result.type = makeMapIteratorType(_keyType, _elementType);
      DefaultMapStorage::iterator it = ptr.end();
      result.value = result.type->initializeStorage(&it);
      *(GenericValuePtr*)&result = result.clone();
      return result;

    }
    void insert(void** storage, void* keyStorage, void* valueStorage)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*)ptrFromStorage(storage);
      ptr.push_back(std::make_pair(
        _keyType->clone(keyStorage), _elementType->clone(valueStorage)));
    }
    size_t size(void* storage)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*) ptrFromStorage(&storage);
      return ptr.size();
    }
    void destroy(void* storage)
    {
      DefaultMapStorage& ptr = *(DefaultMapStorage*)ptrFromStorage(&storage);
      for (unsigned i=0; i<ptr.size(); ++i)
      {
        _keyType->destroy(ptr[i].first);
        _elementType->destroy(ptr[i].second);
      }
      Methods::destroy(storage);
    }
    void* clone(void* storage)
    {
      void* result = initializeStorage();
      DefaultMapStorage& src = *(DefaultMapStorage*)ptrFromStorage(&storage);
      DefaultMapStorage& dst = *(DefaultMapStorage*)ptrFromStorage(&result);
      for (unsigned i=0; i<src.size(); ++i)
        dst.push_back(std::make_pair(
          _keyType->clone(src[i].first),
          _elementType->clone(src[i].second)
          ));
      return result;
    }
    const TypeInfo& info()
    {
      return _info;
    }
    typedef DefaultTypeImplMethods<DefaultMapStorage> Methods;
    void* initializeStorage(void* ptr=0) { return Methods::initializeStorage(ptr);}   \
    virtual void* ptrFromStorage(void**s) { return Methods::ptrFromStorage(s);}

    Type* _keyType;
    Type* _elementType;
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
      qiLogDebug("qi.type") << "Instanciating tuple " << _name;
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
      qiLogError("qi.type") << "The following operation failed on data type "
      << typeName << " :" << operation;
    }
  }
}


