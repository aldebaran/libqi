/*
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qimessaging/genericvalue.hpp>
#include <qimessaging/genericobject.hpp>

namespace qi
{

GenericValue GenericValue::convert(Type& targetType) const
{
  if (targetType.info() == typeOf<GenericValue>()->info())
  {
    // Target is metavalue: special case
    GenericValue res;
    res.type = &targetType;
    res.value = new GenericValue(clone());
    return res;
  }
  if (type->info() == typeOf<GenericValue>()->info())
  { // Source is metavalue: special case
    GenericValue* metaval = (GenericValue*)value;
    return metaval->convert(targetType);
  }
  else if (type->info() == typeOf<GenericObject>()->info())
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
  if (targetType.info() == type->info())
  { // Same type, just clone
    res.type = type;
    res.value = res.type->clone(value);
  }
  else
  { // Different type, go through value
    res.type = &targetType;
    qi::detail::DynamicValue temp;
    type->toValue(value, temp);
    if (temp.type == detail::DynamicValue::Invalid)
      qiLogWarning("qi.meta") << "Cast error " << type->infoString()
        << " -> " << targetType.infoString();
    //std::cerr <<"Temp value has " << temp << std::endl;
    res.value = res.type->fromValue(temp);
  }
  return res;
}

}
