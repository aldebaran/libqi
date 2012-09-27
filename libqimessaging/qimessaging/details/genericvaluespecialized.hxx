#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#ifndef _QIMESSAGING_GENERICVALUESPECIALIZED_HXX_
#define _QIMESSAGING_GENERICVALUESPECIALIZED_HXX_

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

}
#endif
