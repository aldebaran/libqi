#ifndef KA_EMPTY_HPP
#define KA_EMPTY_HPP
#pragma once
#include <initializer_list>
#include "macro.hpp"
#include "macroregular.hpp"
#include "utility.hpp"

/// Defines the `ka::empty` function that is, amongst other, part of the
/// `EmptyMutable` concept. This function is defined here for:
///
/// - raw pointers (a pointer is 'empty' if it is null)
/// - std::unique_ptr, std::shared_ptr, boost::shared_ptr (in `memory.hpp`)
/// - std::initializer_list
/// - boost::optional (in `opt.hpp`)
/// - std::function (in `functional.hpp`)
/// - any type that defines a member function `empty` (in this case `ka::empty`
///   simply forwards to the member function). This includes standard
///   containers, `boost::function`, etc.
///
/// `ka::empty` is defined as a polymorphic *function object*, that calls a free
/// function `empty` through `ADL`. The benefits of this approach are that:
///
/// - the caller always write `ka::empty` (no `using ka::empty` needed) while
///   ADL is still triggered.
/// - as a function object, `ka::empty` is easy to manipulate (as a member in
///   another type, for function composition, etc.), in contrast to a template
///   function.
///
/// The general approach is taken from Eric Niebler:
/// http://ericniebler.com/2014/10/21/customization-point-design-in-c11-and-beyond/

namespace ka {

namespace detail {
  // Overloads are defined here and available for `empty_fn_t`.

  template<typename T> KA_CONSTEXPR
  bool empty(T* t) KA_NOEXCEPT(true) {
    return t == nullptr;
  }

  inline KA_CONSTEXPR
  bool empty(std::nullptr_t) KA_NOEXCEPT(true) {
    return true;
  }

// MSVC already defines `std::empty(std::initializer_list<T>)`.
#if !BOOST_COMP_MSVC
  template<typename T> KA_CONSTEXPR
  bool empty(std::initializer_list<T> x) KA_NOEXCEPT(true) {
    return x.size() == 0;
  }
#endif

  // Dispatches to the member function if possible.
  // Note: Do not remove the trailing return type as it is needed for SFINAE.
  template<typename T> KA_CONSTEXPR
  auto empty(T&& t) -> decltype(fwd<T>(t).empty()) {
    return fwd<T>(t).empty();
  }

  // Must be defined in this namespace to get access to the overloads.
  struct empty_fn_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(empty_fn_t)
  // Predicate:
    template<typename T> KA_CONSTEXPR
    auto operator()(T&& t) const -> decltype(empty(fwd<T>(t))) {
      // Performs ADL.
      return empty(fwd<T>(t));
    }
  };
} // namespace detail

/// Predicate that calls the `empty` free function through ADL.
using detail::empty_fn_t;

namespace {
  static auto const& empty = static_const_t<empty_fn_t>::value;
}

} // namespace ka

#endif // KA_EMPTY_HPP
