/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/lexical_cast.hpp>

#include <qitype/anyreference.hpp>
#include <qitype/anyobject.hpp>

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
    static void dropIt(GenericObject* ptr, const AnyValue& v)
    {
      qiLogDebug() << "dropIt";
      delete ptr;
    }
  }


  std::pair<AnyReference, bool> AnyReferenceBase::convert(DynamicTypeInterface* targetType) const
  {
    // same-type check (not done before, useful mainly for AnyObject)
    if (targetType->info() == type()->info())
      return std::make_pair(*this, false);
    AnyReference result;

    result._type = targetType;
    result._value = targetType->initializeStorage();
    static_cast<DynamicTypeInterface*>(targetType)->set(&result._value, AnyReference(*this));
    return std::make_pair(result, true);
  }

  std::pair<AnyReference, bool> AnyReferenceBase::convert(PointerTypeInterface* targetType) const
  {
    AnyReference result;

    switch (_type->kind())
    {
    case TypeKind_Pointer:
    {
      TypeInterface* srcPointedType = static_cast<PointerTypeInterface*>(_type)->pointedType();
      TypeInterface* dstPointedType = static_cast<PointerTypeInterface*>(targetType)->pointedType();
      qiLogDebug() << "Pointees are " << srcPointedType->infoString() << " " << dstPointedType->infoString();
      // We only try to handle conversion for pointer to objects
      if (srcPointedType->kind() != TypeKind_Object || dstPointedType->kind() != TypeKind_Object)
      {
        // However, we need the full check for exact match here
        if (_type->info() == targetType->info())
          return std::make_pair(*this, false);
        else
        {
          qiLogDebug() << "Conversion between non-object pointers not supported";
          return std::make_pair(AnyReference(), false);
        }
      }
      AnyReference pointedSrc = static_cast<PointerTypeInterface*>(_type)->dereference(_value);
      // try object conversion (inheritance)
      std::pair<AnyReference, bool> pointedDstPair = pointedSrc.convert(dstPointedType);
      if (!pointedDstPair.first._type)
      {
        qiLogDebug() << "Attempting object->proxy conversion";
        // try object->proxy conversion by simply rewrapping this
        detail::ManagedObjectPtr o(
              new GenericObject(
                static_cast<ObjectTypeInterface*>(pointedSrc._type),
                pointedSrc._value),
              boost::bind(dropIt, _1, qi::AnyValue(*this)));
        return AnyReference::from(o).convert((TypeInterface*)targetType);
      }
      if (pointedDstPair.second)
        qiLogError() << "assertion error, allocated converted reference";
      // We must re-reference
      AnyReference pointedDst = pointedDstPair.first;
      void* ptr = pointedDst._type->ptrFromStorage(&pointedDst._value);
      result = AnyReference((TypeInterface*)targetType);
      targetType->setPointee(&result._value, ptr);
      return std::make_pair(result, false);
    }
    case TypeKind_Object:
    {
      std::pair<AnyReference, bool> gv = convert(
                                              static_cast<PointerTypeInterface*>(targetType)->pointedType());
      if (!gv.first._type)
        return gv;
      // Re-pointerise it
      void* ptr = gv.first._type->ptrFromStorage(&gv.first._value);
      AnyReference result;
      result._type = targetType;
      result._value = targetType->initializeStorage(&ptr);
      return std::make_pair(result, false);
    }
    default:
      break;
    }
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReferenceBase::convert(ListTypeInterface* targetType) const
  {
    AnyReference result;

    switch (_type->kind())
    {
    case TypeKind_List:
    {
      ListTypeInterface* targetListType = static_cast<ListTypeInterface*>(targetType);
      ListTypeInterface* sourceListType = static_cast<ListTypeInterface*>(_type);

      TypeInterface* srcElemType = sourceListType->elementType();
      TypeInterface* dstElemType = targetListType->elementType();
      bool needConvert = (srcElemType->info() != dstElemType->info());
      result = AnyReference((TypeInterface*)targetListType);

      AnyIterator iend = end();
      for (AnyIterator it = begin(); it!= iend; ++it)
      {
        AnyReference val = *it;
        if (!needConvert)
          result._append(val);
        else
        {
          std::pair<AnyReference,bool> c = val.convert(dstElemType);
          if (!c.first._type)
          {
            qiLogDebug() << "List element conversion failure from "
                         << val._type->infoString() << " to " << dstElemType->infoString();
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

  std::pair<AnyReference, bool> AnyReferenceBase::convert(StringTypeInterface* targetType) const
  {
    AnyReference result;

    switch (_type->kind())
    {
    case TypeKind_String:
    {
      if (targetType->info() == _type->info())
        return std::make_pair(*this, false);
      result._type = targetType;
      result._value = targetType->initializeStorage();
      std::pair<char*, size_t> v = static_cast<StringTypeInterface*>(_type)->get(_value);
      static_cast<StringTypeInterface*>(targetType)->set(&result._value,
                                                v.first, v.second);
      return std::make_pair(result, true);
    }
    case TypeKind_Raw:
    {
      qiLogWarning() << "Conversion attempt from raw to string";
      return std::make_pair(AnyReference(), false);
    }
    default:
      break;
    }
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReferenceBase::convert(RawTypeInterface* targetType) const
  {
    AnyReference result;

    switch (_type->kind())
    {
    case TypeKind_Raw:
    {
      if (targetType->info() == _type->info())
        return std::make_pair(*this, false);
      result._type = targetType;
      result._value = targetType->initializeStorage();
      std::pair<char*, size_t> v = static_cast<RawTypeInterface*>(_type)->get(_value);
      static_cast<RawTypeInterface*>(result._type)->set(&result._value, v.first, v.second);
      return std::make_pair(result, true);
    }
    case TypeKind_String:
    {
      std::pair<char*, size_t> data = static_cast<StringTypeInterface*>(_type)->get(_value);
      result._type = targetType;
      result._value = targetType->initializeStorage();
      static_cast<RawTypeInterface*>(result._type)->set(&result._value, data.first, data.second);
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReferenceBase::convert(FloatTypeInterface* targetType) const
  {
    AnyReference result;

    switch (_type->kind())
    {
    case TypeKind_Float:
    {
      result._type = targetType;
      result._value = targetType->initializeStorage();
      static_cast<FloatTypeInterface*>(targetType)->set(&result._value,
                                                        static_cast<FloatTypeInterface*>(_type)->get(_value));
      return std::make_pair(result, true);
    }
    case TypeKind_Int:
    {
      AnyReference result(static_cast<TypeInterface*>(targetType));
      int64_t v = static_cast<IntTypeInterface*>(_type)->get(_value);
      if (static_cast<IntTypeInterface*>(_type)->isSigned())
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

  std::pair<AnyReference, bool> AnyReferenceBase::convert(IntTypeInterface* targetType) const
  {
    AnyReference result;

    switch (_type->kind())
    {
    case TypeKind_Int:
    {
      IntTypeInterface* tsrc = static_cast<IntTypeInterface*>(_type);
      IntTypeInterface* tdst = static_cast<IntTypeInterface*>(targetType);
      int64_t v = tsrc->get(_value);
      /* Bounce to GVP to perform overflow checks
      */
      AnyReference result((TypeInterface*)tdst);
      if (tsrc->isSigned())
        result.setInt(v);
      else
        result.setUInt((uint64_t)v);
      tdst->set(&result._value, v);
      return std::make_pair(result, true);
    }
    case TypeKind_Float:
    {
      double v = static_cast<FloatTypeInterface*>(_type)->get(_value);
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

  std::pair<AnyReference, bool> AnyReferenceBase::convert(StructTypeInterface* targetType) const
  {
    AnyReference result;
    StructTypeInterface* tdst = targetType;

    switch (_type->kind())
    {
    case TypeKind_Tuple:
    {
      StructTypeInterface* tsrc = static_cast<StructTypeInterface*>(_type);
      std::vector<void*> sourceData = tsrc->get(_value);
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
        if (!conv.first._type)
        {
          qiLogWarning() << "Conversion failure in tuple member between "
                         << srcTypes[i]->infoString() << " and " << dstTypes[i]->infoString();
          return std::make_pair(AnyReference(), false);
        }
        targetData.push_back(conv.first._value);
        mustDestroy.push_back(conv.second);
      }
      void* dst = tdst->initializeStorage();
      tdst->set(&dst, targetData);
      for (unsigned i=0; i<mustDestroy.size(); ++i)
      {
        if (mustDestroy[i])
          dstTypes[i]->destroy(targetData[i]);
      }
      result._type = targetType;
      result._value = dst;
      return std::make_pair(result, true);
    }
    case TypeKind_List:
    {
      // No explicit type-check, convert will do it
      // handles for instance [i] -> (iii) if size matches
      // or [m] -> (anything) if effective types match
      ListTypeInterface* tsrc = static_cast<ListTypeInterface*>(_type);

      AnyIterator srcBegin = tsrc->begin(_value);
      AnyIterator srcEnd = tsrc->end(_value);

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
        if (!conv.first._type)
        {
          for (unsigned u = 0; u < targetData.size(); ++u)
            if (mustDestroy[u])
              dstTypes[u]->destroy(targetData[u]);
          return std::make_pair(AnyReference(), false);
        }
        targetData.push_back(conv.first._value);
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
      result._type = targetType;
      result._value = dst;
      return std::make_pair(result, true);
    }
    default:
      break;
    }
    return std::make_pair(AnyReference(), false);
  }

  std::pair<AnyReference, bool> AnyReferenceBase::convert(MapTypeInterface* targetType) const
  {
    AnyReference result;

    switch (_type->kind())
    {
    case TypeKind_Map:
    {
      result = AnyReference(static_cast<TypeInterface*>(targetType));

      MapTypeInterface* targetMapType = static_cast<MapTypeInterface*>(targetType);
      MapTypeInterface* srcMapType = static_cast<MapTypeInterface*>(_type);

      TypeInterface* srcKeyType = srcMapType->keyType();
      TypeInterface* srcElementType = srcMapType->elementType();

      TypeInterface* targetKeyType = targetMapType->keyType();
      TypeInterface* targetElementType = targetMapType->elementType();

      bool sameKey = srcKeyType->info() == targetKeyType->info();
      bool sameElem = srcElementType->info() == targetElementType->info();

      AnyIterator iend = end();
      for (AnyIterator it = begin(); it != iend; ++it)
      {
        std::pair<AnyReference, bool> ck, cv;
        AnyReference kv = *it;
        if (!sameKey)
        {
          ck = kv[0].convert(targetKeyType);
          if (!ck.first._type)
            return std::make_pair(AnyReference(), false);
        }
        if (!sameElem)
        {
          cv = kv[1].convert(targetElementType);
          if (!cv.first._type)
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
    case TypeKind_List:
    {
      // Accept [(kv)] and convert to {kv}
      // Also accept [[m]] , [[k]] if k=v and size match, and other compatible stuffs
      result = AnyReference(static_cast<TypeInterface*>(targetType));
      ListTypeInterface* tsrc = static_cast<ListTypeInterface*>(_type);

      AnyIterator srcBegin = tsrc->begin(_value);
      AnyIterator srcEnd = tsrc->end(_value);

      std::vector<TypeInterface*> vectorValueType;
      vectorValueType.push_back(static_cast<MapTypeInterface*>(targetType)->keyType());
      vectorValueType.push_back(static_cast<MapTypeInterface*>(targetType)->elementType());

      TypeInterface *pairType = makeTupleType(vectorValueType);

      while (srcBegin != srcEnd)
      {
        std::pair<AnyReference, bool> conv = (*srcBegin).convert(pairType);
        if (!conv.first._type)
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

  std::pair<char*, size_t> AnyReferenceBase::asRaw() const {
    if (kind() != TypeKind_Raw)
      throw std::runtime_error("asRaw only available for raw kind");
    return static_cast<RawTypeInterface*>(_type)->get(_value);
  }

  std::pair<AnyReference, bool> AnyReferenceBase::convert(TypeInterface* targetType) const
  {
    qiLogDebug() << "convert "
      << _type->infoString() << '(' << _type->kind() << ") "
      << targetType->infoString() << '(' << targetType->kind() << ')' ;
    /* Can have false-negative (same effective type, different Type instances
     * but we do not care, correct check (by comparing info() result
     * is more expensive than the dummy conversion that will happen.
     */
    if (_type == targetType)
      return std::make_pair(*this, false);

    if (!targetType || !_type) {
      qiLogWarning() << "Conversion error: can't convert to/from a null type.";
      return std::make_pair(AnyReference(), false);
    }

    AnyReference result;
    TypeKind skind = _type->kind();
    TypeKind dkind = targetType->kind();

    if (skind == dkind)
    {
      switch(dkind)
      {
      case TypeKind_Void:
        return std::make_pair(qi::AnyReference(targetType), true);
      case TypeKind_Float:
        return convert(static_cast<FloatTypeInterface*>(targetType));
      case TypeKind_Int:
        return convert(static_cast<IntTypeInterface*>(targetType));
      case TypeKind_String:
        return convert(static_cast<StringTypeInterface*>(targetType));
      case TypeKind_List:
        return convert(static_cast<ListTypeInterface*>(targetType));
      case TypeKind_Map:
        return convert(static_cast<MapTypeInterface*>(targetType));
      case TypeKind_Pointer:
        return convert(static_cast<PointerTypeInterface*>(targetType));
      case TypeKind_Tuple:
        return convert(static_cast<StructTypeInterface*>(targetType));
      case TypeKind_Dynamic:
        return convert(static_cast<DynamicTypeInterface*>(targetType));
      case TypeKind_Raw:
        return convert(static_cast<RawTypeInterface*>(targetType));
      case TypeKind_Unknown:
      {
        /* Under clang macos, typeInfo() comparison fails
           * for non-exported (not forced visibility=default since we default to hidden)
           * symbols. So ugly hack, compare the strings.
           */
        if (targetType->info() == _type->info()
    #ifdef __clang__
            || targetType->info().asString() ==  _type->info().asString()
    #endif
            )
          return std::make_pair(*this, false);
        else
          return std::make_pair(AnyReference(), false);
      }
      default:
        break;
      }
    }
    // skind != dkind
    if (skind == TypeKind_List && dkind == TypeKind_Tuple)
      return convert(static_cast<StructTypeInterface*>(targetType));
    else if (skind == TypeKind_List && dkind == TypeKind_Map)
      return convert(static_cast<MapTypeInterface*>(targetType));
    else if (skind == TypeKind_Float && dkind == TypeKind_Int)
      return convert(static_cast<IntTypeInterface*>(targetType));
    else if (skind == TypeKind_Int && dkind == TypeKind_Float)
      return convert(static_cast<FloatTypeInterface*>(targetType));
    else if (skind == TypeKind_String && dkind == TypeKind_Raw)
      return convert(static_cast<RawTypeInterface*>(targetType));
    else if (skind == TypeKind_Raw && dkind == TypeKind_String)
      return convert(static_cast<StringTypeInterface*>(targetType));

    if (targetType->info() == typeOf<AnyObject>()->info()
        && _type->kind() == TypeKind_Pointer
        && static_cast<PointerTypeInterface*>(_type)->pointedType()->kind() == TypeKind_Object)
    { // Pointer to concrete object -> AnyObject
      // Keep a copy of this in AnyObject, and destroy on AnyObject destruction
      // That way if this is a shared_ptr, we link to it correctly
      PointerTypeInterface* pT = static_cast<PointerTypeInterface*>(_type);
      AnyObject obj(new GenericObject(
              static_cast<ObjectTypeInterface*>(pT->pointedType()),
              pT->dereference(_value)._value),
             boost::bind(&AnyObject::deleteGenericObjectOnlyAndKeep<AnyValue>, _1, AnyValue(*this)));
      return std::make_pair(AnyReference::from(obj).clone(), true);
    }

    if (_type->info() == typeOf<AnyObject>()->info()
        && targetType->kind() == TypeKind_Pointer)
    { // Attempt specialized proxy conversion
      qiLogDebug() << "Attempting specialized proxy conversion";
      detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
      detail::ProxyGeneratorMap::iterator it = map.find(
                                                 static_cast<PointerTypeInterface*>(targetType)->pointedType()->info());
      if (it != map.end())
      {
        AnyReference res = (it->second)(*(AnyObject*)_value);
        return std::make_pair(res, true);
      }
      else
        qiLogDebug() << "type "
                     << static_cast<PointerTypeInterface*>(targetType)->pointedType()->infoString()
                     <<" not found in proxy map";
    }

    if (targetType->kind() == TypeKind_Dynamic)
      return convert(static_cast<DynamicTypeInterface*>(targetType));

    if (_type->kind() == TypeKind_Dynamic)
    {
      AnyReference gv = asDynamic();
      std::pair<AnyReference, bool> result = gv.convert(targetType);
      return result;
    }

    if (skind == TypeKind_Object && dkind == TypeKind_Pointer)
      return convert(static_cast<PointerTypeInterface*>(targetType));

    if (skind == TypeKind_Object)
    {
      // Try inheritance
      ObjectTypeInterface* osrc = static_cast<ObjectTypeInterface*>(_type);
      qiLogDebug() << "inheritance check "
        << osrc <<" " << (osrc?osrc->inherits(targetType):false);
      int inheritOffset = 0;
      if (osrc && (inheritOffset =  osrc->inherits(targetType)) != -1)
      {
        // We return a Value that point to the same data as this.
        result._type = targetType;
        result._value = (void*)((long)_value + inheritOffset);
        return std::make_pair(result, false);
      }
    }

    if (_type->info() == targetType->info())
      return std::make_pair(*this, false);

    return std::make_pair(AnyReference(), false);
  }

  AnyReference AnyReferenceBase::convertCopy(TypeInterface* targetType) const
  {
    std::pair<AnyReference, bool> res = convert(targetType);
    if (res.second)
      return res.first;
    else
      return res.first.clone();
  }

  bool operator< (const AnyReference& a, const AnyReference& b)
  {
    //qiLogDebug() << "Compare " << a._type << ' ' << b._type;
#define GET(v, t) static_cast< t ## TypeInterface *>(v.type())->get(v.rawValue())
    if (!a.type())
      return b.type() != 0;
    if (!b.type())
      return false;
    /* < operator for char* does not do what we want, so force
    * usage of get() below for string types.
    */
    if ((a.type() == b.type() || a.type()->info() == b.type()->info())
        && a.type()->kind() != TypeKind_String)
    {
      //qiLogDebug() << "Compare sametype " << a._type->infoString();
      return a.type()->less(a.rawValue(), b.rawValue());
    }
    // Comparing values of different types
    TypeKind ka = a.type()->kind();
    TypeKind kb = b.type()->kind();
    //qiLogDebug() << "Compare " << ka << ' ' << kb;
    if (ka != kb)
    {
      if (ka == TypeKind_Int && kb == TypeKind_Float)
        return GET(a, Int) < GET(b, Float);
      else if (ka == TypeKind_Float && kb == TypeKind_Int)
        return GET(a, Float) < GET(b, Int);
      else
        return ka < kb; // Safer than comparing pointers
    }
    else switch(ka)
    {
    case TypeKind_Void:
      return false;
    case TypeKind_Int:
      return GET(a, Int) < GET(b, Int);
    case TypeKind_Float:
      return GET(a, Float) < GET(b, Float);
    case TypeKind_String:
    {
      std::pair<char*, size_t> ca, cb;
      ca = GET(a, String);
      cb = GET(b, String);
      bool res = ca.second == cb.second?
                   (memcmp(ca.first, cb.first, ca.second) < 0) : (ca.second < cb.second);
      qiLogDebug() << "Compare " << ca.first << ' ' << cb.first << ' ' << res;
      return res;
    }
    case TypeKind_List:
    case TypeKind_Map: // omg, same code!
    {
      size_t la = a.size();
      size_t lb = b.size();
      if (la != lb)
        return la < lb;
      AnyIterator ita   = a.begin();
      AnyIterator enda = a.end();
      AnyIterator itb   = b.begin();
      AnyIterator endb = b.end();
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
    case TypeKind_Object:
    case TypeKind_Pointer:
    case TypeKind_Tuple:
    case TypeKind_Dynamic:
    case TypeKind_Raw:
    case TypeKind_Unknown:
    case TypeKind_Iterator:
    case TypeKind_Function:
    case TypeKind_Signal:
    case TypeKind_Property:
      return a.rawValue() < b.rawValue();
    }
#undef GET
    return a.rawValue() < b.rawValue();
  }

  bool operator< (const AnyValue& a, const AnyValue& b)
  {
    return a.asReference() < b.asReference();
  }

  bool operator==(const AnyReference& a, const AnyReference& b)
  {
    if (a.kind() == TypeKind_Iterator && b.kind() == TypeKind_Iterator
        && a.type()->info() == b.type()->info())
    {
      return static_cast<IteratorTypeInterface*>(a.type())->equals(a.rawValue(), b.rawValue());
    }
    else
      return ! (a < b) && !(b < a);
  }

  bool operator==(const AnyValue& a, const AnyValue& b)
  {
    return a.asReference() == b.asReference();
  }

  bool operator==(const AnyIterator& a, const AnyIterator& b)
  {
    return a.asReference() == b.asReference();
  }

  AnyValue AnyReferenceBase::toTuple(bool homogeneous) const
  {
    if (kind() == TypeKind_Tuple)
      return AnyValue(*this);
    else if (kind() != TypeKind_List)
      throw std::runtime_error("Expected Tuple or List kind");
    // convert list to tuple

    ListTypeInterface* t = static_cast<ListTypeInterface*>(_type);
    TypeInterface* te = t->elementType();
    DynamicTypeInterface* td = 0;
    if (te->kind() == TypeKind_Dynamic)
      td = static_cast<DynamicTypeInterface*>(te);
    if (!homogeneous && !td)
      throw std::runtime_error("Element type is not dynamic");
    std::vector<AnyReference> elems;
    AnyIterator it = begin();
    AnyIterator iend = end();
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
    return AnyValue(makeGenericTuple(elems), false, true);
  }

  AnyObject AnyReferenceBase::toObject() const
  {
    return to<AnyObject>();
  }

  AnyReference AnyReferenceBase::_element(const AnyReference& key, bool throwOnFailure)
  {
    if (kind() == TypeKind_List)
    {
      ListTypeInterface* t = static_cast<ListTypeInterface*>(_type);
      int ikey = (int)key.toInt();
      if (ikey < 0 || static_cast<size_t>(ikey) >= t->size(_value))
      {
        if (throwOnFailure)
          throw std::runtime_error("Index out of range");
        else
          return AnyReference();
      }
      return AnyReference(t->elementType(), t->element(_value, ikey));
    }
    else if (kind() == TypeKind_Map)
    {
      MapTypeInterface* t = static_cast<MapTypeInterface*>(_type);
      std::pair<AnyReference, bool> c = key.convert(t->keyType());
      if (!c.first._type)
        throw std::runtime_error("Incompatible key type");
      // HACK: should be two separate booleans
      bool autoInsert = throwOnFailure;
      AnyReference result
          = t->element(&_value, c.first._value, autoInsert);
      if (c.second)
        c.first.destroy();
      return result;
    }
    else if (kind() == TypeKind_Tuple)
    {
      StructTypeInterface* t = static_cast<StructTypeInterface*>(_type);
      int ikey = (int)key.toInt();
      std::vector<TypeInterface*> types = t->memberTypes();
      if (ikey < 0 || static_cast<size_t>(ikey) >= types.size())
      {
        if (throwOnFailure)
          throw std::runtime_error("Index out of range");
        else
          return AnyReference();
      }
      return AnyReference(types[ikey], t->get(_value, ikey));
    }
    else
      throw std::runtime_error("Expected List, Map or Tuple kind");
  }

  void AnyReferenceBase::_append(const AnyReference& elem)
  {
    if (kind() != TypeKind_List)
      throw std::runtime_error("Expected a list");
    ListTypeInterface* t = static_cast<ListTypeInterface*>(_type);
    std::pair<AnyReference, bool> c = elem.convert(t->elementType());
    t->pushBack(&_value, c.first._value);
    if (c.second)
      c.first.destroy();
  }

  void AnyReferenceBase::_insert(const AnyReference& key, const AnyReference& val)
  {
    if (kind() != TypeKind_Map)
      throw std::runtime_error("Expected a map");
    std::pair<AnyReference, bool> ck(key, false);
    std::pair<AnyReference, bool> cv(val, false);
    MapTypeInterface* t = static_cast<MapTypeInterface*>(_type);
    if (key._type != t->keyType())
      ck = key.convert(t->keyType());
    if (val._type != t->elementType())
      cv = val.convert(t->elementType());
    t->insert(&_value, ck.first._value, cv.first._value);
    if (ck.second)
      ck.first.destroy();
    if (cv.second)
      cv.first.destroy();
  }

  void AnyReferenceBase::update(const AutoAnyReference& val)
  {
    switch(kind())
    {
    case TypeKind_Int:
      setInt(val.toInt());
      break;
    case TypeKind_Float:
      setDouble(val.toDouble());
      break;
    case TypeKind_String:
      setString(val.toString());
      break;
    case TypeKind_Dynamic:
      setDynamic(val);
      break;
    case TypeKind_Tuple:
      //asTupleValuePtr is not const, so copy val to a non-const AnyReference
      setTuple(AnyReference(val).asTupleValuePtr());
    case TypeKind_Raw: {
        std::pair<char*, size_t> pa = val.asRaw();
        setRaw(pa.first, pa.second);
      }
    default:
      throw std::runtime_error("Update not implemented for this type.");
    }
  }

  void AnyReferenceBase::setInt(int64_t v)
  {
    if (kind() == TypeKind_Int)
    {
      IntTypeInterface* type = static_cast<IntTypeInterface*>(this->_type);
      if (!type->isSigned() && v < 0)
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Converting negative value %s to unsigned type", v));
      // not signed gives us an extra bit, but signed can go down an extra value
      if (type->size() > 8)
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to %s bytes", v, type->size()));
      if (type->size() == 0 && (v < 0 || v > 1))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Expected 0 or 1 when converting to bool, got %s", v));
      if (type->size() > 0 && type->size() < 8 && (std::abs(v) >= (1LL << (8*type->size() - (type->isSigned()?1:0))) + ((v<0)?1:0)))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to %s bytes", v, type->size()));
      type->set(&_value, v);
    }
    else if (kind() == TypeKind_Float)
      static_cast<FloatTypeInterface*>(_type)->set(&_value, static_cast<double>(v));
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  void AnyReferenceBase::setDynamic(const qi::AnyReference &element) {
    if (kind() != TypeKind_Dynamic)
      throw std::runtime_error("Value is not a Dynamic");
    DynamicTypeInterface* t = static_cast<DynamicTypeInterface*>(this->_type);
    t->set(&_value, element);
  }

  void AnyReferenceBase::setRaw(const char *buffer, size_t size) {
    if (kind() != TypeKind_Raw)
      throw std::runtime_error("Value is not a Raw");
    static_cast<RawTypeInterface*>(_type)->set(&_value, buffer, size);
  }

  void AnyReferenceBase::setUInt(uint64_t v)
  {
    if (kind() == TypeKind_Int)
    {
      IntTypeInterface* type = static_cast<IntTypeInterface*>(this->_type);
      if (type->size() > 0 && type->size() < 8 && (v >= (1ULL << (8*type->size() - (type->isSigned()?1:0)))))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to %s bytes", v, type->size()));
      if (type->size() == 0 && (v > 1))
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Expected 0 or 1 when converting to bool, got %s", v));
      if (type->size() == 8 && type->isSigned() && v >= 0x8000000000000000ULL)
        throw std::runtime_error(_QI_LOG_FORMAT_HASARG_0("Overflow converting %s to signed int64", v));
      type->set(&_value, (int64_t)v);
    }
    else if (kind() == TypeKind_Float)
      static_cast<FloatTypeInterface*>(_type)->set(&_value,
        static_cast<double>(static_cast<uint64_t>(v)));
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  void AnyReferenceBase::setDouble(double v)
  {
    if (kind() == TypeKind_Float)
      static_cast<FloatTypeInterface*>(_type)->set(&_value, v);
    else if (kind() == TypeKind_Int)
    {
      IntTypeInterface* type = static_cast<IntTypeInterface*>(this->_type);
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
      type->set(&_value, static_cast<int64_t>(v));
    }
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  std::vector<AnyReference> AnyReferenceBase::asTupleValuePtr()
  {
    if (kind() == TypeKind_Tuple)
      return static_cast<StructTypeInterface*>(_type)->values(_value);
    else if (kind() == TypeKind_List || kind() == TypeKind_Map)
    {
      std::vector<AnyReference> result;
      AnyIterator iend = end();
      AnyIterator it = begin();
      for(; it != iend; ++it)
        result.push_back(*it);
      return result;
    }
    else
      throw std::runtime_error("Expected tuple, list or map");
  }

  AnyIterator AnyReferenceBase::begin() const
  {
    if (kind() == TypeKind_List)
      return static_cast<ListTypeInterface*>(_type)->begin(_value);
    else if (kind() == TypeKind_Map)
      return static_cast<MapTypeInterface*>(_type)->begin(_value);
    else
      throw std::runtime_error("Expected list or map");
  }

  AnyIterator AnyReferenceBase::end() const
  {
    if (kind() == TypeKind_List)
      return static_cast<ListTypeInterface*>(_type)->end(_value);
    else if (kind() == TypeKind_Map)
      return static_cast<MapTypeInterface*>(_type)->end(_value);
    else
      throw std::runtime_error("Expected list or map");
  }

  std::map<AnyReference, AnyReference> AnyReferenceBase::asMapValuePtr()
  {
    if (kind() != TypeKind_Map)
      throw std::runtime_error("Expected a map");
    std::map<AnyReference, AnyReference> result;
    AnyIterator iend = end();
    AnyIterator it = begin();
    for(; it != iend; ++it)
    {
      AnyReference elem = *it;
      result[elem[0]] = elem[1];
    }
    return result;
  }

  void AnyReferenceBase::setString(const std::string& v)
  {
    if (kind() != TypeKind_String)
      throw std::runtime_error("Value is not of kind string");
    static_cast<StringTypeInterface*>(_type)->set(&_value, &v[0], v.size());
  }

  void AnyReferenceBase::setTuple(const std::vector<AnyReference>& values) {
    if (kind() != TypeKind_Tuple)
      throw std::runtime_error("Value is not a Tuple");
    StructTypeInterface* stype = static_cast<StructTypeInterface*>(_type);
    std::vector<TypeInterface*> types = stype->memberTypes();
    std::vector<void*> vals;

    if (types.size() != values.size())
      throw std::runtime_error(_QI_LOG_FORMAT("Can't change values of the tuple, size mismatch (%d != %d)", types.size(), values.size()));
    vals.resize(types.size());
    for (unsigned int i = 0; i < values.size(); ++i) {
      //TODO: implement conversion here
      if (types[i]->info() == values[i].type()->info())
        vals[i] = values[i].rawValue();
      else
        throw std::runtime_error("Type mismatchs when assigning values to tuple");
    }
    stype->set(&_value, vals);
  }

  size_t AnyReferenceBase::size() const
  {
    if (kind() == TypeKind_List)
      return static_cast<ListTypeInterface*>(_type)->size(_value);
    if (kind() == TypeKind_Map)
      return static_cast<MapTypeInterface*>(_type)->size(_value);
    if (kind() == TypeKind_Tuple)
      return static_cast<StructTypeInterface*>(_type)->memberTypes().size();
    else
      throw std::runtime_error("Expected List, Map or Tuple.");
  }


  AnyReference AnyReferenceBase::asDynamic() const
  {
    if (kind() != TypeKind_Dynamic)
      throw std::runtime_error("Not of dynamic kind");
    DynamicTypeInterface* d = static_cast<DynamicTypeInterface*>(_type);
    return d->get(_value);
  }

  AnyReference AnyReferenceBase::operator*()
  {
    if (kind() == TypeKind_Pointer)
      return static_cast<PointerTypeInterface*>(_type)->dereference(_value);
    else if (kind() == TypeKind_Iterator)
      return static_cast<IteratorTypeInterface*>(_type)->dereference(_value);
    else if (kind() == TypeKind_Dynamic)
      return static_cast<DynamicTypeInterface*>(_type)->get(_value);
    else
      throw std::runtime_error("Expected pointer or iterator");
  }

  std::vector<TypeInterface*> AnyReferenceBase::membersType() const
  {
    if (kind() == TypeKind_Tuple)
      return static_cast<StructTypeInterface*>(_type)->memberTypes();
    else
      throw std::runtime_error("Expected tuple");
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

