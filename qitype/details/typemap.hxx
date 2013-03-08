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
  virtual Type* elementType() const;
  virtual Type* keyType() const;
  virtual size_t size(void* storage);
  virtual GenericIterator begin(void* storage);
  virtual GenericIterator end(void* storage);
  virtual void insert(void** storage, void* keyStorage, void* valueStorage);
  virtual GenericValuePtr element(void** storage, void* keyStorage, bool autoInsert);
  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
};

}

namespace qi {
template<typename M>
TypeMapImpl<M>::TypeMapImpl()
{
  // register our iterator type
  registerType(typeid(typename M::iterator), new TypeIteratorImpl<M>());
}


template<typename M> Type*
TypeMapImpl<M>::elementType() const
{
  static Type* result = typeOf<typename M::mapped_type>();
  return result;
}

template<typename M> Type*
TypeMapImpl<M>::keyType() const
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


template<typename M> GenericIterator
TypeMapImpl<M>::begin(void* storage)
{
  M* ptr = (M*)ptrFromStorage(&storage);
  return GenericValue::from(ptr->begin());
}

template<typename M> GenericIterator
TypeMapImpl<M>::end(void* storage)
{
  M* ptr = (M*)ptrFromStorage(&storage);
  return GenericValue::from(ptr->end());
}

template<typename M> void
TypeMapImpl<M>::insert(void** storage, void* keyStorage, void* valueStorage)
{
  static Type* elemType = typeOf<typename M::mapped_type>();
  static Type* keyType = typeOf<typename M::key_type>();
  M* ptr = (M*) ptrFromStorage(storage);
  typename M::key_type& key = *(typename M::key_type*)keyType->ptrFromStorage(&keyStorage);
  typename M::mapped_type& val = *(typename M::mapped_type*)elemType->ptrFromStorage(&valueStorage);
  typename M::iterator it = ptr->find(key);
  if (it == ptr->end())
    ptr->insert(std::make_pair(key, val));
  else
    it->second = val;
}

template<typename M> GenericValuePtr
TypeMapImpl<M>::element(void** storage, void* keyStorage, bool autoInsert)
{
  //static Type* elemType = typeOf<typename M::mapped_type>();
  static Type* keyType = typeOf<typename M::key_type>();
  M* ptr = (M*) ptrFromStorage(storage);
  typename M::key_type* key = (typename M::key_type*)keyType->ptrFromStorage(&keyStorage);
  typename M::iterator it = ptr->find(*key);
  if (it == ptr->end())
  {
    if (!autoInsert)
      return GenericValuePtr();
    typename M::mapped_type& e = (*ptr)[*key];
    return GenericValuePtr(&e);
  }
  else
    return GenericValuePtr(&((typename M::mapped_type&)(it->second)));
}



template<typename K, typename V, typename C, typename A>
struct TypeImpl<std::map<K,V, C, A> >: public TypeMapImpl<std::map<K, V,C,A> > {};

}
#endif  // _QITYPE_DETAILS_TYPEMAP_HXX_
