#ifndef KA_INDEXED_HPP
#define KA_INDEXED_HPP
#pragma once
#include "integersequence.hpp"
#include "typetraits.hpp"
#include "utility.hpp"
#include "macroregular.hpp"

namespace ka {
  /// Associates a type to an index.
  ///
  /// This type is useful to dissociate same type alternatives in sum types like
  /// `boost::variant`. This is especially true in generic code, where we
  /// typically cannot guarantee that the same type won't appear twice or more
  /// in the sum. That is, `indexed_t` allows one to implement the categorical
  /// sum, which in our case is a *disjoint* union.
  ///
  /// Example: Handling two alternatives of same type with `boost::variant`.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// boost::variant<int, int> v1 { 12 }; // It is unclear which alternative is used.
  /// // auto i = boost::get<int>(v1); This won't compile because it's ambiguous.
  ///
  /// boost::variant<indexed_t<0, int>, indexed_t<1, int>>
  ///   v2 { indexed<1>(42) }; // The chosen alternative is non-ambiguous.
  /// auto i = boost::get<indexed_t<1, int>>(v2); // This call is unambiguous.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  template<std::size_t I, typename T>
  struct indexed_t {
    static constexpr auto index = I;
    T value;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(indexed_t, value)
  // Readable:
    constexpr
    auto operator*() const& noexcept -> T const& {
      return value;
    }
  // Mutable:
    auto operator*() & noexcept -> T& {
      return value;
    }
    auto operator*() && noexcept(std::is_nothrow_constructible<Decay<T>, T&&>::value) -> T {
      return mv(value);
    }
  };

  /// Constructs an `indexed_t` with the template parameter as its index and the given parameter as
  /// its value.
  template<std::size_t I, typename T> constexpr
  auto indexed(T&& val) noexcept(std::is_nothrow_constructible<Decay<T>, T&&>::value)
    -> indexed_t<I, Decay<T>> {
    return { fwd<T>(val) };
  }

  namespace detail {
    template<template<typename...> class VT, typename... T, std::size_t... I>
    auto apply_indexed(index_sequence<I...>) -> VT<indexed_t<I, T>...>;
  } // namespace detail

  /// Helper type to bind a type variadic template to a sequence of indexed types.
  ///
  /// Example: Defining a `boost::variant` of a long list of indexed types.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// using my_variant = ka::ApplyIndexed<boost::variant, int, double, char,
  ///                                                     float, int, long,
  ///                                                     std::string>;
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Note: Unfortunately, the compiler is not always capable of deducing the types `T` if this
  /// type is used as the parameter of a function template.
  template<template<typename...> class VT, typename... T>
  using ApplyIndexed =
    decltype(detail::apply_indexed<VT, T...>(index_sequence_for<T...>{}));
} // namespace ka

#endif // KA_INDEXED_HPP
