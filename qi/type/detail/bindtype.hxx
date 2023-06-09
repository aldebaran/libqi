#pragma once
/*
**  Copyright (C) 2013, 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_BINDTYPE_HXX_
#define _QITYPE_DETAIL_BINDTYPE_HXX_

#include <boost/callable_traits.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <type_traits>
#include <utility>

namespace qi::detail::bind
{

template<typename Fn, typename... T>
struct FilterMapParameters {};

template<typename Fn>
struct FilterMapParameters<Fn> {
  using Type = std::tuple<>;
};

template<typename Fn, typename T0, typename... T>
struct FilterMapParameters<Fn, T0, T...> {
  using Type = typename FilterMapParameters<Fn, T...>::Type;
};

template<typename Fn, int Idx, typename... T>
struct FilterMapParameters<Fn, boost::arg<Idx>, T...> {
  using Type =
    decltype(std::tuple_cat(
      // bind placeholders start at index 1, `tuple_element` counts from 0.
      std::tuple<std::tuple_element_t<Idx - 1, boost::callable_traits::args_t<Fn>>>(),
      typename FilterMapParameters<Fn, T...>::Type()
    ));
};

/// `T` may be a template type parameterized with the types of the
/// parameters of the `boost::bind` call. If it is not, which is the case
/// when binding a function that takes no parameters., the result of this
/// type is an empty tuple.
template<typename Fn, typename T>
struct ApplyFilterMapParameters {
  using Type = std::tuple<>;
};

template<typename Fn, template<typename...> typename BindParameters, typename... Param>
struct ApplyFilterMapParameters<Fn, BindParameters<Param...>> {
  using Type = typename FilterMapParameters<Fn, Param...>::Type;
};
template<typename T> struct boost_bind_result_type {};
template<typename R, typename A, typename B>
struct boost_bind_result_type<boost::_bi::bind_t<R, A, B> >
{
using type = R;
};

/// Applies a filter map operation on the parameter types of a `boost::bind` result.
///
/// It keeps parameters that are placeholders (`boost::arg<I>`) and removes
/// parameters that are already bound, and then replaces placeholders with
/// the associated function parameter type at the placeholder index.
/// The member type `type` is a `std::tuple` parameterized with the result
/// of the filter map.
///
/// Example:
/// ```
///   int fun(char, float, std::string) {
///     return 32;
///   }
///   auto f = boost::bind(fun, boost::placeholders::_3, 3.14, boost::placeholders::_1);
///   using BindF = typename qi::boost_bind_function_type<decltype(f)>::type;
///   static_assert(std::is_same_v<BindF, int(std::string, char)>);
/// ```
template<typename T> struct boost_bind_parameter_types {};
template<typename Res, typename Fn, typename BindParameters>
struct boost_bind_parameter_types<boost::_bi::bind_t<Res, Fn, BindParameters> >
{
  using type = typename detail::bind::ApplyFilterMapParameters<Fn, BindParameters>::Type;
};

/// Takes as argument the result type of a `boost::bind`, and returns
/// a compatible function type.
template<typename T> struct boost_bind_function_type
{
  using type = boost::callable_traits::apply_return_t<
    typename boost_bind_parameter_types<T>::type,
    typename boost_bind_result_type<T>::type
  >;
  using pointer_type = std::add_pointer_t<type>;
};

} // namespace qi::detail::bind

#endif  // _QITYPE_DETAIL_BINDTYPE_HXX_
