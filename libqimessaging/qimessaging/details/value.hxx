/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once

#ifndef _QI_MESSAGING_VALLUE_HXX_
#define _QI_MESSAGING_VALUE_HXX_

namespace qi {


class ValueClone
{
public:
  void* clone(void* src)
  {
    return new Value(((Value*)src)->clone());
  }

  void destroy(void* ptr)
  {
    ((Value*)ptr)->destroy();
    delete (Value*)ptr;
  }
};

class ValueValue
{
public:

  bool toValue(const void* ptr, qi::detail::DynamicValue& val)
  {
    Value* m = (Value*)ptr;
    return m->type->toValue(m->value, val);
  }
  void* fromValue(const qi::detail::DynamicValue& val)
  {
    Value v = ::qi::toValue(val);
    return new Value(v.clone());
  }
};

template<> class TypeImpl<Value>:
  public DefaultTypeImpl<
    Value,
    ValueClone,
    ValueValue,
    TypeDefaultSerialize<Value>
    > {};

template<typename T>
Value toValue(const T& v)
{
  Value res;
  res.type = typeOf<typename boost::remove_const<T>::type>();
  res.value = (void *) const_cast<T*>(&v);
  return res;
}

inline
Value Value::clone() const
{
  Value res;
  res.type = type;
  res.value = type?res.type->clone(value):0;
  return res;
}

template<typename T>
std::pair<const T*, bool> Value::as() const
{
  Type* targetType =  typeOf<T>();
  if (type->info() == targetType->info())
    return std::make_pair((const T*)value, false);
  else
  {
    Value mv = convert(*targetType);
    return std::make_pair((const T*)mv.value, true);
  }
}

/** Type conversion. Will always make a copy.
 */
inline
Value Value::convert(Type& targetType) const
{
  if (targetType.info() == typeOf<Value>()->info())
  {
    // Target is metavalue: special case
    Value res;
    res.type = &targetType;
    res.value = new Value(clone());
    return res;
  }
  if (type->info() == typeOf<Value>()->info())
  { // Source is metavalue: special case
    Value* metaval = (Value*)value;
    return metaval->convert(targetType);
  }
  Value res;
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

inline AutoValue::AutoValue(const AutoValue& b)
{
  value = b.value;
  type = b.type;
}

template<typename T> AutoValue::AutoValue(const T& ptr)
{
  *(Value*)this = toValue(ptr);
}

inline AutoValue::AutoValue()
{
  value = type = 0;
}

namespace detail
{
  /** This class can be used to convert the return value of an arbitrary function
  * into a Value. It handles functions returning void.
  *
  *  Usage:
  *    ValueCopy val;
  *    val(), functionCall(arg);
  *
  *  in val(), parenthesis are useful to avoid compiler warning "val not used" when handling void.
  */
  class ValueCopy: public Value
  {
  public:
    template<typename T> void operator,(const T& any);
    ValueCopy &operator()() { return *this; }
  };

  template<typename T> void ValueCopy::operator,(const T& any)
  {
    *(Value*)this = toValue(any);
    *(Value*)this = clone();
  }
}


}

#endif
