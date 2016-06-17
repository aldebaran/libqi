#pragma once
/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_CONCEPTPREDICATE_HPP_
#define _QI_CONCEPTPREDICATE_HPP_

#include <utility>
#include <initializer_list>
#include <functional>
#include <qi/type/traits.hpp>
#include <qi/range.hpp>
#include <qi/detail/relationpredicate.hpp>
#include <qi/assert.hpp>

/// @file
/// Contains predicates that check at runtime if a value of a certain type
/// respects the constraints of a particular concept.
///
/// Of course, if the code doesn't compile for a certain type, this type
/// doesn't model the given concept.
/// Concepts and properties used here are further described in
/// [Elements of Programming (Stepanov-MacJones, 2009)](http://elementsofprogramming.com/)
///
/// Partially-formed state definition
/// =============================================
/// An object that can only be assigned or destroyed. Any other operation is undefined.
///
/// The main use is for default constructors. It allows them to possibly
/// do nothing, for example no allocation, and so on, and be later assigned.
/// It can be useful for arrays.
///
/// Also, moved-from objects are typically in this state.
///
/// `Regular` concept definition
/// =============================================
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Regular(T) =
///     T() yields an object in a partially-formed state.
///  && (forall a, b in T) T a = b; implies a == b
///  && (forall a, b in T) a = b implies a == b
///  && (forall a, b in T with a == b) modification on a implies a != b // copies independence
///  && equivalence(==)
///  && totalOrdering(<)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Informally, a Regular type behaves like a builtin type (e.g int) with
/// respect to the construction, copy, assignment and equality.
/// This greatly simplifies value manipulation and makes easier algorithm
/// optimization by allowing substituting one expression by another
/// (simpler, optimized) equal one.
///
/// More specifically, this concept ensures for example that two copies,
/// either made via copy construction or assignment, are equal and independent.
/// Copy independence means that modifying an object doesn't affect its copies.
///
/// `PseudoRegular` concept definition
/// =============================================
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// PseudoRegular(T) =
///  Same as Regular except that the runtime constraints are dropped.
///  This means that all the expressions required by Regular must be
///  valid (i.e. must compile), but for example
///  T a = b; does not necessarily implies a == b anymore
/// (ditto for other implications).
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// A word on the notation
/// =============================================
/// The following is used in concept specifications and below:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// "x: T" means "x is a variable with the type T"
/// "f: T -> U" means "f is a function taking a T and returning a U"
/// "f: T x U -> V" means "f is a function taking a T and a U and returning a V"
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// The two following terms are defined to understand the preconditions below.
///
/// `Transformation` definition
/// =============================================
/// A transformation is a regular function taking a single argument and
/// returning a value of the same type (i.e. it has the type T -> T).
/// For example, char toUpper(char) is a transformation.
///
/// `Action` definition
/// =============================================
/// An action is a regular function taking a value by reference and returning
/// nothing (i.e. it has the type `T& -> void`).
///
/// It is semantically equivalent to the transformation T -> T.
///
/// For example, if "popping" a range means "removing its first element",
/// you could write a transformation
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///  popTransfo: Rng -> Rng
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// returning a new range without the first element and not modifying the
/// original range,
/// which would be equivalent to the action
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///  popAction: Rng& -> void
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// modifying the given range directly.
///
/// The benefit of an action is to avoid a potentially expensive copy, with
/// the drawback of a somewhat more cumbersome writing.
///
/// For a real example, the Range concept defines "pop" as an action.
///
/// `Transformation` and `action` notation
/// =============================================
/// Below, the notation f^n means that f is a transformation that is called n times.
///
/// For example, with f: T -> T and x: T,
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// f^3(x) means f(f(f(x))).
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// By definition, f^0(x) == x.
///
/// For convenience, this notation is extended to actions.
/// For example, with a: T& -> void and x: T,
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// a^3(x) means (a(x), a(x), a(x)).
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace qi
{
  namespace detail
  {
    /// True if the type of the range elements is regular for all the passed values.
    /// Precondition: (forall x, y in rng where x = pop^n(rng), y = pop^m(rng) and n, m >= 0 and n != m) x != y
    ///  In other words, all elements in the range are different.
    /// ReadableForwardRange<PseudoRegular T> Rng
    template<typename Rng>
    bool isRegular(Rng rng)
    {
      if (isEmpty(rng)) return true;

      auto x = front(rng);
      using T = decltype(x);

      // Default construction
      T y;
      // Assignment
      y = x;
      QI_TRUE_OR_RETURN_FALSE(x == y && !(x != y));
      // Copy
      T z = x;
      QI_TRUE_OR_RETURN_FALSE(z == x && !(z != x));
      // Equality
      QI_TRUE_OR_RETURN_FALSE(isEquivalence(std::equal_to<T>{}, rng));
      // Total ordering
      QI_TRUE_OR_RETURN_FALSE(isTotalOrdering(std::less<T>{}, rng));
      return true;
    }

    // PseudoRegular T
    template<typename T>
    bool isRegular(std::initializer_list<T> l)
    {
      return isRegular(boundedRange(l));
    }
  } // namespace detail

} // namespace qi

#endif  // _QITYPE_CONCEPTPREDICATE_HPP_
