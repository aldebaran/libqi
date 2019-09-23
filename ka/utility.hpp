#ifndef KA_UTILITY_HPP
#define KA_UTILITY_HPP
#pragma once
#include <type_traits>
#include <utility>
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
  /// template<typename T>
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

  /// Replaces the value of an object with a new value and returns the old value of the object.
  /// It is useful to implement move assignment operators and move constructors for example.
  ///
  /// Note: If any of the constructors and assigment operators throws an exception, the behavior
  /// is undefined as is the value of the object.
  ///
  /// Note: This is equivalent to `std::exchange()` from C++14.
  /// TODO: Deprecate then remove it when C++14 is enabled on this library.
  ///
  /// Example: Implementing a move constructor.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// struct my_struct_t {
  ///   int* p;
  ///   int n;
  ///
  ///   my_struct_t(my_struct_t&& o)
  ///     : p{ ka::exchange(o.p, nullptr) }
  ///     , n{ ka::exchange(o.n, 0) } {
  ///   }
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// MoveConstructible T && MoveAssignable<U> T
  template<typename T, typename U = T>
  T exchange(T& obj, U&& new_value) {
    T old_value = std::move(obj);
    obj = fwd<U>(new_value);
    return old_value;
  }

  /// Helper type to avoid ODR violations.
  template<typename T>
  struct static_const_t {
    static KA_CONSTEXPR T value{};
  };

  template<typename T> KA_CONSTEXPR
  T static_const_t<T>::value;
} // namespace ka

#endif // KA_UTILITY_HPP
