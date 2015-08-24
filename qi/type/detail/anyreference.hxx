#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_DETAIL_ANYREFERENCE_HXX_
#define _QI_TYPE_DETAIL_ANYREFERENCE_HXX_

#include <type_traits>

namespace qi
{

namespace detail
{

inline AnyReference AnyReferenceBase::clone() const
{
  AnyReference res;
  res._type  = _type;
  res._value = _type ? res._type->clone(_value) : 0;
  return res;
}

inline qi::Signature AnyReferenceBase::signature(bool resolveDynamic) const
{
  if (!_type)
    return qi::Signature();
  else
    return _type->signature(_value, resolveDynamic);
}

inline void AnyReferenceBase::destroy()
{
  if (_type)
    _type->destroy(_value);
  _value = _type = 0;
}

inline AnyReferenceBase::AnyReferenceBase()
  : _type(0)
  , _value(0)
{
}

inline AnyReferenceBase::AnyReferenceBase(TypeInterface* type)
  : _type(type)
  , _value(type->initializeStorage())
{
}

template<typename T>
AnyReference AnyReferenceBase::fromPtr(const T* ptr)
{
  static TypeInterface* t = 0;
  if (!t)
    t = typeOf<typename std::remove_const<T>::type>();
  void *value = t->initializeStorage(const_cast<void*>((const void*)ptr));
  return AnyReference(t, value);
}

template<typename T>
AnyReference AnyReferenceBase::from(const T& ptr)
{
  AnyReference ref;
  static TypeInterface* t = 0;
  QI_ONCE( t = typeOf<typename std::remove_const<T>::type>());
  return AnyReference(t, t->initializeStorage(const_cast<void*>((const void*)&ptr)));
}

inline TypeKind AnyReferenceBase::kind() const
{
  if (!_type)
    throw std::runtime_error("Can't take the kind of an invalid value");
  else
    return _type->kind();
}

template<TypeKind T> struct TypeOfKind {};

#define TYPE_OF_KIND(k, t) template<> struct TypeOfKind<k> { typedef t type;}
// Kind -> handler Type (IntTypeInterface, ListTypeInterface...)  accessor
TYPE_OF_KIND(TypeKind_Int, IntTypeInterface);
TYPE_OF_KIND(TypeKind_Float, FloatTypeInterface);
TYPE_OF_KIND(TypeKind_String, StringTypeInterface);
#undef TYPE_OF_KIND

// Optimized AnyReferenceBase::as<T> for direct access to a subType getter
template<typename T, TypeKind k>
inline T valueAs(const AnyReferenceBase& v)
{
  if (v.kind() == k)
    return static_cast<T>(
      static_cast<typename TypeOfKind<k>::type*>(v.type())
        ->get(v.rawValue()));
  // Fallback to default which will attempt a full conversion.
  return v.to<T>();
}

template<typename T>
inline T* AnyReferenceBase::ptr(bool check)
{
  if (!_type || (check && typeOf<T>()->info() != _type->info()))
    return 0;
  else
    return (T*)_type->ptrFromStorage(&_value);
}

template<typename T>
inline T& AnyReferenceBase::as()
{
  T* p = ptr<T>(true);
  if (!p)
    throw std::runtime_error("Type mismatch");
  return *p;
}

template<typename T>
inline T AnyReferenceBase::to(const T&) const
{
  return to<T>();
}

QI_NORETURN QI_API void throwConversionFailure(
    TypeInterface* from, TypeInterface* to);

template<typename T>
inline T AnyReferenceBase::to() const
{
  TypeInterface* targetType = typeOf<T>();
  std::pair<AnyReference, bool> conv = convert(targetType);
  if (!conv.first._type)
  {
    throwConversionFailure(_type, targetType);
  }
  T result = *conv.first.ptr<T>(false);
  if (conv.second)
    conv.first.destroy();
  return result;
}

template<>
inline void AnyReferenceBase::to<void>() const
{
  return;
}

inline bool AnyReferenceBase::isValid() const
{
  return _type != 0;
}

inline bool AnyReferenceBase::isValue() const
{
  return _type != 0 && _type->info() != typeOf<void>()->info();
}

inline int64_t AnyReferenceBase::toInt() const
{
  return detail::valueAs<int64_t, TypeKind_Int>(*this);
}

inline uint64_t AnyReferenceBase::toUInt() const
{
  return detail::valueAs<uint64_t, TypeKind_Int>(*this);
}

inline float AnyReferenceBase::toFloat() const
{
  return detail::valueAs<float, TypeKind_Float>(*this);
}

inline double AnyReferenceBase::toDouble() const
{
  return detail::valueAs<double, TypeKind_Float>(*this);
}

inline std::string AnyReferenceBase::toString() const
{
  return to<std::string>();
}

template<typename T>
inline std::vector<T> AnyReferenceBase::toList() const
{
  return to<std::vector<T> >();
}

template<typename K, typename V>
inline std::map<K, V> AnyReferenceBase::toMap() const
{
  return to<std::map<K, V> >();
}

inline AnyReferenceVector AnyReferenceBase::asListValuePtr()
{
  return asTupleValuePtr();
}

template<typename T>
void AnyReferenceBase::set(const T& v)
{
  update(AnyReferenceBase::from(v));
 }

inline void AnyReferenceBase::setFloat(float v)
{
  setDouble(static_cast<double>(v));
}

template<typename E, typename K>
E& AnyReferenceBase::element(const K& key)
{
  return (*this)[key].template as<E>();
}

template<typename K>
AnyReference AnyReferenceBase::operator[](const K& key)
{
  return _element(AnyReferenceBase::from(key), true);
}

template<typename T>
void AnyReferenceBase::append(const T& element)
{
  _append(AnyReference::from(element));
}

template<typename K, typename V>
void AnyReferenceBase::insert(const K& key, const V& val)
{
  _insert(AnyReference::from(key), AnyReference::from(val));
}

template<typename K>
AnyReference AnyReferenceBase::find(const K& key)
{
  return _element(AnyReference::from(key), false);
}

} // namespace detail

inline bool operator != (const AnyReference& a, const AnyReference& b)
{
  return !(a==b);
}

} // namespace qi

#endif
