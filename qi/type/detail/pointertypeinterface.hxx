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
  inline NullPtrException::NullPtrException()
    : std::runtime_error("attempting to dereference a null pointer")
  {}

  template<typename T> class PointerTypeInterfaceImpl: public PointerTypeInterface
  {
  public:
    TypeInterface* pointedType() override
    {
      // Caching the result here is dangerous if T uses runtime factory.
      return typeOf<T>();
    }
    PointerKind pointerKind() override { return Raw; }
    AnyReference dereference(void* storage) override
    {
      // We are in DirectAccess mode, so storage is a T*.
      auto value = storage;
      if (value == nullptr)
        throw NullPtrException{};
      return AnyReference(pointedType(), value);
    }

    void set(void** storage, AnyReference pointer) override
    {
      AnyReference obj = *pointer;
      *storage = obj.rawValue();
    }

    void setPointee(void** storage, void* pointer) override
    {
      *storage = pointer;
    }

    using TypeMethodsImpl = DefaultTypeImplMethods<T*, TypeByValue<T*>>;
    _QI_BOUNCE_TYPE_METHODS(TypeMethodsImpl);
  };

  template<typename T> class TypeImpl<T*>: public PointerTypeInterfaceImpl<T>{};

  template<typename T> class TypeSharedPointerImpl: public PointerTypeInterface
  {
  public:
    TypeInterface* pointedType() override
    {
      return typeOf<typename T::element_type>();
    }
    PointerKind pointerKind() override { return Shared; }
    AnyReference dereference(void* storage) override
    {
      const auto& sharedPtr = *reinterpret_cast<T*>(ptrFromStorage(&storage));
      auto* valuePtr = sharedPtr.get();
      if (valuePtr == nullptr)
        throw NullPtrException{};
      return AnyReference(pointedType(), valuePtr);
    }
    void set(void** storage, AnyReference pointer) override
    {
      T* ptr = (T*)ptrFromStorage(storage);
      T* otherPtr = (T*)pointer.rawValue();
      *ptr = *otherPtr;
    }
    void setPointee(void** /*storage*/, void* /*pointer*/) override
    {
      // we can't do that as it means that we would take ownership of pointer
      throw std::runtime_error("cannot convert to shared_ptr");
    }
    using Impl = DefaultTypeImplMethods<T, TypeByPointerPOD<T>>;
     _QI_BOUNCE_TYPE_METHODS(Impl);
  };

  template<typename T> class TypeImpl<boost::shared_ptr<T> >: public TypeSharedPointerImpl<boost::shared_ptr<T> >{};
}

#endif  // _QITYPE_DETAIL_TYPEPOINTER_HXX_
