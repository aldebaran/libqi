#ifndef KA_CONCEPTPREDICATE_HPP
#define KA_CONCEPTPREDICATE_HPP
#pragma once
#include <initializer_list>
#include <functional>
#include <utility>
#include "concept.hpp"
#include "functional.hpp"
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

  /// True if the type is a mathematical functor for all the passed pairs of
  /// composable functions.
  ///
  /// A mathematical functor is a function `X` that sends morphisms `A -> B` to
  /// morphisms `X(A) -> X(B)`, preserving composition. In our case, the
  /// mathematical functor translates to a polymorphic function that "lifts"
  /// functions `A -> B` to functions `X<A> -> X<B>` preserving function
  /// composition, where `X` models the `Functor` concept (see `concept.hpp`).
  ///
  /// This function lifts passed functions, then compares them through the given
  /// equivalence.
  ///
  /// Example: Checking that a type template models the `Functor` concept.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // Pair of composable functions.
  /// auto const f = [](A a) -> B {return B{2 * a.value};};
  /// auto const g = [](B b) -> C {return C{b.value + 1};};
  ///
  /// Here, `ka::fmap` will be used to (generically) lift `f` and `g`.
  /// `my_functor_fn_equ` will be called with pairs of lifted functions and
  /// decide if they are equivalent or not (for instance, by approximating
  /// extensional equality and comparing functions outputs for a sample of
  /// inputs). It is `my_functor_fn_equ` that determines which actual model of
  /// `Functor` is tested, by calling the generically lifted functions with
  /// appropriate `Functor` values (here of some `my_functor_t<T>` type).
  ///
  /// EXPECT_TRUE(is_functor(
  ///   ka::fmap,                    // function lifting
  ///   array_fn(ka::product(g, f)), // pairs of composable functions
  ///   my_functor_fn_equ            // equivalence of lifted functions
  /// ));
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Precondition: equivalence(e)
  ///
  /// Function<Function<X<U> (X<T>)> (Function<U (T)>)> F (with Functor X, forall Type T, U)
  /// Linearizable<Tuple<Function<C (B)>, Function<B (A)>>> L
  /// Relation<Function<C (A)>> R
  template<typename F, typename L, typename R = equal_t>
  auto is_functor(F X, L composable_functions, R e = equal) -> bool {
    using functional_ops::operator*; // Function composition.

    auto const _1 = id_transfo_t{};

    // Lifting of identity is the same as identity.
    if (! e(X(_1), _1)) return false;

    // Lifting of composition is the same as composition of lifting.
    for (auto const& pair: composable_functions) {
      auto const& g = std::get<0>(pair);
      auto const& f = std::get<1>(pair);
      if (! e(X(g * f), X(g) * X(f))) return false;
    }
    return true;
  }
} // namespace ka

#endif  // KA_CONCEPTPREDICATE_HPP
