#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_TYPEPOINTER_HXX_
#define _QITYPE_DETAILS_TYPEPOINTER_HXX_

#include <boost/shared_ptr.hpp>

namespace qi
{
  template<typename T> class PointerTypeInterfaceImpl: public PointerTypeInterface
  {
  public:
    TypeInterface* pointedType() const
    {
      // Caching the result here is dangerous if T uses runtime factory.
      return typeOf<T>();
    }
    PointerKind pointerKind() const { return Raw;}
    GenericValuePtr dereference(void* storage)
    {
      GenericValuePtr result;
      result.type = pointedType();
      // We are in DirectAccess mode, so storage is a T*.
      result.value = result.type->initializeStorage(storage);
      return result;
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
    TypeInterface* pointedType() const
    {
      return typeOf<typename T::element_type>();
    }
    PointerKind pointerKind() const { return Shared;}
    GenericValuePtr dereference(void* storage)
    {
      T* ptr = (T*)ptrFromStorage(&storage);
      GenericValuePtr result;
      result.type = pointedType();
      result.value = result.type->initializeStorage(ptr->get());
      return result;
    }
    void setPointee(void** storage, void* pointer)
    {
      T* ptr = (T*)ptrFromStorage(storage);
      *ptr = T((typename T::element_type*)pointer);
    }
     _QI_BOUNCE_TYPE_METHODS(DefaultTypeImplMethods<T>);
  };

  template<typename T> class TypeImpl<boost::shared_ptr<T> >: public TypeSharedPointerImpl<boost::shared_ptr<T> >{};
}

#endif  // _QITYPE_DETAILS_TYPEPOINTER_HXX_
