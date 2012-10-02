#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DETAILS_GENERICVALUE_HXX_
#define _QIMESSAGING_DETAILS_GENERICVALUE_HXX_

#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_floating_point.hpp>

#include <qimessaging/typespecialized.hpp>

namespace qi {


template<> class TypeImpl<GenericValue>:
  public DefaultTypeImpl<
    GenericValue,
    TypeByPointer<GenericValue>
    > {
      virtual void* clone(void* src)
      {
        return new GenericValue(((GenericValue*)src)->clone());
      }
      virtual void destroy(void* ptr)
      {
        ((GenericValue*)ptr)->destroy();
        delete (GenericValue*)ptr;
      }
      virtual Kind kind() const
      {
        return Dynamic;
      }
    };

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

inline GenericValue::GenericValue()
: type(0)
, value(0)
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

namespace detail {

  // Optimized GenericValue::as<T> for direct access to a subType getter
  template<typename T, Type::Kind k>
  inline T valueAs(const GenericValue& v)
  {
    if (v.kind() == k)
      return static_cast<typename TypeOfKind<k>::type* const>(v.type)->get(v.value);
    // Fallback to default which will attempt a full conversion.
    return v.as<T>();
  }
}

template<typename T>
inline T* GenericValue::ptr(bool check)
{
  if (check && typeOf<T>()->info() != type->info())
    return 0;
  else
    return (T*)type->ptrFromStorage(&value);
}

template<typename T>
inline T GenericValue::as(const T&) const
{
  return as<T>();
}

template<typename T>
inline T GenericValue::as() const
{
  std::pair<GenericValue, bool> conv = convert(typeOf<T>());
  if (!conv.first.type)
  {
    qiLogWarning("qi.GenericValue") << "Conversion from " << type->infoString()
    << " to " << typeOf<T>()->infoString() << " failed";
    return T();
  }
  T result = *conv.first.ptr<T>(false);
  if (conv.second)
    conv.first.destroy();
  return result;
}

inline int64_t GenericValue::asInt() const
{
  return detail::valueAs<int64_t, Type::Int>(*this);
}

inline float GenericValue::asFloat() const
{
  return detail::valueAs<float, Type::Float>(*this);
}

inline double GenericValue::asDouble() const
{
  return detail::valueAs<double, Type::Float>(*this);
}


inline std::string GenericValue::asString() const
{
  return as<std::string>();
}

inline void GenericValue::serialize(ODataStream& out) const
{
  type->serialize(out, value);
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
