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
#include <qi/concept.hpp>

/// @file
/// Contains predicates that check at runtime if a value of a certain type
/// respects the constraints of a particular concept.
///
/// Of course, if the code doesn't compile for a certain type, this type
/// doesn't model the given concept.
/// Concepts and properties used here are further described in
/// [Elements of Programming (Stepanov-MacJones, 2009)](http://elementsofprogramming.com/)

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
