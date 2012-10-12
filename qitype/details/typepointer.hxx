#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPEPOINTER_HXX_
#define _QIMESSAGING_TYPEPOINTER_HXX_

#include <boost/shared_ptr.hpp>

namespace qi
{
  template<typename T> class TypePointerImpl: public TypePointer
  {
  public:
    Type* pointedType() const
    {
      static Type* result = 0;
      if (!result)
        result = typeOf<T>();
      return result;
    }

    GenericValue dereference(void* storage)
    {
      GenericValue result;
      result.type = pointedType();
      // We are in DirectAccess mode, so storage is a T*.
      result.value = result.type->initializeStorage(storage);
      return result;
    }

    typedef DefaultTypeImplMethods<T*,
                                     TypeByValue<T*>
                                     > TypeMethodsImpl;
    _QI_BOUNCE_TYPE_METHODS(TypeMethodsImpl);
  };

  template<typename T> class TypeImpl<T*>: public TypePointerImpl<T>{};

  template<typename T> class TypeSharedPointerImpl: public TypePointer
  {
  public:
    Type* pointedType() const
    {
      static Type* result = 0;
      if (!result)
        result = typeOf<typename T::element_type>();
      return result;
    }

    GenericValue dereference(void* storage)
    {
      T* ptr = (T*)ptrFromStorage(&storage);
      GenericValue result;
      result.type = pointedType();
      result.value = result.type->initializeStorage(ptr->get());
      return result;
    }
     _QI_BOUNCE_TYPE_METHODS(DefaultTypeImplMethods<T>);
  };

  template<typename T> class TypeImpl<boost::shared_ptr<T> >: public TypeSharedPointerImpl<boost::shared_ptr<T> >{};
}

#endif
