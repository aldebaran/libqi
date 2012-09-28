#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPELIST_HXX_
#define _QIMESSAGING_TYPELIST_HXX_

namespace qi
{
  // List container
template<template<typename U> class C, typename T> class TypeListImpl:
public TypeList,
public DefaultTypeImplMethods<typename C<T>::type,
                               TypeDefaultAccess<typename C<T>::type >,
                               TypeDefaultClone<TypeDefaultAccess<typename C<T>::type > >,
                               TypeDefaultSerialize<TypeDefaultAccess<typename C<T>::type > >
                               >
{
public:
  typedef DefaultTypeImplMethods<typename C<T>::type,
                               TypeDefaultAccess<typename C<T>::type >,
                               TypeDefaultClone<TypeDefaultAccess<typename C<T>::type > >,
                               TypeDefaultSerialize<TypeDefaultAccess<typename C<T>::type > >
                               > MethodsImpl;
  TypeListImpl();
  virtual Type* elementType(void* storage) const;
  virtual GenericListIterator begin(void* storage);
  virtual GenericListIterator end(void* storage);
  virtual void pushBack(void* storage, void* valueStorage);
  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
};

// list iterator
template<typename C> class TypeListIteratorImpl
: public TypeListIterator
, public  detail::TypeImplMethodsBySize<typename C::iterator, detail::TypeAutoClone, TypeNoSerialize>::type
{
public:
  typedef typename detail::TypeImplMethodsBySize<typename C::iterator, detail::TypeAutoClone, TypeNoSerialize>::type
  TypeImpl;
  virtual GenericValue dereference(void* storage);
  virtual void  next(void** storage);
  virtual bool equals(void* s1, void* s2);
  _QI_BOUNCE_TYPE_METHODS(TypeImpl);
};


}
#include <qimessaging/genericvaluespecialized.hpp>
namespace qi {
template<template<typename U> class C, typename T>
TypeListImpl<C, T>::TypeListImpl()
{
  // register our iterator type
  registerType(typeid(typename C<T>::iterator), new TypeListIteratorImpl<C<T> >());
}


template<template<typename U> class C, typename T> Type*
TypeListImpl<C, T>::elementType(void*) const
{
  static Type* result = typeOf<T>();
  return result;
}

template<template<typename U> class C, typename T> GenericListIterator
TypeListImpl<C, T>::begin(void* storage)
{
  static Type* iterType = typeOf<typename C<T>::iterator>();
  C<T>* ptr = (C<T>*)ptrFromStorage(&storage);
  // ptr->begin() gives us an iterator on the stack.
  // So we need to clone it. Hopefuly sizeof iterator is small, so it fits in
  // a byvalue GenericValue
  GenericListIterator result;
  GenericValue val;
  val.type = iterType;
  typename C<T>::iterator res = ptr->begin(); // do not inline below!
  val.value = iterType->initializeStorage(&res);
  *(GenericValue*)&result = val.clone();
  return result;
}

template<template<typename U> class C, typename T> GenericListIterator
TypeListImpl<C, T>::end(void* storage)
{
  static Type* iterType = typeOf<typename C<T>::iterator>();
  C<T>* ptr = (C<T>*)ptrFromStorage(&storage);
  GenericListIterator result;
  GenericValue val;
  val.type = iterType;
  typename C<T>::iterator res = ptr->end(); // do not inline below!
  val.value = iterType->initializeStorage(&res);
  *(GenericValue*)&result = val.clone();
  return result;
}

template<template<typename U> class C, typename T> void
TypeListImpl<C, T>::pushBack(void* storage, void* valueStorage)
{
  static Type* elemType = typeOf<T>();
  C<T>* ptr = (C<T>*) ptrFromStorage(&storage);
  ptr->push_back(*(T*)elemType->ptrFromStorage(&valueStorage));
}


template<typename C> GenericValue TypeListIteratorImpl<C>::dereference(void* storage)
{
  typename C::iterator* ptr = (typename C::iterator*)ptrFromStorage(&storage);
  typename C::value_type& val = **ptr;
  return ::qi::toValue(val); // Value is in the container, no need to clone
}

template<typename C> void TypeListIteratorImpl<C>::next(void** storage)
{
  typename C::iterator* ptr = (typename C::iterator*)TypeImpl::Access::ptrFromStorage(storage);
  ++(*ptr);
}

template<typename C>  bool TypeListIteratorImpl<C>::equals(void* s1, void* s2)
{
  typename C::iterator* ptr1 = (typename C::iterator*)TypeImpl::Access::ptrFromStorage(&s1);
  typename C::iterator* ptr2 = (typename C::iterator*)TypeImpl::Access::ptrFromStorage(&s2);
  return *ptr1 == *ptr2;
}


// Because of the allocator template arg, std::vector does not match
// "template<typename U> typename C"
// However our TypeImpl must appear as std::vector<T> or we will have to
// override a lot of stuffs for our custom type (signature), so set a
// 'type' typedef and use in TypeListImpl
template<typename T> struct vector1: public std::vector<T>
{
  typedef std::vector<T> type;
};
template<typename T> struct list1: public std::list<T>
{
  typedef std::list<T> type;
};

template<typename T> struct tvector1: public std::vector<T>{};
template<typename T> struct tlist1: public std::list<T>{};
// There is no way to register a template container type :(
template<typename T> struct TypeImpl<std::vector<T> >: public TypeListImpl<vector1, T> {};
template<typename T> struct TypeImpl<std::list<T> >: public TypeListImpl<list1, T> {};
}

#endif
