#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DETAILS_GENERICVALUE_HXX_
#define _QI_MESSAGING_VALUE_HXX_

#include <boost/type_traits/remove_const.hpp>
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
    TypeDefaultAccess<GenericValue>,
    ValueClone,
    ValueValue,
    TypeDefaultSerialize<TypeDefaultAccess<GenericValue> >
    > {};

namespace detail
{
  template<typename T> struct TypeCopy
  {
    void operator()(T &dst, const T &src)
    {
      dst = src;
    }
  };

  template<int I> struct TypeCopy<char[I] >
  {
    void operator() (char* dst, const char* src)
    {
      memcpy(dst, src, I);
    }
  };
}

template<typename T>
GenericValue toValue(const T& v)
{
  GenericValue res;
  res.type = typeOf<typename boost::remove_const<T>::type>();
  res.value = res.type->initializeStorage(const_cast<void*>((const void*)&v));
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
std::pair<const T*, bool> GenericValue::to() const
{
  Type* targetType =  typeOf<T>();
  if (type->info() == targetType->info())
    return std::make_pair((const T*)type->ptrFromStorage((void**)&value), false);
  else
  {
    std::pair<GenericValue, bool> mv = convert(targetType);
    // NOTE: delete theResult.first will not do, destroy must be called,
    return std::make_pair((const T*)mv.first.type->ptrFromStorage(&mv.first.value), mv.second);
  }
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

inline std::string GenericValue::signature() const
{
  if (!type)
      return "";
    else
      return type->signature();
}

inline void GenericValue::destroy()
{
  if (type && value)
    type->destroy(value);
}

inline void GenericValue::serialize(ODataStream& os) const
{
  if (type)
    type->serialize(os, value);
}

inline GenericValue::GenericValue()
: value(0)
, type(0)
{
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
