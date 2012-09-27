/*
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qimessaging/genericvalue.hpp>
#include <qimessaging/genericobject.hpp>

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
        static_cast<TypeString*>(type)->get(value));
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

std::pair<GenericValue, bool> GenericValue::convert2(Type* targetType) const
{
  if (targetType->info() == typeOf<GenericValue>()->info())
  {
    // Target is metavalue: special case
    GenericValue res;
    res.type = targetType;
    res.value = new GenericValue(*this);
    return std::make_pair(res, false);
  }
  if (type->info() == typeOf<GenericValue>()->info())
  { // Source is metavalue: special case
    GenericValue* metaval = (GenericValue*)value;
    return metaval->convert2(targetType);
  }
  if (type->info() == typeOf<GenericObject>()->info())
  {
    GenericObject* obj = (GenericObject*)value;
    GenericValue v;
    v.type = obj->type;
    v.value = obj->value;
    return v.convert2(targetType);
  }
  GenericValue res;
  //std::cerr <<"convert " << targetType.info().name() <<" "
  //<< type->info().name() << std::endl;
  if (targetType->info() == type->info())
  { // Same type
    return std::make_pair(*this, false);
  }
  // Different type
  // Try inheritance
  ObjectType* osrc = dynamic_cast<ObjectType*>(type);
  qiLogDebug("qi.meta") << "inheritance check "
    << osrc <<" " << (osrc?osrc->inherits(targetType):false);
  int inheritOffset = 0;
  if (osrc && (inheritOffset =  osrc->inherits(targetType)) != -1)
  {
    res.type = targetType;
    // We *must* clone, destroy will be called
    res.value = (void*)((long)value + inheritOffset);
    return std::make_pair(res, false);
  }

  // Nothing else worked, go through value
  res.type = targetType;
  qi::detail::DynamicValue temp;
  type->toValue(value, temp);
  if (temp.type == detail::DynamicValue::Invalid)
    qiLogWarning("qi.meta") << "Cast error " << type->infoString()
  << " -> " << targetType->infoString();
  //std::cerr <<"Temp value has " << temp << std::endl;
  res.value = res.type->fromValue(temp);
  return std::make_pair(res, true);
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
