#ifndef KA_UTILITY_HPP
#define KA_UTILITY_HPP
#pragma once
#include <type_traits>
#include "macro.hpp"

namespace ka {
  /// Less noisy equivalent to `std::forward`.
  ///
  /// The only purpose is to reduce the noise in generic code:
  /// occurrences of `std::forward<T>(t)` can be replaced by `fwd<T>(t)`.
  ///
  /// Note: This code is from the libstdc++ shipped with g++-7.
  template<typename T>
  BOOST_CONSTEXPR T&& fwd(typename std::remove_reference<T>::type& t) KA_NOEXCEPT(true) {
    return static_cast<T&&>(t);
  }

  template<typename T>
  BOOST_CONSTEXPR T&& fwd(typename std::remove_reference<T>::type&& t) KA_NOEXCEPT(true) {
    static_assert(!std::is_lvalue_reference<T>::value,
      "template argument substituting T is an lvalue reference type");
    return static_cast<T&&>(t);
  }

  /// Produces an L-value reference in a non-evaluated context.
  ///
  /// Note: Because of the non-evaluated context, the function need not be defined.
  ///
  /// Note: This follows the same idea as `std::declval()`.
  ///
  /// Example: Statically selecting an overload.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // Different overloads of `f` will return different types.
  /// template<typename T>
  /// T f(T& t) {
  ///   // ...
  /// }
  ///
  /// template<typename T>
  /// T* f(T (&a)[N]) {
  ///   // ...
  /// }
  ///
  /// template<ytpename T>
  /// struct X {
  ///   // Produce a "fake" L-value reference in a `decltype` context.
  ///   using U = decltype(f(declref<T>()));
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  template<typename T>
  T& declref();

  /// Produces an L-value reference to const in a non-evaluated context.
  ///
  /// Note: Because of the non-evaluated context, the function need not be defined.
  ///
  /// Note: This follows the same idea as `std::declval()`.
  ///
  /// See also `declref`.
  template<typename T>
  T const& declcref();
} // namespace ka

#endif // KA_UTILITY_HPP
