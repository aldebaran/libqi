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
template<typename T> class ListTypeInterfaceImpl:
public ListTypeInterface
{
public:
  typedef DefaultTypeImplMethods<T,
                               TypeByPointer<T>
                               > MethodsImpl;
  ListTypeInterfaceImpl();
  virtual size_t size(void* storage);
  virtual TypeInterface* elementType() const;
  virtual GenericIterator begin(void* storage);
  virtual GenericIterator end(void* storage);
  virtual void pushBack(void** storage, void* valueStorage);
  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
};

// Type impl for any class that behaves as a forward iterator (++, *, ==)
template<typename T> class TypeSimpleIteratorImpl: public IteratorTypeInterface
{
public:
  typedef T Storage;
  virtual GenericValueRef dereference(void* storage)
  {
    T* ptr = (T*)ptrFromStorage(&storage);
    return GenericValueRef(*(*ptr));
  }
  virtual void next(void** storage)
  {
    T* ptr = (T*)ptrFromStorage(storage);
    ++(*ptr);
  }
  virtual bool equals(void* s1, void* s2)
  {
    T* p1 = (T*)ptrFromStorage(&s1);
    T* p2 = (T*)ptrFromStorage(&s2);
    return *p1 == *p2;
  }
  typedef DefaultTypeImplMethods<Storage> TypeImpl;
  _QI_BOUNCE_TYPE_METHODS(TypeImpl);
  static GenericIterator make(const T& val)
  {
    static TypeSimpleIteratorImpl<T>* type = new TypeSimpleIteratorImpl<T>();
    return GenericValue(GenericValuePtr(type, type->initializeStorage(const_cast<void*>((const void*)&val))));
  }
};


template<typename T>
ListTypeInterfaceImpl<T>::ListTypeInterfaceImpl()
{
}

template<typename T> TypeInterface*
ListTypeInterfaceImpl<T>::elementType() const
{
  static TypeInterface* result = typeOf<typename T::value_type>();
  return result;
}

template<typename T> GenericIterator
ListTypeInterfaceImpl<T>::begin(void* storage)
{
  T* ptr = (T*)ptrFromStorage(&storage);
  return TypeSimpleIteratorImpl<typename T::iterator>::make(ptr->begin());
}

template<typename T> GenericIterator
ListTypeInterfaceImpl<T>::end(void* storage)
{
  T* ptr = (T*)ptrFromStorage(&storage);
  return TypeSimpleIteratorImpl<typename T::iterator>::make(ptr->end());
}

template<typename T> void
ListTypeInterfaceImpl<T>::pushBack(void** storage, void* valueStorage)
{
  static TypeInterface* elemType = typeOf<typename T::value_type>();
  T* ptr = (T*) ptrFromStorage(storage);
  ptr->push_back(*(typename T::value_type*)elemType->ptrFromStorage(&valueStorage));
}

template<typename T> size_t
ListTypeInterfaceImpl<T>::size(void* storage)
{
  T* ptr = (T*) ptrFromStorage(&storage);
  return ptr->size();
}

// There is no way to register a template container type :(
template<typename T> struct TypeImpl<std::vector<T> >: public ListTypeInterfaceImpl<std::vector<T> > {};
template<typename T> struct TypeImpl<std::list<T> >: public ListTypeInterfaceImpl<std::list<T> > {};

}

#endif  // _QITYPE_DETAILS_TYPELIST_HXX_
