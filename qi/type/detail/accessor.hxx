#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_ACCESSOR_HXX_
#define _QITYPE_DETAIL_ACCESSOR_HXX_

#include <type_traits>

namespace qi
{
  /** Traits and functions to factor code taking any accessor as argument.
  * Supports member function, member object, and free-function
  */
  namespace detail
  {
    template<typename T, typename Void = void> struct Accessor
    {
      typedef std::false_type is_accessor;
    };
    template<typename C, typename T> struct AccessorBase
    {
      typedef std::true_type is_accessor;
      typedef typename std::remove_const<T>::type value_type;
      typedef C class_type;
    };
    // we must explicitely check for is_member_object_pointer because T C::*
    // can match functions also even if it may not make sense
    template<typename C, typename T> struct Accessor<T C::*, typename std::enable_if<std::is_member_object_pointer<T C::*>::value>::type>
    : public AccessorBase<C, T>
    {
      typedef T C::* type;
      static T& access(C* instance, type acc)
      {
        return *instance.*acc;
      }
    };
    template<typename C, typename T> struct Accessor<T& (C::*)()>
    : public AccessorBase<C, T>
    {
      typedef T& (C::*type)();
      static T& access(C* instance, type acc)
      {
        return (*instance.*acc)();
      }
    };
    template<typename C, typename T> struct Accessor<T& (C::*)() const>
    : public AccessorBase<C, T>
    {
      typedef T& (C::*type)() const;
      static T& access(C* instance, type acc)
      {
        return (*instance.*acc)();
      }
    };
    template<typename C, typename T> struct Accessor<T& (*)(C*)>
    : public AccessorBase<C, T>
    {
      typedef T& (*type)(C*);
      static T& access(C* instance, type acc)
      {
        return acc(instance);
      }
    };
  }
}

#endif  // _QITYPE_DETAIL_ACCESSOR_HXX_


