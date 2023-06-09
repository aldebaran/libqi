#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_ACCESSOR_HXX_
#define _QITYPE_DETAIL_ACCESSOR_HXX_

#include <boost/callable_traits.hpp>
#include <type_traits>
#include <ka/typetraits.hpp>

namespace qi
{

class SignalBase;
class PropertyBase;

/// Traits and functions to factor code taking any accessor as argument.
/// Accessors can be pointers to member (functions or data) or free-functions
/// taking a pointer to the instance as their sole argument, that return
/// a lvalue reference.
namespace detail::accessor
{

template <typename T>
using ValueType = std::remove_reference_t<boost::callable_traits::return_type_t<T>>;

template <typename T>
using ReturnTypeIsLValueRef = std::is_lvalue_reference<boost::callable_traits::return_type_t<T>>;

template <typename... T> struct IsAccessorFreeFnArgs : std::false_type {};
template <typename T>
struct IsAccessorFreeFnArgs<T>
    : std::conjunction<std::is_pointer<T>,
                       std::is_class<std::remove_pointer_t<T>>> {};

template <typename T>
using HasInstanceAsFirstArg =
    std::disjunction<std::is_member_pointer<T>,
                     boost::callable_traits::args_t<T, IsAccessorFreeFnArgs>>;

/// Evaluates to `std::true_type` if `T` is an accessor.
template <typename T>
using IsAccessor = std::conjunction<ReturnTypeIsLValueRef<T>, HasInstanceAsFirstArg<T>>;

template<typename A, typename C>
decltype(auto) invoke(A&& accessor, C* instance)
{
  using Instance = std::tuple_element_t<0, boost::callable_traits::args_t<A>>;
  /// `Instance` can be a cv-qualified ref or a pointer, so we remove everything
  /// so that we can get the raw type.
  using RawInstance = std::remove_pointer_t<ka::RemoveCvRef<Instance>>;
  return std::invoke(std::forward<A>(accessor), static_cast<RawInstance*>(instance));
}

} // namespace detail::accessor
} // namespace qi

#endif  // _QITYPE_DETAIL_ACCESSOR_HXX_
