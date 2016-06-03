#pragma once
/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_CONCEPTPREDICATES_HPP_
#define _QI_CONCEPTPREDICATES_HPP_

// Contains predicates that check at runtime if a value of a certain type
// respect the constraints of a particular concept.
// Of course, if the code doesn't compile for a certain type, this type
// doesn't model the given concept.
// Concepts used here are further described in Elements of Programming (Addison-Wesley, 2009).

namespace qi
{

  namespace detail
  {
#define QI_TRUE_OR_RETURN_FALSE(condition) if (!(condition)) return false

    // partially-formed state = an object that can only be assigned or destroyed.
    // Any other operation is undefined.

    // Regular(T) =
    //     T() yields an object in a partially-formed state.
    //  && (for a, b in T) T a = b; implies a == b
    //  && (for a, b in T) a = b implies a == b
    //  && (for a, b in T with a == b) modification on a implies a != b // copies independence
    //  && (for a, b in T) a < b is a total ordering

    /// This version is to be used if no action can be defined (typically T is empty).
    // PseudoRegular T
    template<typename T>
    bool isRegular(const T& x)
    {
      // Default construction
      T y;
      // Assignment
      y = x;
      QI_TRUE_OR_RETURN_FALSE(x == y && !(x != y));
      // Copy
      T z = x;
      QI_TRUE_OR_RETURN_FALSE(z == x && !(z != x));
      // Total ordering
      QI_TRUE_OR_RETURN_FALSE(!(x < y) && !(y < x));
      return true;
    }

    /// An action is a regular function with the signature void (T&).
    /// It is semantically equivalent to the transformation with the
    /// signature T (T).
    /// PseudoRegular T, Action<T> A
    template<typename T, typename A>
    bool isRegular(const T& x, A a)
    {
      // Precondition: forall x in T, (y = x, a(x), x != y)
      QI_TRUE_OR_RETURN_FALSE(isRegular(x));
      T y = x;
      T z;
      { // Disjointness of copies
        z = y;
        a(z);
        QI_TRUE_OR_RETURN_FALSE(z != y && !(z == y));
        QI_TRUE_OR_RETURN_FALSE(y != z && !(y == z));
        y = z;
        QI_TRUE_OR_RETURN_FALSE(y == z && !(y != z));
      }
      { // Total ordering
        y = x;
        z = x;
        a(y);
        QI_TRUE_OR_RETURN_FALSE((y < z && !(z < y)) || (!(y < z) && z < y));
      }
      return true;
    }

#undef QI_TRUE_OR_RETURN_FALSE
  } // namespace detail

} // namespace qi

#endif  // _QITYPE_CONCEPTPREDICATES_HPP_
