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
template<typename M> class MapTypeInterfaceImpl:
public MapTypeInterface
{
public:
  typedef DefaultTypeImplMethods<M,
                               TypeByPointer<M>
                               > MethodsImpl;
  MapTypeInterfaceImpl();
  virtual TypeInterface* elementType() const;
  virtual TypeInterface* keyType() const;
  virtual size_t size(void* storage);
  virtual AnyIterator begin(void* storage);
  virtual AnyIterator end(void* storage);
  virtual void insert(void** storage, void* keyStorage, void* valueStorage);
  virtual AnyReference element(void** storage, void* keyStorage, bool autoInsert);
  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
};

}

namespace qi {
template<typename M>
MapTypeInterfaceImpl<M>::MapTypeInterfaceImpl()
{
}


template<typename M> TypeInterface*
MapTypeInterfaceImpl<M>::elementType() const
{
  static TypeInterface* result = typeOf<typename M::mapped_type>();
  return result;
}

template<typename M> TypeInterface*
MapTypeInterfaceImpl<M>::keyType() const
{
  static TypeInterface* result = typeOf<typename M::key_type>();
  return result;
}

template<typename M> size_t
MapTypeInterfaceImpl<M>::size(void* storage)
{
  M* ptr = (M*)ptrFromStorage(&storage);
  return ptr->size();
}


template<typename M> AnyIterator
MapTypeInterfaceImpl<M>::begin(void* storage)
{
  M* ptr = (M*)ptrFromStorage(&storage);
  return TypeSimpleIteratorImpl<typename M::iterator>::make(ptr->begin());
}

template<typename M> AnyIterator
MapTypeInterfaceImpl<M>::end(void* storage)
{
  M* ptr = (M*)ptrFromStorage(&storage);
  return TypeSimpleIteratorImpl<typename M::iterator>::make(ptr->end());
}

template<typename M> void
MapTypeInterfaceImpl<M>::insert(void** storage, void* keyStorage, void* valueStorage)
{
  static TypeInterface* elemType = typeOf<typename M::mapped_type>();
  static TypeInterface* keyType = typeOf<typename M::key_type>();
  M* ptr = (M*) ptrFromStorage(storage);
  typename M::key_type& key = *(typename M::key_type*)keyType->ptrFromStorage(&keyStorage);
  typename M::mapped_type& val = *(typename M::mapped_type*)elemType->ptrFromStorage(&valueStorage);
  typename M::iterator it = ptr->find(key);
  if (it == ptr->end())
    ptr->insert(std::make_pair(key, val));
  else
    it->second = val;
}

template<typename M> AnyReference
MapTypeInterfaceImpl<M>::element(void** storage, void* keyStorage, bool autoInsert)
{
  //static TypeInterface* elemType = typeOf<typename M::mapped_type>();
  static TypeInterface* keyType = typeOf<typename M::key_type>();
  M* ptr = (M*) ptrFromStorage(storage);
  typename M::key_type* key = (typename M::key_type*)keyType->ptrFromStorage(&keyStorage);
  typename M::iterator it = ptr->find(*key);
  if (it == ptr->end())
  {
    if (!autoInsert)
      return AnyReference();
    typename M::mapped_type& e = (*ptr)[*key];
    return AnyReference(e);
  }
  else
    return AnyReference(((typename M::mapped_type&)(it->second)));
}



template<typename K, typename V, typename C, typename A>
struct TypeImpl<std::map<K,V, C, A> >: public MapTypeInterfaceImpl<std::map<K, V,C,A> > {};

}
#endif  // _QITYPE_DETAILS_TYPEMAP_HXX_
