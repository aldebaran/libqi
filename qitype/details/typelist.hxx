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
  virtual Type* elementType(void* storage) const;
  virtual GenericListIteratorPtr begin(void* storage);
  virtual GenericListIteratorPtr end(void* storage);
  virtual void pushBack(void* storage, void* valueStorage);
  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
};

// list iterator
template<typename C> class TypeListIteratorImpl
: public TypeListIterator
{
public:
  typedef typename detail::TypeImplMethodsBySize<typename C::iterator>::type
  TypeImpl;
  virtual GenericValuePtr dereference(void* storage);
  virtual void  next(void** storage);
  virtual bool equals(void* s1, void* s2);
  _QI_BOUNCE_TYPE_METHODS(TypeImpl);
};

template<typename T>
TypeListImpl<T>::TypeListImpl()
{
  // register our iterator type
  registerType(typeid(typename T::iterator), new TypeListIteratorImpl<T>());
}

template<typename T> Type*
TypeListImpl<T>::elementType(void*) const
{
  static Type* result = typeOf<typename T::value_type>();
  return result;
}

template<typename T> GenericListIteratorPtr
TypeListImpl<T>::begin(void* storage)
{
  static Type* iterType = typeOf<typename T::iterator>();
  T* ptr = (T*)ptrFromStorage(&storage);
  // ptr->begin() gives us an iterator on the stack.
  // So we need to clone it. Hopefuly sizeof iterator is small, so it fits in
  // a byvalue GenericValuePtr
  GenericListIteratorPtr result;
  GenericValuePtr val;
  val.type = iterType;
  typename T::iterator res = ptr->begin(); // do not inline below!
  val.value = iterType->initializeStorage(&res);
  *(GenericValuePtr*)&result = val.clone();
  return result;
}

template<typename T> GenericListIteratorPtr
TypeListImpl<T>::end(void* storage)
{
  static Type* iterType = typeOf<typename T::iterator>();
  T* ptr = (T*)ptrFromStorage(&storage);
  GenericListIteratorPtr result;
  GenericValuePtr val;
  val.type = iterType;
  typename T::iterator res = ptr->end(); // do not inline below!
  val.value = iterType->initializeStorage(&res);
  *(GenericValuePtr*)&result = val.clone();
  return result;
}

template<typename T> void
TypeListImpl<T>::pushBack(void* storage, void* valueStorage)
{
  static Type* elemType = typeOf<typename T::value_type>();
  T* ptr = (T*) ptrFromStorage(&storage);
  ptr->push_back(*(typename T::value_type*)elemType->ptrFromStorage(&valueStorage));
}

template<typename T> size_t
TypeListImpl<T>::size(void* storage)
{
  T* ptr = (T*) ptrFromStorage(&storage);
  return ptr->size();
}

template<typename C> GenericValuePtr TypeListIteratorImpl<C>::dereference(void* storage)
{
  typename C::iterator* ptr = (typename C::iterator*)ptrFromStorage(&storage);
  typename C::value_type& val = **ptr;
  return ::qi::GenericValuePtr::from(val); // Value is in the container, no need to clone
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

// There is no way to register a template container type :(
template<typename T> struct TypeImpl<std::vector<T> >: public TypeListImpl<std::vector<T> > {};
template<typename T> struct TypeImpl<std::list<T> >: public TypeListImpl<std::list<T> > {};
}

#endif  // _QITYPE_DETAILS_TYPELIST_HXX_
