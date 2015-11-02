#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_TYPEINT_HXX_
#define _QITYPE_DETAIL_TYPEINT_HXX_
#include <boost/type_traits/is_signed.hpp>

#include <qi/type/typeinterface.hpp>

namespace qi {

  template<typename T>
  class IntTypeInterfaceImpl: public IntTypeInterface
  {
  public:
    using ImplType = detail::TypeImplMethodsBySize_t<T>;

    int64_t get(void* value) override
    {
      return *(T*)ImplType::Access::ptrFromStorage(&value);
    }

    void set(void** storage, int64_t value) override
    {
      *(T*)ImplType::Access::ptrFromStorage(storage) = (T)value;
    }

    unsigned int size() override
    {
      return sizeof(T);
    }

    bool isSigned() override
    {
      return boost::is_signed<T>::value;
    }

    _QI_BOUNCE_TYPE_METHODS(ImplType);
  };

  template<typename T> class TypeBoolImpl: public IntTypeInterface
  {
  public:
    using ImplType = detail::TypeImplMethodsBySize_t<T>;

    int64_t get(void* value) override
    {
      return *(T*)ImplType::Access::ptrFromStorage(&value);
    }

    void set(void** storage, int64_t value) override
    {
      *(T*)ImplType::Access::ptrFromStorage(storage) = (T)(value != 0);
    }

    unsigned int size() override
    {
      return 0;
    }

    bool isSigned() override
    {
      return 0;
    }

    _QI_BOUNCE_TYPE_METHODS(ImplType);
  };

}


namespace qi {


  template<typename T>
  class FloatTypeInterfaceImpl: public FloatTypeInterface
  {
  public:
    using ImplType = detail::TypeImplMethodsBySize_t<T>;

    double get(void* value) override
    {
      return *(T*)ImplType::Access::ptrFromStorage(&value);
    }

    void set(void** storage, double value) override
    {
      *(T*)ImplType::Access::ptrFromStorage(storage) = (T)value;
    }

    unsigned int size() override
    {
      return sizeof(T);
    }

    _QI_BOUNCE_TYPE_METHODS(ImplType);
  };

}

#endif  // _QITYPE_DETAIL_TYPEINT_HXX_
