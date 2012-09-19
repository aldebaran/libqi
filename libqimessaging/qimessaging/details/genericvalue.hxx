/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once

#ifndef _QIMESSAGING_DETAILS_GENERICVALUE_HXX_
#define _QI_MESSAGING_VALUE_HXX_

namespace qi {


class ValueClone
{
public:
  void* clone(void* src)
  {
    return new GenericValue(((GenericValue*)src)->clone());
  }

  void destroy(void* ptr)
  {
    ((GenericValue*)ptr)->destroy();
    delete (GenericValue*)ptr;
  }
};

class ValueValue
{
public:

  bool toValue(const void* ptr, qi::detail::DynamicValue& val)
  {
    GenericValue* m = (GenericValue*)ptr;
    return m->type->toValue(m->value, val);
  }
  void* fromValue(const qi::detail::DynamicValue& val)
  {
    GenericValue v = ::qi::toValue(val);
    return new GenericValue(v.clone());
  }
};

template<> class TypeImpl<GenericValue>:
  public DefaultTypeImpl<
    GenericValue,
    ValueClone,
    ValueValue,
    TypeDefaultSerialize<GenericValue>
    > {};

template<typename T>
GenericValue toValue(const T& v)
{
  GenericValue res;
  res.type = typeOf<typename boost::remove_const<T>::type>();
  res.value = (void *) const_cast<T*>(&v);
  return res;
}

inline
GenericValue GenericValue::clone() const
{
  GenericValue res;
  res.type = type;
  res.value = type?res.type->clone(value):0;
  return res;
}

template<typename T>
std::pair<const T*, bool> GenericValue::as() const
{
  Type* targetType =  typeOf<T>();
  if (type->info() == targetType->info())
    return std::make_pair((const T*)value, false);
  else
  {
    GenericValue mv = convert(*targetType);
    return std::make_pair((const T*)mv.value, true);
  }
}

/** Type conversion. Will always make a copy.
 */
inline
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
    //std::cerr <<"Temp value has " << temp << std::endl;
    res.value = res.type->fromValue(temp);
  }
  return res;
}

inline AutoGenericValue::AutoGenericValue(const AutoGenericValue& b)
{
  value = b.value;
  type = b.type;
}

template<typename T> AutoGenericValue::AutoGenericValue(const T& ptr)
{
  *(GenericValue*)this = toValue(ptr);
}

inline AutoGenericValue::AutoGenericValue()
{
  value = type = 0;
}

namespace detail
{
  /** This class can be used to convert the return value of an arbitrary function
  * into a GenericValue. It handles functions returning void.
  *
  *  Usage:
  *    ValueCopy val;
  *    val(), functionCall(arg);
  *
  *  in val(), parenthesis are useful to avoid compiler warning "val not used" when handling void.
  */
  class GenericValueCopy: public GenericValue
  {
  public:
    template<typename T> void operator,(const T& any);
    GenericValueCopy &operator()() { return *this; }
  };

  template<typename T> void GenericValueCopy::operator,(const T& any)
  {
    *(GenericValue*)this = toValue(any);
    *(GenericValue*)this = clone();
  }
}


}

#endif  // _QIMESSAGING_DETAILS_GENERICVALUE_HXX_
