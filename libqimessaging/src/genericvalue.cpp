/*
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qimessaging/genericvalue.hpp>
#include <qimessaging/genericobject.hpp>

namespace qi
{

std::pair<GenericValue, bool> GenericValue::convert2(Type* targetType) const
{
  GenericValue result;
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
    return metaval->convert(targetType);
  }
  if (type->info() == typeOf<GenericObject>()->info())
  {
    GenericObject* obj = (GenericObject*)value;
    GenericValue v;
    v.type = obj->type;
    v.value = obj->value;
    return v.convert(targetType);
  }
  // Try inheritance
  ObjectType* osrc = dynamic_cast<ObjectType*>(type);
  qiLogDebug("qi.meta") << "inheritance check "
    << osrc <<" " << (osrc?osrc->inherits(targetType):false);
  int inheritOffset = 0;
  if (osrc && (inheritOffset =  osrc->inherits(targetType)) != -1)
  {
    result.type = targetType;
    // We *must* clone, destroy will be called
    result.value = (void*)((long)value + inheritOffset);
    return std::make_pair(result, false);
  }

  // Nothing worked yet, go through kind system
  Type::Kind skind = type->kind();
  Type::Kind dkind = targetType->kind();
  result.type = targetType;
  if (type->info() == targetType->info())
  {
    return std::make_pair(*this, false);
  }
  if (skind != dkind)
  {
    if (skind == Type::Float && dkind == Type::Int)
    {
      result.type = targetType;
      result.value = targetType->initializeStorage();
      dynamic_cast<TypeInt*>(targetType)->set(&result.value,
        dynamic_cast<TypeFloat*>(type)->get(value));
      return std::make_pair(result, true);
    }
    else if (skind == Type::Int && dkind == Type::Float)
    {
      result.type = targetType;
      result.value = targetType->initializeStorage();
      dynamic_cast<TypeFloat*>(targetType)->set(&result.value,
         dynamic_cast<TypeInt*>(type)->get(value));
      return std::make_pair(result, true);
    }
    else
      return std::make_pair(GenericValue(), false);
  }
  result.type = targetType;
  switch(skind)
  {
  case Type::Float:
    result.value = targetType->initializeStorage();
    dynamic_cast<TypeFloat*>(targetType)->set(&result.value, asDouble());
    break;
  case Type::Int:
    result.value = targetType->initializeStorage();
    dynamic_cast<TypeInt*>(targetType)->set(&result.value, asInt());
    break;
  case Type::List:
    {
      GenericList lsrc = asList();
      TypeList* targetListType = dynamic_cast<TypeList*>(targetType);
      void* storage = targetType->initializeStorage();
      GenericList lresult;
      lresult.type = targetListType;
      lresult.value = storage;
      GenericIterator i = lsrc.begin();
      GenericIterator iend = lsrc.end();
      for (; i!= iend; ++i)
      {
        lresult.pushBack(*i); // will convert
      }
      result = lresult;
    }
    break;
  }
  return std::make_pair(result, true);
}

std::pair<GenericValue, bool> GenericValue::convert(Type* targetType) const
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
    return metaval->convert(targetType);
  }
  if (type->info() == typeOf<GenericObject>()->info())
  {
    GenericObject* obj = (GenericObject*)value;
    GenericValue v;
    v.type = obj->type;
    v.value = obj->value;
    return v.convert(targetType);
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
