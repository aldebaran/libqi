#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DETAILS_GENERICVALUE_HXX_
#define _QIMESSAGING_DETAILS_GENERICVALUE_HXX_

#include <boost/type_traits/remove_const.hpp>

#include <qimessaging/typespecialized.hpp>

namespace qi {


class ValueClone
{
public:
  static void* clone(void* src)
  {
    return new GenericValue(((GenericValue*)src)->clone());
  }

  static void destroy(void* ptr)
  {
    ((GenericValue*)ptr)->destroy();
    delete (GenericValue*)ptr;
  }
};

class ValueValue
{
public:

  static bool toValue(const void* ptr, qi::detail::DynamicValue& val)
  {
    GenericValue* m = (GenericValue*)ptr;
    return m->type->toValue(m->value, val);
  }
  static void* fromValue(const qi::detail::DynamicValue& val)
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
  static Type* type = 0;
  if (!type)
    type = typeOf<typename boost::remove_const<T>::type>();
  GenericValue res;
  res.type = type;
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

inline Type::Kind GenericValue::kind() const
{
 if (!type)
   return Type::Void;
 else
   return type->kind();
}


// Kind -> handler Type (TypeInt, TypeList...)  accessor

class KindNotConvertible;

template<Type::Kind T> struct TypeOfKind
{
  typedef KindNotConvertible type;
};

#define TYPE_OF_KIND(k, t) template<> struct TypeOfKind<k> { typedef t type;}

TYPE_OF_KIND(Type::Int, TypeInt);
TYPE_OF_KIND(Type::Float,  TypeFloat);
TYPE_OF_KIND(Type::String, TypeString);

#undef TYPE_OF_KIND


// Type -> Kind  accessor

namespace detail
{
  struct Nothing {};
  template<Type::Kind k> struct MakeKind
  {
    static const Type::Kind value = k;
  };

  template<typename C, typename T, typename F> struct IfElse
  {
  };

  template<typename T, typename F> struct IfElse<boost::true_type, T, F>
  {
    typedef T type;
  };

  template<typename T, typename F> struct IfElse<boost::false_type, T, F>
  {
    typedef F type;
  };

}

#define IF(cond, type) \
public detail::IfElse<cond, typename detail::MakeKind<type>, detail::Nothing>::type

template<typename T> struct KindOfType
: IF(typename boost::is_integral<T>::type, Type::Int)
, IF(typename boost::is_floating_point<T>::type, Type::Float)
{
};

#undef IF

template<typename T, Type::Kind k>
inline T GenericValue::as() const
{
  if (kind() != k)
  {
    qiLogWarning("qi.GenericValue") << "as: type " << kind() <<" not convertible to kind " << k;
    return T();
  }
  return dynamic_cast<const typename TypeOfKind<k>::type*>(type)->get(value);
}

template<typename T>
inline T GenericValue::as() const
{
  return as<T, KindOfType<T>::value>();
}

inline int64_t GenericValue::asInt() const
{
  return as<int64_t, Type::Int>();
}

inline float GenericValue::asFloat() const
{
  return as<float, Type::Float>();
}

inline double GenericValue::asDouble() const
{
  return as<double, Type::Float>();
}


inline std::string GenericValue::asString() const
{
  return as<std::string, Type::String>();
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
