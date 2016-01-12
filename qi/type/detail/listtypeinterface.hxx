#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_TYPELIST_HXX_
#define _QITYPE_DETAIL_TYPELIST_HXX_

#include <qi/atomic.hpp>

#include <qi/type/detail/anyreference.hpp>
#include <qi/type/detail/anyiterator.hpp>
#include <qi/anyfunction.hpp>

namespace qi
{
  // List container
template<typename T, typename H = ListTypeInterface>
class ListTypeInterfaceImpl: public H
{
public:
  using MethodsImpl = DefaultTypeImplMethods<T, TypeByPointerPOD<T>>;
  ListTypeInterfaceImpl();
  size_t size(void* storage) override;
  TypeInterface* elementType() override;
  AnyIterator begin(void* storage) override;
  AnyIterator end(void* storage) override;
  void pushBack(void** storage, void* valueStorage) override;
  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
  TypeInterface* _elementType;
};

// Type impl for any class that behaves as a forward iterator (++, *, ==)
template<typename T>
class TypeSimpleIteratorImpl: public IteratorTypeInterface
{
public:
  using Storage = T;
  AnyReference dereference(void* storage) override
  {
    T* ptr = (T*)ptrFromStorage(&storage);
    return AnyReference::from(*(*ptr));
  }
  void next(void** storage) override
  {
    T* ptr = (T*)ptrFromStorage(storage);
    ++(*ptr);
  }
  bool equals(void* s1, void* s2) override
  {
    T* p1 = (T*)ptrFromStorage(&s1);
    T* p2 = (T*)ptrFromStorage(&s2);
    return *p1 == *p2;
  }
  using TypeImpl = DefaultTypeImplMethods<Storage, TypeByPointerPOD<T>>;
  _QI_BOUNCE_TYPE_METHODS(TypeImpl);
  static AnyIterator make(const T& val)
  {
    static TypeSimpleIteratorImpl<T>* type = 0;
    QI_THREADSAFE_NEW(type);
    return AnyValue(AnyReference(type, type->initializeStorage(const_cast<void*>((const void*)&val))));
  }
};


template<typename T, typename H>
ListTypeInterfaceImpl<T, H>::ListTypeInterfaceImpl()
{
  _elementType = typeOf<typename T::value_type>();
}

template<typename T, typename H> TypeInterface*
ListTypeInterfaceImpl<T, H>::elementType()
{
  return _elementType;
}

template<typename T, typename H>
AnyIterator ListTypeInterfaceImpl<T, H>::begin(void* storage)
{
  T* ptr = (T*)ptrFromStorage(&storage);
  return TypeSimpleIteratorImpl<typename T::iterator>::make(ptr->begin());
}

template<typename T, typename H>
AnyIterator ListTypeInterfaceImpl<T, H>::end(void* storage)
{
  T* ptr = (T*)ptrFromStorage(&storage);
  return TypeSimpleIteratorImpl<typename T::iterator>::make(ptr->end());
}
namespace detail
{
  template<typename T, typename E>
  void pushBack(T& container, E* element)
  {
    container.push_back(*element);
  }
  template<typename CE, typename E>
  void pushBack(std::set<CE>& container, E* element)
  {
    container.insert(*element);
  }
}
template<typename T, typename H>
void ListTypeInterfaceImpl<T, H>::pushBack(void **storage, void* valueStorage)
{
  T* ptr = (T*) ptrFromStorage(storage);
  detail::pushBack(*ptr, (typename T::value_type*)_elementType->ptrFromStorage(&valueStorage));
}

template<typename T, typename H>
size_t ListTypeInterfaceImpl<T, H>::size(void* storage)
{
  T* ptr = (T*) ptrFromStorage(&storage);
  return ptr->size();
}

// There is no way to register a template container type :(
template<typename T> struct TypeImpl<std::vector<T> >: public ListTypeInterfaceImpl<std::vector<T> >
{
  static_assert(!boost::is_same<T,bool>::value, "std::vector<bool> is not supported by AnyValue.");
};
template<typename T> struct TypeImpl<std::list<T> >: public ListTypeInterfaceImpl<std::list<T> > {};
template<typename T> struct TypeImpl<std::set<T> >: public ListTypeInterfaceImpl<std::set<T> > {};


// varargs container
template<typename T>
class VarArgsTypeInterfaceImpl: public ListTypeInterfaceImpl<typename T::VectorType, VarArgsTypeInterface>
{
public:
  using BaseClass = ListTypeInterfaceImpl<typename T::VectorType, VarArgsTypeInterface>;

  using MethodsImpl = DefaultTypeImplMethods<T, TypeByPointerPOD<T>>;
  VarArgsTypeInterfaceImpl() {}

  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);

  void* adaptStorage(void** storage) {
    T* ptr = (T*) ptrFromStorage(storage);
    //return ptr
    typename T::VectorType& v = ptr->args();
    return &v;
  }

  size_t size(void* storage) override {
    return BaseClass::size(adaptStorage(&storage));
  }
  AnyIterator begin(void* storage) override {
    return BaseClass::begin(adaptStorage(&storage));
  }
  AnyIterator end(void* storage) override {
    return BaseClass::end(adaptStorage(&storage));
  }
  void pushBack(void** storage, void* valueStorage) override {
    void* vstor = adaptStorage(storage);
    BaseClass::pushBack(&vstor, valueStorage);
  }

  //ListTypeInterface* _list;
};


template<typename T> struct TypeImpl<qi::VarArguments<T> >: public VarArgsTypeInterfaceImpl<qi::VarArguments<T> > {};
}

#endif  // _QITYPE_DETAIL_TYPELIST_HXX_
