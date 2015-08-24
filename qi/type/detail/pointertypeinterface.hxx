#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_TYPEPOINTER_HXX_
#define _QITYPE_DETAIL_TYPEPOINTER_HXX_

#include <boost/shared_ptr.hpp>

namespace qi
{
  template<typename T> class PointerTypeInterfaceImpl: public PointerTypeInterface
  {
  public:
    TypeInterface* pointedType()
    {
      // Caching the result here is dangerous if T uses runtime factory.
      return typeOf<T>();
    }
    PointerKind pointerKind() { return Raw; }
    AnyReference dereference(void* storage)
    {
      // We are in DirectAccess mode, so storage is a T*.
      void* value = pointedType()->initializeStorage(storage);
      return AnyReference(pointedType(), value);
    }

    void set(void** storage, AnyReference pointer)
    {
      AnyReference obj = *pointer;
      *storage = obj.rawValue();
    }

    void setPointee(void** storage, void* pointer)
    {
      *storage = pointer;
    }

    typedef DefaultTypeImplMethods<T*,
                                     TypeByValue<T*>
                                     > TypeMethodsImpl;
    _QI_BOUNCE_TYPE_METHODS(TypeMethodsImpl);
  };

  template<typename T> class TypeImpl<T*>: public PointerTypeInterfaceImpl<T>{};

  template<typename T> class TypeSharedPointerImpl: public PointerTypeInterface
  {
  public:
    TypeInterface* pointedType()
    {
      return typeOf<typename T::element_type>();
    }
    PointerKind pointerKind() { return Shared; }
    AnyReference dereference(void* storage)
    {
      T* ptr = (T*)ptrFromStorage(&storage);
      void *value = pointedType()->initializeStorage(ptr->get());
      return AnyReference(pointedType(), value);
    }
    void set(void** storage, AnyReference pointer)
    {
      T* ptr = (T*)ptrFromStorage(storage);
      T* otherPtr = (T*)pointer.rawValue();
      *ptr = *otherPtr;
    }
    void setPointee(void** storage, void* pointer)
    {
      // we can't do that as it means that we would take ownership of pointer
      throw std::runtime_error("cannot convert to shared_ptr");
    }
    typedef DefaultTypeImplMethods<T, TypeByPointerPOD<T> > Impl;
     _QI_BOUNCE_TYPE_METHODS(Impl);
  };

  template<typename T> class TypeImpl<boost::shared_ptr<T> >: public TypeSharedPointerImpl<boost::shared_ptr<T> >{};
}

#endif  // _QITYPE_DETAIL_TYPEPOINTER_HXX_
