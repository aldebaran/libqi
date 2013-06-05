/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/lexical_cast.hpp>

#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>

#if defined(_MSC_VER) && _MSC_VER <= 1500
// vs2008 32 bits does not have std::abs() on int64
namespace std
{
  qi::int64_t abs(qi::int64_t x)
  {
    if (x < 0)
      return -x;
    else
      return x;
  }
}
#endif

qiLogCategory("qitype.genericvalue");

namespace qi
{

  namespace
  {
    static void dropIt(const GenericValue& v)
    {
    }
  }


  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(TypeDynamic* targetType) const
  {
    GenericValuePtr result;

    result.type = targetType;
    result.value = targetType->initializeStorage();
    static_cast<TypeDynamic*>(targetType)->set(&result.value, *this);
    return std::make_pair(result, true);
    return std::make_pair(GenericValuePtr(), false);
  }

  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(TypePointer* targetType) const
  {
    GenericValuePtr result;

    switch (type->kind())
    {
    case Type::Pointer:
    {
      Type* srcPointedType = static_cast<TypePointer*>(type)->pointedType();
      Type* dstPointedType = static_cast<TypePointer*>(targetType)->pointedType();
      // We only try to handle conversion for pointer to objects
      if (srcPointedType->kind() != Type::Object || dstPointedType->kind() != Type::Object)
      {
        // However, we need the full check for exact match here
        if (type->info() == targetType->info())
          return std::make_pair(*this, false);
        else
        {
          qiLogDebug() << "Conversion between non-object pointers not supported";
          return std::make_pair(GenericValuePtr(), false);
        }
      }
      GenericValuePtr pointedSrc = static_cast<TypePointer*>(type)->dereference(value);
      // try object conversion (inheritance)
      std::pair<GenericValuePtr, bool> pointedDstPair = pointedSrc.convert(dstPointedType);
      if (!pointedDstPair.first.type)
      {
        qiLogDebug() << "Attempting object->proxy conversion";
        // try object->proxy conversion by simply rewrapping this
        ObjectPtr o(new GenericObject(static_cast<ObjectType*>(pointedSrc.type), pointedSrc.value));
        return GenericValueRef(o).convert((Type*)targetType);
      }
      if (pointedDstPair.second)
        qiLogError() << "assertion error, allocated converted reference";
      // We must re-reference
      GenericValuePtr pointedDst = pointedDstPair.first;
      void* ptr = pointedDst.type->ptrFromStorage(&pointedDst.value);
      result.type = targetType;
      result.value = targetType->initializeStorage(&ptr);
      return std::make_pair(result, false);
    }
    case Type::Object:
    {
      std::pair<GenericValuePtr, bool> gv = convert(
                                              static_cast<TypePointer*>(targetType)->pointedType());
      if (!gv.first.type)
        return gv;
      // Re-pointerise it
      void* ptr = gv.first.type->ptrFromStorage(&gv.first.value);
      GenericValuePtr result;
      result.type = targetType;
      result.value = targetType->initializeStorage(&ptr);
      return std::make_pair(result, false);
    }
    default:
      break;
    }
    return std::make_pair(GenericValuePtr(), false);
  }

  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(TypeList* targetType) const
  {
    GenericValuePtr result;

    switch (type->kind())
    {
    case Type::List:
    {
      TypeList* targetListType = static_cast<TypeList*>(targetType);
      TypeList* sourceListType = static_cast<TypeList*>(type);

      Type* srcElemType = sourceListType->elementType();
      Type* dstElemType = targetListType->elementType();
      bool needConvert = (srcElemType->info() != dstElemType->info());
      result = GenericValuePtr((Type*)targetListType);

      GenericIterator iend = end();
      for (GenericIterator it = begin(); it!= iend; ++it)
      {
        GenericValuePtr val = *it;
        if (!needConvert)
          result._append(val);
        else
        {
          std::pair<GenericValuePtr,bool> c = val.convert(dstElemType);
          if (!c.first.type)
          {
            qiLogDebug() << "List element conversion failure from "
                         << val.type->infoString() << " to " << dstElemType->infoString();
            result.destroy();
            return std::make_pair(GenericValuePtr(), false);
          }
          result._append(c.first);
          if (c.second)
            c.first.destroy();
        }
      }
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(GenericValuePtr(), false);
  }

  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(TypeString* targetType) const
  {
    GenericValuePtr result;

    switch (type->kind())
    {
    case Type::String:
    {
      if (targetType->info() == type->info())
        return std::make_pair(*this, false);
      result.type = targetType;
      result.value = targetType->initializeStorage();
      std::pair<char*, size_t> v = static_cast<TypeString*>(type)->get(value);
      static_cast<TypeString*>(targetType)->set(&result.value,
                                                v.first, v.second);
      return std::make_pair(result, true);
    }
    case Type::Raw:
    {
      qiLogWarning() << "Conversion attempt from raw to string";
      return std::make_pair(GenericValuePtr(), false);
    }
    default:
      break;
    }
    return std::make_pair(GenericValuePtr(), false);
  }

  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(TypeRaw* targetType) const
  {
    GenericValuePtr result;

    switch (type->kind())
    {
    case Type::Raw:
    {
      if (targetType->info() == type->info())
        return std::make_pair(*this, false);
      result.type = targetType;
      result.value = targetType->initializeStorage();
      std::pair<char*, size_t> v = static_cast<TypeRaw*>(type)->get(value);
      static_cast<TypeRaw*>(result.type)->set(&result.value, v.first, v.second);
      return std::make_pair(result, true);
    }
    case Type::String:
    {
      std::pair<char*, size_t> data = static_cast<TypeString*>(type)->get(value);
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<TypeRaw*>(result.type)->set(&result.value, data.first, data.second);
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(GenericValuePtr(), false);
  }

  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(TypeFloat* targetType) const
  {
    GenericValuePtr result;

    switch (type->kind())
    {
    case Type::Float:
    {
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<TypeFloat*>(targetType)->set(&result.value,
                                               static_cast<TypeFloat*>(type)->get(value));
      return std::make_pair(result, true);
    }
    case Type::Int:
    {
      GenericValuePtr result(static_cast<Type*>(targetType));
      int64_t v = static_cast<TypeInt*>(type)->get(value);
      if (static_cast<TypeInt*>(type)->isSigned())
        result.setInt(v);
      else
        result.setUInt((uint64_t)v);
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(GenericValuePtr(), false);
  }

  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(TypeInt* targetType) const
  {
    GenericValuePtr result;

    switch (type->kind())
    {
    case Type::Int:
    {
      TypeInt* tsrc = static_cast<TypeInt*>(type);
      TypeInt* tdst = static_cast<TypeInt*>(targetType);
      int64_t v = tsrc->get(value);
      /* Bounce to GVP to perform overflow checks
      */
      GenericValuePtr result((Type*)tdst);
      if (tsrc->isSigned())
        result.setInt(v);
      else
        result.setUInt((uint64_t)v);
      tdst->set(&result.value, v);
      return std::make_pair(result, true);
    }
    case Type::Float:
    {
      double v = static_cast<TypeFloat*>(type)->get(value);
      TypeInt* tdst = static_cast<TypeInt*>(targetType);
      GenericValuePtr result((Type*)tdst);
      // bounce to setDouble for overflow check
      result.setDouble(v);
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(GenericValuePtr(), false);
  }

  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(TypeTuple* targetType) const
  {
    GenericValuePtr result;
    TypeTuple* tdst = targetType;

    switch (type->kind())
    {
    case Type::Tuple:
    {
      TypeTuple* tsrc = static_cast<TypeTuple*>(type);
      std::vector<void*> sourceData = tsrc->get(value);
      std::vector<Type*> srcTypes = tsrc->memberTypes();
      std::vector<Type*> dstTypes = tdst->memberTypes();
      if (dstTypes.size() != sourceData.size())
      {
        qiLogWarning() << "Conversion failure: tuple size mismatch between " << tsrc->signature().toString() << " and " << tdst->signature().toString();
        return std::make_pair(GenericValuePtr(), false);
      }

      std::vector<void*> targetData;
      std::vector<bool> mustDestroy;
      for (unsigned i=0; i<dstTypes.size(); ++i)
      {
        std::pair<GenericValuePtr, bool> conv = GenericValuePtr(srcTypes[i], sourceData[i]).convert(dstTypes[i]);
        if (!conv.first.type)
        {
          qiLogWarning() << "Conversion failure in tuple member between "
                         << srcTypes[i]->infoString() << " and " << dstTypes[i]->infoString();
          return std::make_pair(GenericValuePtr(), false);
        }
        targetData.push_back(conv.first.value);
        mustDestroy.push_back(conv.second);
      }
      void* dst = tdst->initializeStorage();
      tdst->set(&dst, targetData);
      for (unsigned i=0; i<mustDestroy.size(); ++i)
      {
        if (mustDestroy[i])
          dstTypes[i]->destroy(targetData[i]);
      }
      result.type = targetType;
      result.value = dst;
      return std::make_pair(result, true);
    }
    case Type::List:
    {
      // No explicit type-check, convert will do it
      // handles for instance [i] -> (iii) if size matches
      // or [m] -> (anything) if effective types match
      TypeList* tsrc = static_cast<TypeList*>(type);

      GenericIterator srcBegin = tsrc->begin(value);
      GenericIterator srcEnd = tsrc->end(value);

      std::vector<Type*> dstTypes = tdst->memberTypes();
      std::vector<void*> targetData;
      targetData.reserve(dstTypes.size());
      std::vector<bool> mustDestroy;
      mustDestroy.reserve(dstTypes.size());

      if (this->size() != dstTypes.size())
      {
        qiLogWarning() << "Conversion failure: containers size mismatch between "
                       << tsrc->signature().toString() << " and " << tdst->signature().toString();
        return std::make_pair(GenericValuePtr(), false);
      }

      unsigned int i = 0;
      while (srcBegin != srcEnd)
      {
        std::pair<GenericValuePtr, bool> conv = (*srcBegin).convert(dstTypes[i]);
        if (!conv.first.type)
        {
          for (unsigned u = 0; u < targetData.size(); ++u)
            if (mustDestroy[u])
              dstTypes[u]->destroy(targetData[u]);
          return std::make_pair(GenericValuePtr(), false);
        }
        targetData.push_back(conv.first.value);
        mustDestroy.push_back(conv.second);
        ++srcBegin;
        ++i;
      }
      void* dst = tdst->initializeStorage();
      tdst->set(&dst, targetData);
      for (i = 0; i < mustDestroy.size(); ++i)
      {
        if (mustDestroy[i])
          dstTypes[i]->destroy(targetData[i]);
      }
      result.type = targetType;
      result.value = dst;
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(GenericValuePtr(), false);
  }

  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(TypeMap* targetType) const
  {
    GenericValuePtr result;

    switch (type->kind())
    {
    case Type::Map:
    {
      result = GenericValuePtr(static_cast<Type*>(targetType));

      TypeMap* targetMapType = static_cast<TypeMap*>(targetType);
      TypeMap* srcMapType = static_cast<TypeMap*>(type);

      Type* srcKeyType = srcMapType->keyType();
      Type* srcElementType = srcMapType->elementType();

      Type* targetKeyType = targetMapType->keyType();
      Type* targetElementType = targetMapType->elementType();

      bool sameKey = srcKeyType->info() == targetKeyType->info();
      bool sameElem = srcElementType->info() == targetElementType->info();

      GenericIterator iend = end();
      for (GenericIterator it = begin(); it != iend; ++it)
      {
        std::pair<GenericValuePtr, bool> ck, cv;
        GenericValuePtr kv = *it;
        if (!sameKey)
        {
          ck = kv[0].convert(targetKeyType);
          if (!ck.first.type)
            return std::make_pair(GenericValuePtr(), false);
        }
        if (!sameElem)
        {
          cv = kv[1].convert(targetElementType);
          if (!cv.first.type)
            return std::make_pair(GenericValuePtr(), false);
        }
        result._insert(sameKey?kv[0]:ck.first, sameElem?kv[1]:cv.first);
        if (!sameKey && ck.second)
          ck.first.destroy();
        if (!sameElem && cv.second)
          cv.first.destroy();
      }
      return std::make_pair(result, true);
    }
    case Type::List:
    {
      // Accept [(kv)] and convert to {kv}
      // Also accept [[m]] , [[k]] if k=v and size match, and other compatible stuffs
      result = GenericValuePtr(static_cast<Type*>(targetType));
      TypeList* tsrc = static_cast<TypeList*>(type);

      GenericIterator srcBegin = tsrc->begin(value);
      GenericIterator srcEnd = tsrc->end(value);

      Type *pairType = (*result.end()).type;

      while (srcBegin != srcEnd)
      {
        std::pair<GenericValuePtr, bool> conv = (*srcBegin).convert(pairType);
        if (!conv.first.type)
        {
          result.destroy();
          return std::make_pair(GenericValuePtr(), false);
        }
        result._insert(conv.first[0], conv.first[1]);
        if (conv.second)
          conv.first.destroy();
        ++srcBegin;
      }
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(GenericValuePtr(), false);
  }

  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(Type* targetType) const
  {
    // qiLogDebug() << "convert " << type->infoString() << ' ' << targetType->infoString();
    /* Can have false-negative (same effective type, different Type instances
     * but we do not care, correct check (by comparing info() result
     * is more expensive than the dummy conversion that will happen.
     */
    if (type == targetType)
      return std::make_pair(*this, false);

    if (!targetType || !type) {
      qiLogWarning() << "Conversion error: can't convert to/from a null type.";
      return std::make_pair(GenericValuePtr(), false);
    }

    GenericValuePtr result;
    Type::Kind skind = type->kind();
    Type::Kind dkind = targetType->kind();

    if (skind == dkind)
    {
      switch(dkind)
      {
      case Type::Float:
        return convert(static_cast<TypeFloat*>(targetType));
      case Type::Int:
        return convert(static_cast<TypeInt*>(targetType));
      case Type::String:
        return convert(static_cast<TypeString*>(targetType));
      case Type::List:
        return convert(static_cast<TypeList*>(targetType));
      case Type::Map:
        return convert(static_cast<TypeMap*>(targetType));
      case Type::Pointer:
        return convert(static_cast<TypePointer*>(targetType));
      case Type::Tuple:
        return convert(static_cast<TypeTuple*>(targetType));
      case Type::Dynamic:
        return convert(static_cast<TypeDynamic*>(targetType));
      case Type::Raw:
        return convert(static_cast<TypeRaw*>(targetType));
      case Type::Unknown:
      {
        /* Under clang macos, typeInfo() comparison fails
           * for non-exported (not forced visibility=default since we default to hidden)
           * symbols. So ugly hack, compare the strings.
           */
        if (targetType->info() == type->info()
    #ifdef __clang__
            || targetType->info().asString() ==  type->info().asString()
    #endif
            )
          return std::make_pair(*this, false);
        else
          return std::make_pair(GenericValuePtr(), false);
      }
      default:
        break;
      } // switch
    } // skind == dkind
    if (skind == Type::List && dkind == Type::Tuple)
      return convert(static_cast<TypeTuple*>(targetType));
    else if (skind == Type::List && dkind == Type::Map)
      return convert(static_cast<TypeMap*>(targetType));
    else if (skind == Type::Float && dkind == Type::Int)
      return convert(static_cast<TypeInt*>(targetType));
    else if (skind == Type::Int && dkind == Type::Float)
      return convert(static_cast<TypeFloat*>(targetType));
    else if (skind == Type::String && dkind == Type::Raw)
      return convert(static_cast<TypeRaw*>(targetType));
    else if (skind == Type::Raw && dkind == Type::String)
      return convert(static_cast<TypeString*>(targetType));

    if (targetType->info() == typeOf<ObjectPtr>()->info()
        && type->kind() == Type::Pointer
        && static_cast<TypePointer*>(type)->pointedType()->kind() == Type::Object)
    { // Pointer to concrete object -> ObjectPtr
      // Keep a copy of this in ObjectPtr, and destroy on ObjectPtr destruction
      // That way if this is a shared_ptr, we link to it correctly
      TypePointer* pT = static_cast<TypePointer*>(type);
      ObjectPtr o(
            new GenericObject(
              static_cast<ObjectType*>(pT->pointedType()),
              pT->dereference(value).value),
            boost::bind(dropIt, GenericValue(*this)));
      return std::make_pair(GenericValueRef(o).clone(), true);
    }

    if (type->info() == typeOf<ObjectPtr>()->info()
        && targetType->kind() == Type::Pointer
        && static_cast<TypePointer*>(targetType)->pointedType()->kind() == Type::Object)
    { // Attempt specialized proxy conversion
      qiLogDebug() << "Attempting specialized proxy conversion";
      detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
      detail::ProxyGeneratorMap::iterator it = map.find(
                                                 static_cast<TypePointer*>(targetType)->pointedType()->info());
      if (it != map.end())
      {
        GenericValuePtr res = (it->second)(*(ObjectPtr*)value);
        return std::make_pair(res, true);
      }
      else
        qiLogDebug() << "type "
                     << static_cast<TypePointer*>(targetType)->pointedType()->infoString()
                     <<" not found in proxy map";
    }

    if (targetType->kind() == Type::Dynamic)
      return convert(static_cast<TypeDynamic*>(targetType));

    if (type->kind() == Type::Dynamic)
    {
      GenericValuePtr gv = asDynamic();
      std::pair<GenericValuePtr, bool> result = gv.convert(targetType);
      return result;
    }

    if (skind == Type::Object && dkind == Type::Pointer)
      return convert(static_cast<TypePointer*>(targetType));

    if (skind == Type::Object)
    {
      // Try inheritance
      ObjectType* osrc = static_cast<ObjectType*>(type);
      qiLogDebug() << "inheritance check "
        << osrc <<" " << (osrc?osrc->inherits(targetType):false);
      int inheritOffset = 0;
      if (osrc && (inheritOffset =  osrc->inherits(targetType)) != -1)
      {
        // We return a Value that point to the same data as this.
        result.type = targetType;
        result.value = (void*)((long)value + inheritOffset);
        return std::make_pair(result, false);
      }
    }

    if (type->info() == targetType->info())
      return std::make_pair(*this, false);

    return std::make_pair(GenericValuePtr(), false);
  }

  GenericValuePtr GenericValuePtr::convertCopy(Type* targetType) const
  {
    std::pair<GenericValuePtr, bool> res = convert(targetType);
    if (res.second)
      return res.first;
    else
      return res.first.clone();
  }

  bool operator< (const GenericValuePtr& a, const GenericValuePtr& b)
  {
    //qiLogDebug() << "Compare " << a.type << ' ' << b.type;
#define GET(v, t) static_cast<Type ## t *>(v.type)->get(v.value)
    if (!a.type)
      return b.type != 0;
    if (!b.type)
      return false;
    /* < operator for char* does not do what we want, so force
    * usage of get() below for string types.
    */
    if ((a.type == b.type || a.type->info() == b.type->info())
        && a.type->kind() != Type::String)
    {
      //qiLogDebug() << "Compare sametype " << a.type->infoString();
      return a.type->less(a.value, b.value);
    }
    // Comparing values of different types
    Type::Kind ka = a.type->kind();
    Type::Kind kb = b.type->kind();
    //qiLogDebug() << "Compare " << ka << ' ' << kb;
    if (ka != kb)
    {
      if (ka == Type::Int && kb == Type::Float)
        return GET(a, Int) < GET(b, Float);
      else if (ka == Type::Float && kb == Type::Int)
        return GET(a, Float) < GET(b, Int);
      else
        return ka < kb; // Safer than comparing pointers
    }
    else switch(ka)
    {
    case Type::Void:
      return false;
    case Type::Int:
      return GET(a, Int) < GET(b, Int);
    case Type::Float:
      return GET(a, Float) < GET(b, Float);
    case Type::String:
    {
      std::pair<char*, size_t> ca, cb;
      ca = GET(a, String);
      cb = GET(b, String);
      bool res = ca.second == cb.second?
                   (memcmp(ca.first, cb.first, ca.second) < 0) : (ca.second < cb.second);
      qiLogDebug() << "Compare " << ca.first << ' ' << cb.first << ' ' << res;
      return res;
    }
    case Type::List:
    case Type::Map: // omg, same code!
    {
      size_t la = a.size();
      size_t lb = b.size();
      if (la != lb)
        return la < lb;
      GenericIterator ita   = a.begin();
      GenericIterator enda = a.end();
      GenericIterator itb   = b.begin();
      GenericIterator endb = b.end();
      while (ita != enda)
      {
        assert (! (itb == endb));
        GenericValuePtr ea = *ita;
        GenericValuePtr eb = *itb;
        if (ea < eb)
          return true;
        else if (eb < ea)
          return false;
        ++ita;
        ++itb;
      }
      return false; // list are equals
    }
    case Type::Object:
    case Type::Pointer:
    case Type::Tuple:
    case Type::Dynamic:
    case Type::Raw:
    case Type::Unknown:
    case Type::Iterator:
      return a.value < b.value;
    }
#undef GET
    return a.value < b.value;
  }
  bool operator< (const GenericValue& a, const GenericValue& b)
  {
    return (const GenericValuePtr&)a < (const GenericValuePtr&)b;
  }

  bool operator==(const GenericValuePtr& a, const GenericValuePtr& b)
  {
    if (a.kind() == Type::Iterator && b.kind() == Type::Iterator
        && a.type->info() == b.type->info())
    {
      return static_cast<TypeIterator*>(a.type)->equals(a.value, b.value);
    }
    else
      return ! (a < b) && !(b<a);
  }

  bool operator==(const GenericValue& a, const GenericValue& b)
  {
    return (const GenericValuePtr&)a == (const GenericValuePtr&)b;
  }

  bool operator==(const GenericIterator& a, const GenericIterator& b)
  {
    return (const GenericValuePtr&)a == (const GenericValuePtr&)b;
  }

  GenericValue GenericValuePtr::toTuple(bool homogeneous) const
  {
    if (kind() == Type::Tuple)
      return GenericValue(*this);
    else if (kind() != Type::List)
      throw std::runtime_error("Expected Tuple or List kind");
    // convert list to tuple

    TypeList* t = static_cast<TypeList*>(type);
    Type* te = t->elementType();
    TypeDynamic* td = 0;
    if (te->kind() == Type::Dynamic)
      td = static_cast<TypeDynamic*>(te);
    if (!homogeneous && !td)
      throw std::runtime_error("Element type is not dynamic");
    std::vector<GenericValuePtr> elems;
    GenericIterator it = begin();
    GenericIterator iend = end();
    while (it != iend)
    {
      GenericValuePtr e = *it;
      if (homogeneous)
        elems.push_back(e);
      else
        elems.push_back(e.asDynamic());
      ++it;
    }

    //makeGenericTuple allocates, steal the result
    return GenericValue(makeGenericTuple(elems), false, true);
  }

  ObjectPtr GenericValuePtr::toObject() const
  {
    return to<ObjectPtr>();
  }

  GenericValuePtr GenericValuePtr::_element(const GenericValuePtr& key, bool throwOnFailure)
  {
    if (kind() == Type::List)
    {
      TypeList* t = static_cast<TypeList*>(type);
      int ikey = (int)key.toInt();
      if (ikey < 0 || static_cast<size_t>(ikey) >= t->size(value))
      {
        if (throwOnFailure)
          throw std::runtime_error("Index out of range");
        else
          return GenericValuePtr();
      }
      return GenericValuePtr(t->elementType(), t->element(value, ikey));
    }
    else if (kind() == Type::Map)
    {
      TypeMap* t = static_cast<TypeMap*>(type);
      std::pair<GenericValuePtr, bool> c = key.convert(t->keyType());
      if (!c.first.type)
        throw std::runtime_error("Incompatible key type");
      // HACK: should be two separate booleans
      bool autoInsert = throwOnFailure;
      GenericValuePtr result
          = t->element(&value, c.first.value, autoInsert);
      if (c.second)
        c.first.destroy();
      return result;
    }
    else if (kind() == Type::Tuple)
    {
      TypeTuple* t = static_cast<TypeTuple*>(type);
      int ikey = (int)key.toInt();
      std::vector<Type*> types = t->memberTypes();
      if (ikey < 0 || static_cast<size_t>(ikey) >= types.size())
      {
        if (throwOnFailure)
          throw std::runtime_error("Index out of range");
        else
          return GenericValuePtr();
      }
      return GenericValuePtr(types[ikey], t->get(value, ikey));
    }
    else
      throw std::runtime_error("Expected List, Map or Tuple kind");
  }

  void GenericValuePtr::_append(const GenericValuePtr& elem)
  {
    if (kind() != Type::List)
      throw std::runtime_error("Expected a list");
    TypeList* t = static_cast<TypeList*>(type);
    std::pair<GenericValuePtr, bool> c = elem.convert(t->elementType());
    t->pushBack(&value, c.first.value);
    if (c.second)
      c.first.destroy();
  }

  void GenericValuePtr::_insert(const GenericValuePtr& key, const GenericValuePtr& val)
  {
    if (kind() != Type::Map)
      throw std::runtime_error("Expected a map");
    std::pair<GenericValuePtr, bool> ck(key, false);
    std::pair<GenericValuePtr, bool> cv(val, false);
    TypeMap* t = static_cast<TypeMap*>(type);
    if (key.type != t->keyType())
      ck = key.convert(t->keyType());
    if (val.type != t->elementType())
      cv = val.convert(t->elementType());
    t->insert(&value, ck.first.value, cv.first.value);
    if (ck.second)
      ck.first.destroy();
    if (cv.second)
      cv.first.destroy();
  }

  void GenericValuePtr::update(const GenericValuePtr& val)
  {
    switch(kind())
    {
    case Type::Int:
      setInt(val.toInt());
      break;
    case Type::Float:
      setDouble(val.toDouble());
      break;
    case Type::String:
      setString(val.toString());
      break;
    case Type::Dynamic:
      setDynamic(val);
    default:
      throw std::runtime_error("Update not implemented for this type.");
    }
  }

  void GenericValuePtr::setInt(int64_t v)
  {
    if (kind() == Type::Int)
    {
      TypeInt* type = static_cast<TypeInt*>(this->type);
      if (!type->isSigned() && v < 0)
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Converting negative value %s to unsigned type", v));
      // not signed gives us an extra bit, but signed can go down an extra value
      if (type->size() > 8)
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to %s bytes", v, type->size()));
      if (type->size() == 0 && (v < 0 || v > 1))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Expected 0 or 1 when converting to bool, got %s", v));
      if (type->size() > 0 && type->size() < 8 && (std::abs(v) >= (1LL << (8*type->size() - (type->isSigned()?1:0))) + ((v<0)?1:0)))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to %s bytes", v, type->size()));
      type->set(&value, v);
    }
    else if (kind() == Type::Float)
      static_cast<TypeFloat*>(type)->set(&value,
        static_cast<double>(v));
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  void GenericValuePtr::setDynamic(const qi::GenericValuePtr &element) {
    if (kind() != Type::Dynamic)
      throw std::runtime_error("Value is not a Dynamic");
    TypeDynamic* t = static_cast<TypeDynamic*>(this->type);
    t->set(&value, element);
  }

  void GenericValuePtr::setUInt(uint64_t v)
  {
    if (kind() == Type::Int)
    {
      TypeInt* type = static_cast<TypeInt*>(this->type);
      if (type->size() > 0 && type->size() < 8 && (v >= (1ULL << (8*type->size() - (type->isSigned()?1:0)))))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to %s bytes", v, type->size()));
      if (type->size() == 0 && (v > 1))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Expected 0 or 1 when converting to bool, got %s", v));
      if (type->size() == 8 && type->isSigned() && v >= 0x8000000000000000ULL)
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to signed int64", v));
      type->set(&value, (int64_t)v);
    }
    else if (kind() == Type::Float)
      static_cast<TypeFloat*>(type)->set(&value,
        static_cast<double>(static_cast<uint64_t>(v)));
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  void GenericValuePtr::setDouble(double v)
  {
    if (kind() == Type::Float)
      static_cast<TypeFloat*>(type)->set(&value, v);
    else if (kind() == Type::Int)
    {
      TypeInt* type = static_cast<TypeInt*>(this->type);
      if (v < 0 && !type->isSigned())
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Converting negative value %s to unsigned type", v));
      if (type->size() == 0 && std::min(std::abs(v), std::abs(v-1)) > 0.01)
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Expected 0 or 1 when converting to bool, got %s", v));
      if (type->size() != 0 && type->size() < 8 && (std::abs(v) >= (1ULL << (8*type->size() - (type->isSigned()?1:0))) + ((v<0)?1:0)))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to %s bytes", v, type->size()));
      if (type->size() == 8
          && std::abs(v) > (type->isSigned()?
            (double)std::numeric_limits<int64_t>::max()
            :(double)std::numeric_limits<uint64_t>::max()))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to %s bytes", v, type->size()));
      type->set(&value, static_cast<int64_t>(v));
    }
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  namespace detail
  {
    QI_NORETURN void throwConversionFailure(Type* from, Type* to)
    {
      std::stringstream msg;
      msg << "Conversion from ";
      if (from) {
        msg << from->infoString() << '(' << from->kind() << ')';
      } else {
        msg << "NULL Type";
      }
      msg << " to ";
      if (to) {
        msg << to->infoString() << '(' << to->kind() << ")";
      } else {
        msg << "NULL Type";
      }
      msg << " failed";
      qiLogWarning() << msg.str();
      throw std::runtime_error(msg.str());
    }
  }
}

