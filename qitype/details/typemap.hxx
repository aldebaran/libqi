#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_TYPEMAP_HXX_
#define _QITYPE_DETAILS_TYPEMAP_HXX_

namespace qi
{
  // List container
template<typename M> class TypeMapImpl:
public TypeMap
{
public:
  typedef DefaultTypeImplMethods<M,
                               TypeByPointer<M>
                               > MethodsImpl;
  TypeMapImpl();
  virtual Type* elementType(void* storage) const;
  virtual Type* keyType(void* storage) const;
  virtual size_t size(void* storage);
  virtual GenericMapIterator begin(void* storage);
  virtual GenericMapIterator end(void* storage);
  virtual void insert(void* storage, void* keyStorage, void* valueStorage);
  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
};

// map iterator
template<typename C> class TypeMapIteratorImpl
: public TypeMapIterator
{
public:
  typedef typename detail::TypeImplMethodsBySize<typename C::iterator>::type
  TypeImpl;
  virtual std::pair<GenericValue, GenericValue> dereference(void* storage);
  virtual void  next(void** storage);
  virtual bool equals(void* s1, void* s2);
  _QI_BOUNCE_TYPE_METHODS(TypeImpl);
};


}

namespace qi {
template<typename M>
TypeMapImpl<M>::TypeMapImpl()
{
  // register our iterator type
  registerType(typeid(typename M::iterator), new TypeMapIteratorImpl<M>());
}


template<typename M> Type*
TypeMapImpl<M>::elementType(void*) const
{
  static Type* result = typeOf<typename M::mapped_type>();
  return result;
}

template<typename M> Type*
TypeMapImpl<M>::keyType(void*) const
{
  static Type* result = typeOf<typename M::key_type>();
  return result;
}

template<typename M> size_t
TypeMapImpl<M>::size(void* storage)
{
  M* ptr = (M*)ptrFromStorage(&storage);
  return ptr->size();
}

template<typename M> GenericMapIterator
TypeMapImpl<M>::begin(void* storage)
{
  static Type* iterType = typeOf<typename M::iterator>();
  M* ptr = (M*)ptrFromStorage(&storage);
  // ptr->begin() gives us an iterator on the stack.
  // So we need to clone it. Hopefuly sizeof iterator is small, so it fits in
  // a byvalue GenericValue
  GenericMapIterator result;
  GenericValue val;
  val.type = iterType;
  typename M::iterator res = ptr->begin(); // do not inline below!
  val.value = iterType->initializeStorage(&res);
  *(GenericValue*)&result = val.clone();
  return result;
}

template<typename M> GenericMapIterator
TypeMapImpl<M>::end(void* storage)
{
  static Type* iterType = typeOf<typename M::iterator>();
  M* ptr = (M*)ptrFromStorage(&storage);
  GenericMapIterator result;
  GenericValue val;
  val.type = iterType;
  typename M::iterator res = ptr->end(); // do not inline below!
  val.value = iterType->initializeStorage(&res);
  *(GenericValue*)&result = val.clone();
  return result;
}

template<typename M> void
TypeMapImpl<M>::insert(void* storage, void* keyStorage, void* valueStorage)
{
  static Type* elemType = typeOf<typename M::mapped_type>();
  static Type* keyType = typeOf<typename M::key_type>();
  M* ptr = (M*) ptrFromStorage(&storage);
  ptr->insert(std::make_pair(
    *(typename M::key_type*)keyType->ptrFromStorage(&keyStorage),
    *(typename M::mapped_type*)elemType->ptrFromStorage(&valueStorage)));
}


template<typename C> std::pair<GenericValue, GenericValue> TypeMapIteratorImpl<C>::dereference(void* storage)
{
  typename C::iterator* ptr = (typename C::iterator*)ptrFromStorage(&storage);
  typename C::value_type& val = **ptr;
  return std::make_pair(::qi::toValue(val.first), ::qi::toValue(val.second)); // Value is in the container, no need to clone
}

template<typename C> void TypeMapIteratorImpl<C>::next(void** storage)
{
  typename C::iterator* ptr = (typename C::iterator*)TypeImpl::Access::ptrFromStorage(storage);
  ++(*ptr);
}

template<typename C>  bool TypeMapIteratorImpl<C>::equals(void* s1, void* s2)
{
  typename C::iterator* ptr1 = (typename C::iterator*)TypeImpl::Access::ptrFromStorage(&s1);
  typename C::iterator* ptr2 = (typename C::iterator*)TypeImpl::Access::ptrFromStorage(&s2);
  return *ptr1 == *ptr2;
}

template<typename K, typename V, typename C, typename A>
struct TypeImpl<std::map<K,V, C, A> >: public TypeMapImpl<std::map<K, V,C,A> > {};

}
#endif  // _QITYPE_DETAILS_TYPEMAP_HXX_
