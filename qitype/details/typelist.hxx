#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_TYPELIST_HXX_
#define _QITYPE_DETAILS_TYPELIST_HXX_

namespace qi
{
  // List container
template<typename T> class TypeListImpl:
public TypeList
{
public:
  typedef DefaultTypeImplMethods<T,
                               TypeByPointer<T>
                               > MethodsImpl;
  TypeListImpl();
  virtual size_t size(void* storage);
  virtual Type* elementType() const;
  virtual GenericIterator begin(void* storage);
  virtual GenericIterator end(void* storage);
  virtual void pushBack(void** storage, void* valueStorage);
  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
};


template<typename C> class TypeIteratorImpl
: public TypeIterator
{
public:
  typedef typename detail::TypeImplMethodsBySize<typename C::iterator>::type
  TypeImpl;
  typedef typename C::iterator Iterator;
  virtual GenericValueRef dereference(void* storage)
  {
    Iterator& i = *(Iterator*)ptrFromStorage(&storage);
    // Here we assume *i is a ref, ie not returned on the stack
    // It seems true for lists and maps
    return GenericValueRef(*i);
  }
  virtual void  next(void** storage)
  {
    Iterator& i = *(Iterator*)ptrFromStorage(storage);
    ++i;
  }
  virtual bool equals(void* s1, void* s2)
  {
    Iterator& i1 = *(Iterator*)ptrFromStorage(&s1);
    Iterator& i2 = *(Iterator*)ptrFromStorage(&s2);
    return i1 == i2;
  }
  _QI_BOUNCE_TYPE_METHODS(TypeImpl);
};


template<typename T>
TypeListImpl<T>::TypeListImpl()
{
  // register our iterator type
  registerType(typeid(typename T::iterator), new TypeIteratorImpl<T>());
}

template<typename T> Type*
TypeListImpl<T>::elementType() const
{
  static Type* result = typeOf<typename T::value_type>();
  return result;
}

template<typename T> GenericIterator
TypeListImpl<T>::begin(void* storage)
{
  T* ptr = (T*)ptrFromStorage(&storage);
  return GenericValue::from(ptr->begin());
}

template<typename T> GenericIterator
TypeListImpl<T>::end(void* storage)
{
  T* ptr = (T*)ptrFromStorage(&storage);
  return GenericValue::from(ptr->end());
}

template<typename T> void
TypeListImpl<T>::pushBack(void** storage, void* valueStorage)
{
  static Type* elemType = typeOf<typename T::value_type>();
  T* ptr = (T*) ptrFromStorage(storage);
  ptr->push_back(*(typename T::value_type*)elemType->ptrFromStorage(&valueStorage));
}

template<typename T> size_t
TypeListImpl<T>::size(void* storage)
{
  T* ptr = (T*) ptrFromStorage(&storage);
  return ptr->size();
}

// There is no way to register a template container type :(
template<typename T> struct TypeImpl<std::vector<T> >: public TypeListImpl<std::vector<T> > {};
template<typename T> struct TypeImpl<std::list<T> >: public TypeListImpl<std::list<T> > {};
}

#endif  // _QITYPE_DETAILS_TYPELIST_HXX_
