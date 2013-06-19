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


  std::pair<AnyReference, bool> AnyReference::convert(DynamicTypeInterface* targetType) const
  {
    AnyReference result;

    result.type = targetType;
    result.value = targetType->initializeStorage();
    static_cast<DynamicTypeInterface*>(targetType)->set(&result.value, *this);
    return std::make_pair(result, true);
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReference::convert(PointerTypeInterface* targetType) const
  {
    AnyReference result;

    switch (type->kind())
    {
    case TypeInterface::Pointer:
    {
      TypeInterface* srcPointedType = static_cast<PointerTypeInterface*>(type)->pointedType();
      TypeInterface* dstPointedType = static_cast<PointerTypeInterface*>(targetType)->pointedType();
      // We only try to handle conversion for pointer to objects
      if (srcPointedType->kind() != TypeInterface::Object || dstPointedType->kind() != TypeInterface::Object)
      {
        // However, we need the full check for exact match here
        if (type->info() == targetType->info())
          return std::make_pair(*this, false);
        else
        {
          qiLogDebug() << "Conversion between non-object pointers not supported";
          return std::make_pair(AnyReference(), false);
        }
      }
      AnyReference pointedSrc = static_cast<PointerTypeInterface*>(type)->dereference(value);
      // try object conversion (inheritance)
      std::pair<AnyReference, bool> pointedDstPair = pointedSrc.convert(dstPointedType);
      if (!pointedDstPair.first.type)
      {
        qiLogDebug() << "Attempting object->proxy conversion";
        // try object->proxy conversion by simply rewrapping this
        AnyObject o(new GenericObject(static_cast<ObjectTypeInterface*>(pointedSrc.type), pointedSrc.value));
        return AnyReference(o).convert((TypeInterface*)targetType);
      }
      if (pointedDstPair.second)
        qiLogError() << "assertion error, allocated converted reference";
      // We must re-reference
      AnyReference pointedDst = pointedDstPair.first;
      void* ptr = pointedDst.type->ptrFromStorage(&pointedDst.value);
      result = AnyReference((TypeInterface*)targetType);
      targetType->setPointee(&result.value, ptr);
      return std::make_pair(result, false);
    }
    case TypeInterface::Object:
    {
      std::pair<AnyReference, bool> gv = convert(
                                              static_cast<PointerTypeInterface*>(targetType)->pointedType());
      if (!gv.first.type)
        return gv;
      // Re-pointerise it
      void* ptr = gv.first.type->ptrFromStorage(&gv.first.value);
      AnyReference result;
      result.type = targetType;
      result.value = targetType->initializeStorage(&ptr);
      return std::make_pair(result, false);
    }
    default:
      break;
    }
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReference::convert(ListTypeInterface* targetType) const
  {
    AnyReference result;

    switch (type->kind())
    {
    case TypeInterface::List:
    {
      ListTypeInterface* targetListType = static_cast<ListTypeInterface*>(targetType);
      ListTypeInterface* sourceListType = static_cast<ListTypeInterface*>(type);

      TypeInterface* srcElemType = sourceListType->elementType();
      TypeInterface* dstElemType = targetListType->elementType();
      bool needConvert = (srcElemType->info() != dstElemType->info());
      result = AnyReference((TypeInterface*)targetListType);

      GenericIterator iend = end();
      for (GenericIterator it = begin(); it!= iend; ++it)
      {
        AnyReference val = *it;
        if (!needConvert)
          result._append(val);
        else
        {
          std::pair<AnyReference,bool> c = val.convert(dstElemType);
          if (!c.first.type)
          {
            qiLogDebug() << "List element conversion failure from "
                         << val.type->infoString() << " to " << dstElemType->infoString();
            result.destroy();
            return std::make_pair(AnyReference(), false);
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
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReference::convert(StringTypeInterface* targetType) const
  {
    AnyReference result;

    switch (type->kind())
    {
    case TypeInterface::String:
    {
      if (targetType->info() == type->info())
        return std::make_pair(*this, false);
      result.type = targetType;
      result.value = targetType->initializeStorage();
      std::pair<char*, size_t> v = static_cast<StringTypeInterface*>(type)->get(value);
      static_cast<StringTypeInterface*>(targetType)->set(&result.value,
                                                v.first, v.second);
      return std::make_pair(result, true);
    }
    case TypeInterface::Raw:
    {
      qiLogWarning() << "Conversion attempt from raw to string";
      return std::make_pair(AnyReference(), false);
    }
    default:
      break;
    }
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReference::convert(RawTypeInterface* targetType) const
  {
    AnyReference result;

    switch (type->kind())
    {
    case TypeInterface::Raw:
    {
      if (targetType->info() == type->info())
        return std::make_pair(*this, false);
      result.type = targetType;
      result.value = targetType->initializeStorage();
      std::pair<char*, size_t> v = static_cast<RawTypeInterface*>(type)->get(value);
      static_cast<RawTypeInterface*>(result.type)->set(&result.value, v.first, v.second);
      return std::make_pair(result, true);
    }
    case TypeInterface::String:
    {
      std::pair<char*, size_t> data = static_cast<StringTypeInterface*>(type)->get(value);
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<RawTypeInterface*>(result.type)->set(&result.value, data.first, data.second);
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReference::convert(FloatTypeInterface* targetType) const
  {
    AnyReference result;

    switch (type->kind())
    {
    case TypeInterface::Float:
    {
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<FloatTypeInterface*>(targetType)->set(&result.value,
                                               static_cast<FloatTypeInterface*>(type)->get(value));
      return std::make_pair(result, true);
    }
    case TypeInterface::Int:
    {
      AnyReference result(static_cast<TypeInterface*>(targetType));
      int64_t v = static_cast<IntTypeInterface*>(type)->get(value);
      if (static_cast<IntTypeInterface*>(type)->isSigned())
        result.setInt(v);
      else
        result.setUInt((uint64_t)v);
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReference::convert(IntTypeInterface* targetType) const
  {
    AnyReference result;

    switch (type->kind())
    {
    case TypeInterface::Int:
    {
      IntTypeInterface* tsrc = static_cast<IntTypeInterface*>(type);
      IntTypeInterface* tdst = static_cast<IntTypeInterface*>(targetType);
      int64_t v = tsrc->get(value);
      /* Bounce to GVP to perform overflow checks
      */
      AnyReference result((TypeInterface*)tdst);
      if (tsrc->isSigned())
        result.setInt(v);
      else
        result.setUInt((uint64_t)v);
      tdst->set(&result.value, v);
      return std::make_pair(result, true);
    }
    case TypeInterface::Float:
    {
      double v = static_cast<FloatTypeInterface*>(type)->get(value);
      IntTypeInterface* tdst = static_cast<IntTypeInterface*>(targetType);
      AnyReference result((TypeInterface*)tdst);
      // bounce to setDouble for overflow check
      result.setDouble(v);
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReference::convert(StructTypeInterface* targetType) const
  {
    AnyReference result;
    StructTypeInterface* tdst = targetType;

    switch (type->kind())
    {
    case TypeInterface::Tuple:
    {
      StructTypeInterface* tsrc = static_cast<StructTypeInterface*>(type);
      std::vector<void*> sourceData = tsrc->get(value);
      std::vector<TypeInterface*> srcTypes = tsrc->memberTypes();
      std::vector<TypeInterface*> dstTypes = tdst->memberTypes();
      if (dstTypes.size() != sourceData.size())
      {
        qiLogWarning() << "Conversion failure: tuple size mismatch between " << tsrc->signature().toString() << " and " << tdst->signature().toString();
        return std::make_pair(AnyReference(), false);
      }

      std::vector<void*> targetData;
      std::vector<bool> mustDestroy;
      for (unsigned i=0; i<dstTypes.size(); ++i)
      {
        std::pair<AnyReference, bool> conv = AnyReference(srcTypes[i], sourceData[i]).convert(dstTypes[i]);
        if (!conv.first.type)
        {
          qiLogWarning() << "Conversion failure in tuple member between "
                         << srcTypes[i]->infoString() << " and " << dstTypes[i]->infoString();
          return std::make_pair(AnyReference(), false);
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
    case TypeInterface::List:
    {
      // No explicit type-check, convert will do it
      // handles for instance [i] -> (iii) if size matches
      // or [m] -> (anything) if effective types match
      ListTypeInterface* tsrc = static_cast<ListTypeInterface*>(type);

      GenericIterator srcBegin = tsrc->begin(value);
      GenericIterator srcEnd = tsrc->end(value);

      std::vector<TypeInterface*> dstTypes = tdst->memberTypes();
      std::vector<void*> targetData;
      targetData.reserve(dstTypes.size());
      std::vector<bool> mustDestroy;
      mustDestroy.reserve(dstTypes.size());

      if (this->size() != dstTypes.size())
      {
        qiLogWarning() << "Conversion failure: containers size mismatch between "
                       << tsrc->signature().toString() << " and " << tdst->signature().toString();
        return std::make_pair(AnyReference(), false);
      }

      unsigned int i = 0;
      while (srcBegin != srcEnd)
      {
        std::pair<AnyReference, bool> conv = (*srcBegin).convert(dstTypes[i]);
        if (!conv.first.type)
        {
          for (unsigned u = 0; u < targetData.size(); ++u)
            if (mustDestroy[u])
              dstTypes[u]->destroy(targetData[u]);
          return std::make_pair(AnyReference(), false);
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
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReference::convert(MapTypeInterface* targetType) const
  {
    AnyReference result;

    switch (type->kind())
    {
    case TypeInterface::Map:
    {
      result = AnyReference(static_cast<TypeInterface*>(targetType));

      MapTypeInterface* targetMapType = static_cast<MapTypeInterface*>(targetType);
      MapTypeInterface* srcMapType = static_cast<MapTypeInterface*>(type);

      TypeInterface* srcKeyType = srcMapType->keyType();
      TypeInterface* srcElementType = srcMapType->elementType();

      TypeInterface* targetKeyType = targetMapType->keyType();
      TypeInterface* targetElementType = targetMapType->elementType();

      bool sameKey = srcKeyType->info() == targetKeyType->info();
      bool sameElem = srcElementType->info() == targetElementType->info();

      GenericIterator iend = end();
      for (GenericIterator it = begin(); it != iend; ++it)
      {
        std::pair<AnyReference, bool> ck, cv;
        AnyReference kv = *it;
        if (!sameKey)
        {
          ck = kv[0].convert(targetKeyType);
          if (!ck.first.type)
            return std::make_pair(AnyReference(), false);
        }
        if (!sameElem)
        {
          cv = kv[1].convert(targetElementType);
          if (!cv.first.type)
            return std::make_pair(AnyReference(), false);
        }
        result._insert(sameKey?kv[0]:ck.first, sameElem?kv[1]:cv.first);
        if (!sameKey && ck.second)
          ck.first.destroy();
        if (!sameElem && cv.second)
          cv.first.destroy();
      }
      return std::make_pair(result, true);
    }
    case TypeInterface::List:
    {
      // Accept [(kv)] and convert to {kv}
      // Also accept [[m]] , [[k]] if k=v and size match, and other compatible stuffs
      result = AnyReference(static_cast<TypeInterface*>(targetType));
      ListTypeInterface* tsrc = static_cast<ListTypeInterface*>(type);

      GenericIterator srcBegin = tsrc->begin(value);
      GenericIterator srcEnd = tsrc->end(value);

      TypeInterface *pairType = (*result.end()).type;

      while (srcBegin != srcEnd)
      {
        std::pair<AnyReference, bool> conv = (*srcBegin).convert(pairType);
        if (!conv.first.type)
        {
          result.destroy();
          return std::make_pair(AnyReference(), false);
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
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReference::convert(TypeInterface* targetType) const
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
      return std::make_pair(AnyReference(), false);
    }

    AnyReference result;
    TypeInterface::Kind skind = type->kind();
    TypeInterface::Kind dkind = targetType->kind();

    if (skind == dkind)
    {
      switch(dkind)
      {
      case TypeInterface::Float:
        return convert(static_cast<FloatTypeInterface*>(targetType));
      case TypeInterface::Int:
        return convert(static_cast<IntTypeInterface*>(targetType));
      case TypeInterface::String:
        return convert(static_cast<StringTypeInterface*>(targetType));
      case TypeInterface::List:
        return convert(static_cast<ListTypeInterface*>(targetType));
      case TypeInterface::Map:
        return convert(static_cast<MapTypeInterface*>(targetType));
      case TypeInterface::Pointer:
        return convert(static_cast<PointerTypeInterface*>(targetType));
      case TypeInterface::Tuple:
        return convert(static_cast<StructTypeInterface*>(targetType));
      case TypeInterface::Dynamic:
        return convert(static_cast<DynamicTypeInterface*>(targetType));
      case TypeInterface::Raw:
        return convert(static_cast<RawTypeInterface*>(targetType));
      case TypeInterface::Unknown:
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
          return std::make_pair(AnyReference(), false);
      }
      default:
        break;
      } // switch
    } // skind == dkind
    if (skind == TypeInterface::List && dkind == TypeInterface::Tuple)
      return convert(static_cast<StructTypeInterface*>(targetType));
    else if (skind == TypeInterface::List && dkind == TypeInterface::Map)
      return convert(static_cast<MapTypeInterface*>(targetType));
    else if (skind == TypeInterface::Float && dkind == TypeInterface::Int)
      return convert(static_cast<IntTypeInterface*>(targetType));
    else if (skind == TypeInterface::Int && dkind == TypeInterface::Float)
      return convert(static_cast<FloatTypeInterface*>(targetType));
    else if (skind == TypeInterface::String && dkind == TypeInterface::Raw)
      return convert(static_cast<RawTypeInterface*>(targetType));
    else if (skind == TypeInterface::Raw && dkind == TypeInterface::String)
      return convert(static_cast<StringTypeInterface*>(targetType));

    if (targetType->info() == typeOf<AnyObject>()->info()
        && type->kind() == TypeInterface::Pointer
        && static_cast<PointerTypeInterface*>(type)->pointedType()->kind() == TypeInterface::Object)
    { // Pointer to concrete object -> AnyObject
      // Keep a copy of this in AnyObject, and destroy on AnyObject destruction
      // That way if this is a shared_ptr, we link to it correctly
      PointerTypeInterface* pT = static_cast<PointerTypeInterface*>(type);
      AnyObject o(
            new GenericObject(
              static_cast<ObjectTypeInterface*>(pT->pointedType()),
              pT->dereference(value).value),
            boost::bind(dropIt, GenericValue(*this)));
      return std::make_pair(AnyReference(o).clone(), true);
    }

    if (type->info() == typeOf<AnyObject>()->info()
        && targetType->kind() == TypeInterface::Pointer)
    { // Attempt specialized proxy conversion
      qiLogDebug() << "Attempting specialized proxy conversion";
      detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
      detail::ProxyGeneratorMap::iterator it = map.find(
                                                 static_cast<PointerTypeInterface*>(targetType)->pointedType()->info());
      if (it != map.end())
      {
        AnyReference res = (it->second)(*(AnyObject*)value);
        return std::make_pair(res, true);
      }
      else
        qiLogDebug() << "type "
                     << static_cast<PointerTypeInterface*>(targetType)->pointedType()->infoString()
                     <<" not found in proxy map";
    }

    if (targetType->kind() == TypeInterface::Dynamic)
      return convert(static_cast<DynamicTypeInterface*>(targetType));

    if (type->kind() == TypeInterface::Dynamic)
    {
      AnyReference gv = asDynamic();
      std::pair<AnyReference, bool> result = gv.convert(targetType);
      return result;
    }

    if (skind == TypeInterface::Object && dkind == TypeInterface::Pointer)
      return convert(static_cast<PointerTypeInterface*>(targetType));

    if (skind == TypeInterface::Object)
    {
      // Try inheritance
      ObjectTypeInterface* osrc = static_cast<ObjectTypeInterface*>(type);
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

    return std::make_pair(AnyReference(), false);
  }

  AnyReference AnyReference::convertCopy(TypeInterface* targetType) const
  {
    std::pair<AnyReference, bool> res = convert(targetType);
    if (res.second)
      return res.first;
    else
      return res.first.clone();
  }

  bool operator< (const AnyReference& a, const AnyReference& b)
  {
    //qiLogDebug() << "Compare " << a.type << ' ' << b.type;
#define GET(v, t) static_cast< t ## TypeInterface *>(v.type)->get(v.value)
    if (!a.type)
      return b.type != 0;
    if (!b.type)
      return false;
    /* < operator for char* does not do what we want, so force
    * usage of get() below for string types.
    */
    if ((a.type == b.type || a.type->info() == b.type->info())
        && a.type->kind() != TypeInterface::String)
    {
      //qiLogDebug() << "Compare sametype " << a.type->infoString();
      return a.type->less(a.value, b.value);
    }
    // Comparing values of different types
    TypeInterface::Kind ka = a.type->kind();
    TypeInterface::Kind kb = b.type->kind();
    //qiLogDebug() << "Compare " << ka << ' ' << kb;
    if (ka != kb)
    {
      if (ka == TypeInterface::Int && kb == TypeInterface::Float)
        return GET(a, Int) < GET(b, Float);
      else if (ka == TypeInterface::Float && kb == TypeInterface::Int)
        return GET(a, Float) < GET(b, Int);
      else
        return ka < kb; // Safer than comparing pointers
    }
    else switch(ka)
    {
    case TypeInterface::Void:
      return false;
    case TypeInterface::Int:
      return GET(a, Int) < GET(b, Int);
    case TypeInterface::Float:
      return GET(a, Float) < GET(b, Float);
    case TypeInterface::String:
    {
      std::pair<char*, size_t> ca, cb;
      ca = GET(a, String);
      cb = GET(b, String);
      bool res = ca.second == cb.second?
                   (memcmp(ca.first, cb.first, ca.second) < 0) : (ca.second < cb.second);
      qiLogDebug() << "Compare " << ca.first << ' ' << cb.first << ' ' << res;
      return res;
    }
    case TypeInterface::List:
    case TypeInterface::Map: // omg, same code!
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
        AnyReference ea = *ita;
        AnyReference eb = *itb;
        if (ea < eb)
          return true;
        else if (eb < ea)
          return false;
        ++ita;
        ++itb;
      }
      return false; // list are equals
    }
    case TypeInterface::Object:
    case TypeInterface::Pointer:
    case TypeInterface::Tuple:
    case TypeInterface::Dynamic:
    case TypeInterface::Raw:
    case TypeInterface::Unknown:
    case TypeInterface::Iterator:
      return a.value < b.value;
    }
#undef GET
    return a.value < b.value;
  }
  bool operator< (const GenericValue& a, const GenericValue& b)
  {
    return (const AnyReference&)a < (const AnyReference&)b;
  }

  bool operator==(const AnyReference& a, const AnyReference& b)
  {
    if (a.kind() == TypeInterface::Iterator && b.kind() == TypeInterface::Iterator
        && a.type->info() == b.type->info())
    {
      return static_cast<IteratorTypeInterface*>(a.type)->equals(a.value, b.value);
    }
    else
      return ! (a < b) && !(b<a);
  }

  bool operator==(const GenericValue& a, const GenericValue& b)
  {
    return (const AnyReference&)a == (const AnyReference&)b;
  }

  bool operator==(const GenericIterator& a, const GenericIterator& b)
  {
    return (const AnyReference&)a == (const AnyReference&)b;
  }

  GenericValue AnyReference::toTuple(bool homogeneous) const
  {
    if (kind() == TypeInterface::Tuple)
      return GenericValue(*this);
    else if (kind() != TypeInterface::List)
      throw std::runtime_error("Expected Tuple or List kind");
    // convert list to tuple

    ListTypeInterface* t = static_cast<ListTypeInterface*>(type);
    TypeInterface* te = t->elementType();
    DynamicTypeInterface* td = 0;
    if (te->kind() == TypeInterface::Dynamic)
      td = static_cast<DynamicTypeInterface*>(te);
    if (!homogeneous && !td)
      throw std::runtime_error("Element type is not dynamic");
    std::vector<AnyReference> elems;
    GenericIterator it = begin();
    GenericIterator iend = end();
    while (it != iend)
    {
      AnyReference e = *it;
      if (homogeneous)
        elems.push_back(e);
      else
        elems.push_back(e.asDynamic());
      ++it;
    }

    //makeGenericTuple allocates, steal the result
    return GenericValue(makeGenericTuple(elems), false, true);
  }

  AnyObject AnyReference::toObject() const
  {
    return to<AnyObject>();
  }

  AnyReference AnyReference::_element(const AnyReference& key, bool throwOnFailure)
  {
    if (kind() == TypeInterface::List)
    {
      ListTypeInterface* t = static_cast<ListTypeInterface*>(type);
      int ikey = (int)key.toInt();
      if (ikey < 0 || static_cast<size_t>(ikey) >= t->size(value))
      {
        if (throwOnFailure)
          throw std::runtime_error("Index out of range");
        else
          return AnyReference();
      }
      return AnyReference(t->elementType(), t->element(value, ikey));
    }
    else if (kind() == TypeInterface::Map)
    {
      MapTypeInterface* t = static_cast<MapTypeInterface*>(type);
      std::pair<AnyReference, bool> c = key.convert(t->keyType());
      if (!c.first.type)
        throw std::runtime_error("Incompatible key type");
      // HACK: should be two separate booleans
      bool autoInsert = throwOnFailure;
      AnyReference result
          = t->element(&value, c.first.value, autoInsert);
      if (c.second)
        c.first.destroy();
      return result;
    }
    else if (kind() == TypeInterface::Tuple)
    {
      StructTypeInterface* t = static_cast<StructTypeInterface*>(type);
      int ikey = (int)key.toInt();
      std::vector<TypeInterface*> types = t->memberTypes();
      if (ikey < 0 || static_cast<size_t>(ikey) >= types.size())
      {
        if (throwOnFailure)
          throw std::runtime_error("Index out of range");
        else
          return AnyReference();
      }
      return AnyReference(types[ikey], t->get(value, ikey));
    }
    else
      throw std::runtime_error("Expected List, Map or Tuple kind");
  }

  void AnyReference::_append(const AnyReference& elem)
  {
    if (kind() != TypeInterface::List)
      throw std::runtime_error("Expected a list");
    ListTypeInterface* t = static_cast<ListTypeInterface*>(type);
    std::pair<AnyReference, bool> c = elem.convert(t->elementType());
    t->pushBack(&value, c.first.value);
    if (c.second)
      c.first.destroy();
  }

  void AnyReference::_insert(const AnyReference& key, const AnyReference& val)
  {
    if (kind() != TypeInterface::Map)
      throw std::runtime_error("Expected a map");
    std::pair<AnyReference, bool> ck(key, false);
    std::pair<AnyReference, bool> cv(val, false);
    MapTypeInterface* t = static_cast<MapTypeInterface*>(type);
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

  void AnyReference::update(const AnyReference& val)
  {
    switch(kind())
    {
    case TypeInterface::Int:
      setInt(val.toInt());
      break;
    case TypeInterface::Float:
      setDouble(val.toDouble());
      break;
    case TypeInterface::String:
      setString(val.toString());
      break;
    case TypeInterface::Dynamic:
      setDynamic(val);
      break;
    default:
      throw std::runtime_error("Update not implemented for this type.");
    }
  }

  void AnyReference::setInt(int64_t v)
  {
    if (kind() == TypeInterface::Int)
    {
      IntTypeInterface* type = static_cast<IntTypeInterface*>(this->type);
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
    else if (kind() == TypeInterface::Float)
      static_cast<FloatTypeInterface*>(type)->set(&value,
        static_cast<double>(v));
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  void AnyReference::setDynamic(const qi::AnyReference &element) {
    if (kind() != TypeInterface::Dynamic)
      throw std::runtime_error("Value is not a Dynamic");
    DynamicTypeInterface* t = static_cast<DynamicTypeInterface*>(this->type);
    t->set(&value, element);
  }

  void AnyReference::setUInt(uint64_t v)
  {
    if (kind() == TypeInterface::Int)
    {
      IntTypeInterface* type = static_cast<IntTypeInterface*>(this->type);
      if (type->size() > 0 && type->size() < 8 && (v >= (1ULL << (8*type->size() - (type->isSigned()?1:0)))))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to %s bytes", v, type->size()));
      if (type->size() == 0 && (v > 1))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Expected 0 or 1 when converting to bool, got %s", v));
      if (type->size() == 8 && type->isSigned() && v >= 0x8000000000000000ULL)
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to signed int64", v));
      type->set(&value, (int64_t)v);
    }
    else if (kind() == TypeInterface::Float)
      static_cast<FloatTypeInterface*>(type)->set(&value,
        static_cast<double>(static_cast<uint64_t>(v)));
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  void AnyReference::setDouble(double v)
  {
    if (kind() == TypeInterface::Float)
      static_cast<FloatTypeInterface*>(type)->set(&value, v);
    else if (kind() == TypeInterface::Int)
    {
      IntTypeInterface* type = static_cast<IntTypeInterface*>(this->type);
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
    QI_NORETURN void throwConversionFailure(TypeInterface* from, TypeInterface* to)
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

