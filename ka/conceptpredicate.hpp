#ifndef KA_CONCEPTPREDICATE_HPP
#define KA_CONCEPTPREDICATE_HPP
#pragma once
#include <initializer_list>
#include <functional>
#include <utility>
#include "concept.hpp"
#include "macro.hpp"
#include "range.hpp"
#include "relationpredicate.hpp"
#include "typetraits.hpp"

/// @file
/// Contains predicates that check at runtime if a value of a certain type
/// respects the constraints of a particular concept.
///
/// Of course, if the code doesn't compile for a certain type, this type
/// doesn't model the given concept.
///
/// Concepts and properties used here are further described in
/// [Elements of Programming (Stepanov-MacJones, 2009)](http://elementsofprogramming.com/)

namespace ka {
  /// True if the type of the range elements is regular for all the passed values.
  ///
  /// Precondition: (forall x, y in rng where x = pop^n(rng), y = pop^m(rng) and n, m >= 0 and n != m) x != y
  ///  In other words, all elements in the range are different.
  ///
  /// TODO: try to make it constexpr when upgrading to C++14.
  ///
  /// ReadableForwardRange<PseudoRegular T> Rng
  template<typename Rng>
  bool is_regular(Rng rng) {
    if (is_empty(rng)) return true;

    auto x = front(rng);
    using T = decltype(x);

    // Default construction
    T y;
    // Assignment
    y = x;
    KA_TRUE_OR_RETURN_FALSE(x == y && !(x != y));
    // Copy
    T z = x;
    KA_TRUE_OR_RETURN_FALSE(z == x && !(z != x));
    // Equality
    KA_TRUE_OR_RETURN_FALSE(is_equivalence(std::equal_to<T>{}, rng));
    // Total ordering
    KA_TRUE_OR_RETURN_FALSE(is_total_ordering(std::less<T>{}, rng));
    return true;
  }

  // PseudoRegular T
  template<typename T>
  bool is_regular(std::initializer_list<T> l) {
    return is_regular(bounded_range(l));
  }
} // namespace ka

#endif  // KA_CONCEPTPREDICATE_HPP
