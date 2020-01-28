#ifndef KA_UNIT_HPP
#define KA_UNIT_HPP
#pragma once
#include <tuple>

namespace ka {
  /// Regular type that conveys no information.
  ///
  /// It can be used in place of `void`, with the benefit of acting as a regular
  /// type (storable in data structures, comparable, etc.), and thus avoiding
  /// `void` specialization in generic code.
  ///
  /// Theoretically, it corresponds to the terminal object in the category of
  /// sets and functions, which is the singleton set (i.e. the set with a single
  /// element). It also happens to be (isomorphic to) the product of no set,
  /// hence the definition as an empty tuple.
  ///
  /// See [unit type](https://en.wikipedia.org/wiki/Unit_type)
  using unit_t = std::tuple<>;

  /// Unique value of `unit_t`.
  constexpr unit_t unit;
} // namespace ka

#endif // KA_UNIT_HPP
