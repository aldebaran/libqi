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
inline void GenericIterator<T>::operator ++()
{
  static_cast<TypeIterator<T>*>(type)->next(&value);
}

template<typename T>
inline void GenericIterator<T>::operator ++(int)
{
  static_cast<TypeIterator<T>*>(type)->next(&value);
}

template<typename T>
inline T GenericIterator<T>::operator *()
{
  return static_cast<TypeIterator<T>*>(type)->dereference(value);
}

template<typename T>
inline bool GenericIterator<T>::operator ==(const GenericIterator& b) const
{
  // Assume both iterators come from the same container instance
  // in which case they have the same type*
  if (type != b.type)
    return false;
  return static_cast<TypeIterator<T>*>(type)->equals(value, b.value);
}

template<typename T>
inline bool  GenericIterator<T>::operator !=(const GenericIterator& b) const
{
  return ! (*this==b);
}

inline GenericList::GenericList()
: GenericValue()
{}

inline GenericList::GenericList(GenericValue& v)
: GenericValue(v)
{}

inline GenericList::GenericList(TypeList* type, void* value)
: GenericValue(type, value)
{}


inline GenericListIterator GenericList::begin()
{
  return static_cast<TypeList*>(type)->begin(value);
}

inline GenericListIterator GenericList::end()
{
  return static_cast<TypeList*>(type)->end(value);
}

inline Type* GenericList::elementType()
{
  return static_cast<TypeList*>(type)->elementType(value);
}

inline size_t GenericList::size()
{
  return static_cast<TypeList*>(type)->size(value);
}

inline void GenericList::pushBack(GenericValue val)
{
  if (val.type == elementType())
  { // False negative is ok, will make a dummy convert
    static_cast<TypeList*>(type)->pushBack(value, val.value);
  }
  else
  {
    std::pair<GenericValue, bool> conv = val.convert(
      static_cast<TypeList*>(type)->elementType(value));
    static_cast<TypeList*>(type)->pushBack(value, conv.first.value);
    if (conv.second)
      conv.first.destroy();
  }
}

inline GenericList GenericValue::asList() const
{
  GenericList result;
  result.type = dynamic_cast<TypeList*>(type);
  if (result.type)
    result.value = value;
  return result;
}


inline GenericMap::GenericMap()
: GenericValue()
{}

inline GenericMap::GenericMap(GenericValue& v)
: GenericValue(v)
{}

inline GenericMap::GenericMap(TypeMap* type, void* value)
: GenericValue(type, value)
{}


inline GenericMapIterator GenericMap::begin()
{
  return static_cast<TypeMap*>(type)->begin(value);
}

inline GenericMapIterator GenericMap::end()
{
  return static_cast<TypeMap*>(type)->end(value);
}

inline size_t GenericMap::size()
{
  return static_cast<TypeMap*>(type)->size(value);
}

inline Type* GenericMap::keyType()
{
  return static_cast<TypeMap*>(type)->keyType(value);
}

inline Type* GenericMap::elementType()
{
  return static_cast<TypeMap*>(type)->elementType(value);
}

inline void GenericMap::insert(GenericValue key, GenericValue val)
{
  std::pair<GenericValue, bool> ck(key, false);
  std::pair<GenericValue, bool> cv(val, false);
  if (key.type != keyType())
    ck = key.convert(keyType());
  if (val.type != elementType())
    cv = val.convert(elementType());

  static_cast<TypeMap*>(type)->insert(value, ck.first.value, cv.first.value);
  if (ck.second)
    ck.first.destroy();
  if (cv.second)
    cv.first.destroy();
}


inline GenericMap GenericValue::asMap() const
{
  GenericMap result;
  result.type = dynamic_cast<TypeMap*>(type);
  if (result.type)
    result.value = value;
  return result;
}

}
#endif  // _QITYPE_DETAILS_GENERICVALUESPECIALIZED_HXX_
