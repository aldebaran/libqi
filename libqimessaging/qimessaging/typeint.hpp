/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#ifndef _QI_MESSAGING_TYPEINT_HPP_
#define _QI_MESSAGING_TYPEINT_HPP_
#include <qimessaging/type.hpp>

namespace qi
{
  class GenericIterator;
class QIMESSAGING_API TypeInt: public Type
{
public:
  virtual int64_t get(void* value) const = 0;
  virtual void set(void** storage, int64_t value) = 0;
  virtual Kind kind() const { return Int;}
};

class QIMESSAGING_API TypeFloat: public Type
{
public:
  virtual double get(void* value) const = 0;
  virtual void set(void** storage, double value) = 0;
  virtual Kind kind() const { return Float;}
};

class QIMESSAGING_API TypeIterator: public Type
{
public:
  virtual GenericValue dereference(void* storage) = 0; // must not be destroyed
  virtual void  next(void** storage) = 0;
  virtual bool equals(void* s1, void* s2) = 0;
};

class QIMESSAGING_API TypeList: public Type
{
public:
  virtual Type* elementType(void* storage) const = 0;
  virtual GenericIterator begin(void* storage) = 0; // Must be destroyed
  virtual GenericIterator end(void* storage) = 0;  //idem
  virtual void pushBack(void* storage, void* valueStorage) = 0;
  virtual Kind kind() const { return List;}
};


// List container
template<template<typename U> class C, typename T> class TypeListImpl:
public TypeList,
public DefaultTypeImplMethods<typename C<T>::type,
                               TypeDefaultAccess<typename C<T>::type >,
                               TypeDefaultClone<TypeDefaultAccess<typename C<T>::type > >,
                               TypeDefaultValue<TypeDefaultAccess<typename C<T>::type > >,
                               TypeDefaultSerialize<TypeDefaultAccess<typename C<T>::type > >
                               >
{
public:
  typedef DefaultTypeImplMethods<typename C<T>::type,
                               TypeDefaultAccess<typename C<T>::type >,
                               TypeDefaultClone<TypeDefaultAccess<typename C<T>::type > >,
                               TypeDefaultValue<TypeDefaultAccess<typename C<T>::type > >,
                               TypeDefaultSerialize<TypeDefaultAccess<typename C<T>::type > >
                               > MethodsImpl;
  TypeListImpl();
  virtual Type* elementType(void* storage) const;
  virtual GenericIterator begin(void* storage);
  virtual GenericIterator end(void* storage);
  virtual void pushBack(void* storage, void* valueStorage);
  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
};

// container iterator
template<typename C> class TypeIteratorImpl
: public TypeIterator
, public  detail::TypeImplMethodsBySize<typename C::iterator, detail::TypeAutoClone, TypeNoValue, TypeNoSerialize>::type
{
public:
  typedef typename detail::TypeImplMethodsBySize<typename C::iterator, detail::TypeAutoClone, TypeNoValue, TypeNoSerialize>::type
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
  registerType(typeid(typename C<T>::iterator), new TypeIteratorImpl<C<T> >());
}


template<template<typename U> class C, typename T> Type*
TypeListImpl<C, T>::elementType(void*) const
{
  return typeOf<T>();
}

template<template<typename U> class C, typename T> GenericIterator
TypeListImpl<C, T>::begin(void* storage)
{
  C<T>* ptr = (C<T>*)ptrFromStorage(&storage);
  // ptr->begin() gives us an iterator on the stack.
  // So we need to clone it. Hopefuly sizeof iterator is small, so it fits in
  // a byvalue GenericValue
  return ::qi::toValue(ptr->begin()).clone().asIterator();
}

template<template<typename U> class C, typename T> GenericIterator
TypeListImpl<C, T>::end(void* storage)
{
  C<T>* ptr = (C<T>*)ptrFromStorage(&storage);
  return ::qi::toValue(ptr->end()).clone().asIterator();
}

template<template<typename U> class C, typename T> void
TypeListImpl<C, T>::pushBack(void* storage, void* valueStorage)
{
  C<T>* ptr = (C<T>*) ptrFromStorage(&storage);
  typedef typename C<T>::value_type ValueType;
  ptr->push_back(*(ValueType*)typeOf<ValueType>()->ptrFromStorage(&valueStorage));
}


template<typename C> GenericValue TypeIteratorImpl<C>::dereference(void* storage)
{
  typename C::iterator* ptr = (typename C::iterator*)ptrFromStorage(&storage);
  typename C::value_type& val = **ptr;
  return ::qi::toValue(val); // Value is in the container, no need to clone
}

template<typename C> void TypeIteratorImpl<C>::next(void** storage)
{
  typename C::iterator* ptr = (typename C::iterator*)TypeImpl::Access::ptrFromStorage(storage);
  ++(*ptr);
}

template<typename C>  bool TypeIteratorImpl<C>::equals(void* s1, void* s2)
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
