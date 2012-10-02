/*
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qimessaging/genericvalue.hpp>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/genericvaluespecialized.hpp>

namespace qi
{

std::pair<GenericValue, bool> GenericValue::convert(Type* targetType) const
{
  /* Can have false-negative (same effective type, different Type instances
   * but we do not care, correct check (by comparing info() result
   * is more expensive than the dummy conversion that will happen.
  */
  if (type == targetType)
  {
    return std::make_pair(*this, false);
  }

  GenericValue result;
  Type::Kind skind = type->kind();
  Type::Kind dkind = targetType->kind();
  if (skind == dkind)
  {
    switch(skind)
    {
    case Type::Float:
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<TypeFloat*>(targetType)->set(&result.value,
        static_cast<TypeFloat*>(type)->get(value));
      return std::make_pair(result, true);
    case Type::Int:
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<TypeInt*>(targetType)->set(&result.value,
        static_cast<TypeInt*>(type)->get(value));
      return std::make_pair(result, true);
    case Type::String:
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<TypeString*>(targetType)->set(&result.value,
        static_cast<TypeString*>(type)->getString(value));
      return std::make_pair(result, true);
    case Type::List:
    {
      result.type = targetType;
      GenericList lsrc = asList();
      TypeList* targetListType = static_cast<TypeList*>(targetType);
      Type* srcElemType = lsrc.elementType();
      void* storage = targetType->initializeStorage();
      Type* dstElemType = targetListType->elementType(storage);
      bool needConvert = (srcElemType->info() != dstElemType->info());
      GenericList lresult;
      lresult.type = targetListType;
      lresult.value = storage;
      GenericListIterator i = lsrc.begin();
      GenericListIterator iend = lsrc.end();
      for (; i!= iend; ++i)
      {
        GenericValue val = *i;
        if (!needConvert)
          lresult.pushBack(val);
        else
        {
          std::pair<GenericValue,bool> c = val.convert(dstElemType);
          lresult.pushBack(c.first);
          if (c.second)
            c.first.destroy();
        }
      }
      return std::make_pair(lresult, true);
    }
    break;
  case Type::Map:
    {
      result.type = targetType;
      GenericMap msrc = asMap();


      TypeMap* targetMapType = static_cast<TypeMap*>(targetType);
      TypeMap* srcMapType = static_cast<TypeMap*>(type);

      Type* srcKeyType = srcMapType->keyType(value);
      Type* srcElementType = srcMapType->elementType(value);


      GenericMap mresult;
      mresult.type = targetType;
      mresult.value = targetMapType->initializeStorage();
      Type* targetKeyType = targetMapType->keyType(mresult.value);
      Type* targetElementType = targetMapType->elementType(mresult.value);

      bool sameKey = srcKeyType->info() == targetKeyType->info();
      bool sameElem = srcElementType->info() == targetElementType->info();

      GenericMapIterator i = msrc.begin();
      GenericMapIterator iend = msrc.end();
      for (; i != iend; ++i)
      {
        std::pair<GenericValue, GenericValue> kv = *i;
        std::pair<GenericValue, bool> ck, cv;
        if (!sameKey)
          ck = kv.first.convert(targetKeyType);
        if (!sameElem)
          cv = kv.second.convert(targetElementType);
        mresult.insert(sameKey?kv.first:ck.first, sameElem?kv.second:cv.first);
        if (!sameKey && ck.second)
          ck.first.destroy();
        if (!sameElem && cv.second)
          cv.first.destroy();
      }
      return std::make_pair(mresult, true);
    }
    break;
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
          return std::make_pair(GenericValue(), false);
      }
      GenericValue pointedSrc = static_cast<TypePointer*>(type)->dereference(value);
      std::pair<GenericValue, bool> pointedDstPair = pointedSrc.convert(dstPointedType);
      if (!pointedDstPair.first.type)
        return std::make_pair(GenericValue(), false);
      if (pointedDstPair.second)
        qiLogError("qi.meta") << "assertion error, allocated converted reference";
      // We must re-reference
      GenericValue pointedDst = pointedDstPair.first;
      void* ptr = pointedDst.type->ptrFromStorage(&pointedDst.value);
      result.type = targetType;
      result.value = targetType->initializeStorage(&ptr);
      return std::make_pair(result, false);
    }
    break;
  case Type::Tuple:
    {
      TypeTuple* tsrc = static_cast<TypeTuple*>(type);
      TypeTuple* tdst = static_cast<TypeTuple*>(targetType);
      std::vector<void*> sourceData = tsrc->get(value);
      std::vector<Type*> srcTypes = tsrc->memberTypes(value);
      std::vector<Type*> dstTypes = tdst->memberTypes(0);
      if (dstTypes.size() != sourceData.size())
        return std::make_pair(GenericValue(), false);

      std::vector<void*> targetData;
      std::vector<bool> mustDestroy;
      for (unsigned i=0; i<dstTypes.size(); ++i)
      {
        std::pair<GenericValue, bool> conv = GenericValue(srcTypes[i], sourceData[i]).convert(dstTypes[i]);
        if (!conv.first.type)
        {
          qiLogWarning("qi.meta") << "Conversion failure in tuple member between "
            << srcTypes[i]->infoString() << " and " << dstTypes[i]->infoString();
          return std::make_pair(GenericValue(), false);
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
  default:
    break;
  } // switch
  } // skind == dkind
  if (skind == Type::Float && dkind == Type::Int)
  {
    result.type = targetType;
    result.value = targetType->initializeStorage();
    static_cast<TypeInt*>(targetType)->set(&result.value,
      static_cast<TypeFloat*>(type)->get(value));
    return std::make_pair(result, true);
  }
  else if (skind == Type::Int && dkind == Type::Float)
  {
    result.type = targetType;
    result.value = targetType->initializeStorage();
    static_cast<TypeFloat*>(targetType)->set(&result.value,
      static_cast<TypeInt*>(type)->get(value));
    return std::make_pair(result, true);
  }

  static Type* genericValueType = typeOf<GenericValue>();
  static Type* genericObjectType = typeOf<GenericObject>();
  if (targetType->info() == genericValueType->info())
  {
    // Target is metavalue: special case
    GenericValue res;
    res.type = targetType;
    res.value = new GenericValue(*this);
    return std::make_pair(res, false);
  }
  if (type->info() == genericValueType->info())
  { // Source is metavalue: special case
    GenericValue* metaval = (GenericValue*)value;
    return metaval->convert(targetType);
  }
  if (type->info() == genericObjectType->info())
  {
    GenericObject* obj = (GenericObject*)value;
    GenericValue v;
    v.type = obj->type;
    v.value = obj->value;
    return v.convert(targetType);
  }
  if (skind == Type::Object)
  {
    // Try inheritance
    ObjectType* osrc = static_cast<ObjectType*>(type);
    qiLogDebug("qi.meta") << "inheritance check "
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
  {
    return std::make_pair(*this, false);
  }

  return std::make_pair(GenericValue(), false);
}

GenericValue GenericValue::convertCopy(Type* targetType) const
{
  std::pair<GenericValue, bool> res = convert(targetType);
  if (res.second)
    return res.first;
  else
    return res.first.clone();
}

}

QI_TYPE_REGISTER(qi::GenericValue);
