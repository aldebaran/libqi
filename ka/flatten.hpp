#ifndef KA_FLATTEN_HPP
#define KA_FLATTEN_HPP
#pragma once

#include "macroregular.hpp"
#include "utility.hpp"

namespace ka {
  namespace detail {
    // Overloads are defined here and available for `flatten_fn_t`.

    // Dispatches to the member function if possible.
    // Note: Do not remove the trailing return type as it is needed for SFINAE.
    template<typename T> constexpr
    auto flatten(T&& t) -> decltype(fwd<T>(t).flatten()) {
      return fwd<T>(t).flatten();
    }

    // TODO: Define default version for standard containers.

    // Must be defined in this namespace to get access to the overloads.
    struct flatten_fn_t {
    // Regular:
      KA_GENERATE_FRIEND_REGULAR_OPS_0(flatten_fn_t)
    // Function<X<A> (X<X<A>>)>:
      template<typename T> constexpr
      auto operator()(T&& t) const -> decltype(flatten(fwd<T>(t))) {
        // Performs ADL.
        return flatten(fwd<T>(t));
      }
    };
  } // namespace detail

  // Avoid ODR violations.
  static constexpr auto& flatten = static_const_t<detail::flatten_fn_t>::value;
} // namespace ka

#endif // KA_FLATTEN_HPP
