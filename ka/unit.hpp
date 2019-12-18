#ifndef KA_UNIT_HPP
#define KA_UNIT_HPP
#pragma once
#include <tuple>
#include "macroregular.hpp"

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
  /// meaning(unit_t) = 1
  /// 1 is the singleton set, i.e. the set with a single element.
  ///
  /// See [unit type](https://en.wikipedia.org/wiki/Unit_type)
  using unit_t = std::tuple<>;

  /// Unique value of `unit_t`.
  constexpr unit_t unit;

  /// Variadic constant type function to unit type.
  ///
  /// This function sends any sequence of types to the unit type.
  ///
  /// meaning(constant_unit_t<T...>::type) = 1
  /// See also `unit_t` for an explanation on 1.
  template<typename...>
  struct constant_unit_t {
  // TypeFunction:
    using type = unit_t;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(constant_unit_t)
  };

  /// Variadic constant type function to unit type (applied version).
  ///
  /// meaning(ConstantUnit<T...>) = 1
  ///
  /// See also `unit_t` for an explanation on 1.
  /// See also `constant_unit_t`.
  /// See also `ConstantVoid`.
  template<typename...>
  using ConstantUnit = unit_t; // Shortcut for `typename constant_unit_t<...>::type`
} // namespace ka

#endif // KA_UNIT_HPP
