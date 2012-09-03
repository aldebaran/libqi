/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once

#ifndef _QI_MESSAGING_METAVALUE_HXX_
#define _QI_MESSAGING_METAVALUE_HXX_

namespace qi {

inline
qi::ODataStream& operator << (qi::ODataStream& od, const MetaValue& mv)
{
  if (mv.type)
    mv.type->serialize(od, mv.value);
  return od;
}

template<typename T>
MetaValue toMetaValue(const T& v)
{
  MetaValue res;
  res.type = metaTypeOf(v);
  res.value = const_cast<T*>(&v);
  return res;
}

inline
MetaValue MetaValue::clone() const
{
  MetaValue res;
  res.type = type;
  res.value = type?res.type->clone(value):0;
  return res;
}

template<typename T>
std::pair<const T*, bool> MetaValue::as() const
{
  MetaType* targetType =  metaTypeOf<T>();
  if (type->info() == targetType->info())
    return std::make_pair((const T*)value, false);
  else
  {
    MetaValue mv = convert(*targetType);
    return std::make_pair((const T*)mv.value, true);
  }
}

/** Type conversion. Will always make a copy.
 */
inline
MetaValue MetaValue::convert(MetaType& targetType) const
{
  MetaValue res;
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
    qi::detail::Value temp;
    type->toValue(value, temp);
    //std::cerr <<"Temp value has " << temp << std::endl;
    res.value = res.type->fromValue(temp);
  }
  return res;
}

inline AutoMetaValue::AutoMetaValue(const AutoMetaValue& b)
{
  value = b.value;
  type = b.type;
}

template<typename T> AutoMetaValue::AutoMetaValue(const T& ptr)
{
  *(MetaValue*)this = toMetaValue(ptr);
}

inline AutoMetaValue::AutoMetaValue()
{
  value = type = 0;
}

template<typename T> void MetaValueCopy::operator,(const T& any)
{
  *(MetaValue*)this = toMetaValue(any);
  *(MetaValue*)this = clone();
}



}

#endif
