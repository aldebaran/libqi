#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_ACCESSOR_HXX_
#define _QITYPE_DETAIL_ACCESSOR_HXX_

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace qi
{
  /** Traits and functions to factor code taking any accessor as argument.
  * Supports member function, member object, and free-function
  */
  namespace detail
  {
    template<typename T, typename Void = void> struct Accessor
    {
      using is_accessor = boost::false_type;
    };
    template<typename C, typename T> struct AccessorBase
    {
      using is_accessor = boost::true_type;
      using value_type = typename boost::remove_const<T>::type;
      using class_type = C;
    };
    // we must explicitely check for is_member_object_pointer because T C::*
    // can match functions also even if it may not make sense
    template<typename C, typename T> struct Accessor<T C::*, typename boost::enable_if<typename boost::is_member_object_pointer<T C::*> >::type >
    : public AccessorBase<C, T>
    {
      using type = T C::*;
      static T& access(C* instance, type acc)
      {
        return *instance.*acc;
      }
    };
    template<typename C, typename T> struct Accessor<T& (C::*)()>
    : public AccessorBase<C, T>
    {
      using type = T& (C::*)();
      static T& access(C* instance, type acc)
      {
        return (*instance.*acc)();
      }
    };
    template<typename C, typename T> struct Accessor<T& (C::*)() const>
    : public AccessorBase<C, T>
    {
      using type = T& (C::*)() const;
      static T& access(C* instance, type acc)
      {
        return (*instance.*acc)();
      }
    };
    template<typename C, typename T> struct Accessor<T& (*)(C*)>
    : public AccessorBase<C, T>
    {
      using type = T& (*)(C*);
      static T& access(C* instance, type acc)
      {
        return acc(instance);
      }
    };
  }
}

#endif  // _QITYPE_DETAIL_ACCESSOR_HXX_


