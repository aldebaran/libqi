#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_GENERICVALUESPECIALIZED_HXX_
#define _QITYPE_DETAILS_GENERICVALUESPECIALIZED_HXX_

namespace qi
{

template<typename T>
inline void GenericIteratorPtr<T>::operator ++()
{
  static_cast<TypeIterator<T>*>(type)->next(&value);
}

template<typename T>
inline void GenericIteratorPtr<T>::operator ++(int)
{
  static_cast<TypeIterator<T>*>(type)->next(&value);
}

template<typename T>
inline T GenericIteratorPtr<T>::operator *()
{
  return static_cast<TypeIterator<T>*>(type)->dereference(value);
}

template<typename T>
inline bool GenericIteratorPtr<T>::operator ==(const GenericIteratorPtr& b) const
{
  // Assume both iterators come from the same container instance
  // in which case they have the same type*
  if (type != b.type)
    return false;
  return static_cast<TypeIterator<T>*>(type)->equals(value, b.value);
}

template<typename T>
inline bool  GenericIteratorPtr<T>::operator !=(const GenericIteratorPtr& b) const
{
  return ! (*this==b);
}

inline GenericListPtr::GenericListPtr()
: GenericValuePtr()
{}

inline GenericListPtr::GenericListPtr(GenericValuePtr& v)
: GenericValuePtr(v)
{}

inline GenericListPtr::GenericListPtr(TypeList* type, void* value)
: GenericValuePtr(type, value)
{}


inline GenericListIteratorPtr GenericListPtr::begin()
{
  return static_cast<TypeList*>(type)->begin(value);
}

inline GenericListIteratorPtr GenericListPtr::end()
{
  return static_cast<TypeList*>(type)->end(value);
}

inline Type* GenericListPtr::elementType()
{
  return static_cast<TypeList*>(type)->elementType();
}

inline size_t GenericListPtr::size()
{
  return static_cast<TypeList*>(type)->size(value);
}

inline void GenericListPtr::pushBack(GenericValuePtr val)
{
  if (val.type == elementType())
  { // False negative is ok, will make a dummy convert
    static_cast<TypeList*>(type)->pushBack(&value, val.value);
  }
  else
  {
    std::pair<GenericValuePtr, bool> conv = val.convert(
      static_cast<TypeList*>(type)->elementType());
    static_cast<TypeList*>(type)->pushBack(&value, conv.first.value);
    if (conv.second)
      conv.first.destroy();
  }
}

inline GenericListPtr GenericValuePtr::asList() const
{
  GenericListPtr result;
  result.type = dynamic_cast<TypeList*>(type);
  if (result.type)
    result.value = value;
  return result;
}


inline GenericMapPtr::GenericMapPtr()
: GenericValuePtr()
{}

inline GenericMapPtr::GenericMapPtr(GenericValuePtr& v)
: GenericValuePtr(v)
{}

inline GenericMapPtr::GenericMapPtr(TypeMap* type, void* value)
: GenericValuePtr(type, value)
{}


inline GenericMapIteratorPtr GenericMapPtr::begin()
{
  return static_cast<TypeMap*>(type)->begin(value);
}

inline GenericMapIteratorPtr GenericMapPtr::end()
{
  return static_cast<TypeMap*>(type)->end(value);
}

inline size_t GenericMapPtr::size()
{
  return static_cast<TypeMap*>(type)->size(value);
}

inline Type* GenericMapPtr::keyType()
{
  return static_cast<TypeMap*>(type)->keyType();
}

inline Type* GenericMapPtr::elementType()
{
  return static_cast<TypeMap*>(type)->elementType();
}

inline void GenericMapPtr::insert(GenericValuePtr key, GenericValuePtr val)
{
  std::pair<GenericValuePtr, bool> ck(key, false);
  std::pair<GenericValuePtr, bool> cv(val, false);
  if (key.type != keyType())
    ck = key.convert(keyType());
  if (val.type != elementType())
    cv = val.convert(elementType());

  static_cast<TypeMap*>(type)->insert(&value, ck.first.value, cv.first.value);
  if (ck.second)
    ck.first.destroy();
  if (cv.second)
    cv.first.destroy();
}


inline GenericMapPtr GenericValuePtr::asMap() const
{
  GenericMapPtr result;
  result.type = dynamic_cast<TypeMap*>(type);
  if (result.type)
    result.value = value;
  return result;
}

}
#endif  // _QITYPE_DETAILS_GENERICVALUESPECIALIZED_HXX_
