#pragma once
#ifndef _QI_RELATIONPREDICATE_HPP_
#define _QI_RELATIONPREDICATE_HPP_

#include <utility>
#include <initializer_list>
#include <functional>
#include <qi/type/traits.hpp>
#include <qi/range.hpp>
#include <qi/assert.hpp>

/// @file
/// Contains predicates that check at runtime if a relation has certain properties.
///
/// Relation definition
/// =============================================
/// A relation is a homogeneous binary predicate, i.e. a FunctionObject
/// with the signature `bool (T, T)` (don't cry, everything's explained ;)).
/// Following the conventional use in mathematics, the "domain" of a relation
/// is the type of its arguments.
/// For example, the domain of bool isGreater(int, int) is int.
///
/// More information on relations can be found in the chapter 4 of
/// [Elements of Programming (Stepanov-MacJones, 2009)](http://elementsofprogramming.com/)
///
/// Formal definition justification
/// =============================================
/// The definitions given here are formal in order to provide a solid foundation
/// to build code upon. For example, the predicates defined here are needed to be
/// able to check the regularity of types (see `isRegular` in conceptpredicate.hpp),
/// which in turn gives a strong guarantee that types well-behave (see also the
/// `Regular` definition in conceptpredicate.hpp), which in turn is the sine-qua-non
/// condition to build easy-to-understand, maintainable and efficient components.
///
/// A word on notation
/// =============================================
/// Below, the symbol |-> is used to give the logical definition of a
/// function. For example,
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Relation R)
/// transitive: R
///   r |-> (forall a in the domain of R) r(a, a)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// means that `transitive` is a predicate on a relation that returns true if
/// for all possible values accepted by r, the relation is true for the value
/// relatively to itself.
/// For example, the predicate would be true for the "has the same age as" relation,
/// because you always have the same age as yourself (duh!). But it would be
/// false for the "is brother of" relation (you're not your own brother).
///
/// The symbol <=> means "is equivalent to".
///
/// Properties
/// =============================================
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Relation R)
/// transitive: R
///  r |-> (forall a, b, c in the domain of R) r(a, b) && r(b, c) implies r(a, c)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Example: "is ancestor" (if a is an ancestor of b, and b is an ancestor of c, a is an ancestor of c)
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Relation R)
/// reflexive: R
///  r |-> (forall a in the domain of R) r(a, a)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Example: "is in the same place" (you're always in the same place than yourself)
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Relation R)
/// symmetric: R
///  r |-> (forall a, b in domain the of R) r(a, b) implies r(b, a)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Example: "is brother" (if a is the brother of b, b is the brother of a)
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Relation R)
/// equivalence: R
///  r |-> transitive(r) && reflexive(r) && symmetric(r)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Example: "has same parents"
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Relation R)
/// totalOrdering: R
///  r |-> transitive(r)
///    && (forall a, b in the domain of R) exactly one of the following holds:
///      r(a, b), r(b, a) or a == b
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Note: The second condition (exactly one holds) is the trichotomy law.
/// Example: "less on integers"
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Relation<T> R, Relation<T> E)
/// weakOrdering: R
///  r |-> transitive(r) && (exists e in E)equivalence(e) &&
///    && (forall a, b in the domain of R) exactly one of the following holds:
///      r(a, b), r(b, a) or e(a, b)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Note: The second condition (exactly one holds) is the weak trichotomy law.
///
/// Example: "is younger" with the equivalence "have same age"
///
/// Note that for a weak ordering r, (!r(a, b) && !r(b, a)) is an equivalence.
///
/// E.g. (!isYounger(a, b) && !isYounger(b, a)) <=> haveSameAge(a, b)

namespace qi
{
  namespace detail
  {
    // TODO C++14: Make the functions in this file constexpr.

    /// Checks that the relation is transitive for all values in the given range.
    /// See the file comment for an explanation of what "transitive" means.
    /// For this test to be exact, 'values' must contain all values in the domain of R.
    /// Relation<T> R, ReadableForwardRange<T> Rng
    template<typename R, typename Rng>
    bool isTransitive(R r, Rng values)
    {
      for (auto v0 = values; !isEmpty(v0); pop(v0))
      {
        const auto& a = front(v0);
        for (auto v1 = values; !isEmpty(v1); pop(v1))
        {
          const auto& b = front(v1);
          for (auto v2 = values; !isEmpty(v2); pop(v2))
          {
            const auto& c = front(v2);
            const auto ab = r(a, b);
            const auto ac = r(a, c);
            const auto ba = r(b, a);
            const auto bc = r(b, c);
            const auto ca = r(c, a);
            const auto cb = r(c, b);
            if (ab && bc) QI_TRUE_OR_RETURN_FALSE(ac);
            if (ac && cb) QI_TRUE_OR_RETURN_FALSE(ab);
            if (ba && ac) QI_TRUE_OR_RETURN_FALSE(bc);
            if (bc && ca) QI_TRUE_OR_RETURN_FALSE(ba);
            if (ca && ab) QI_TRUE_OR_RETURN_FALSE(cb);
            if (cb && ba) QI_TRUE_OR_RETURN_FALSE(ca);
          }
        }
      }
      return true;
    }

    /// Checks that the relation is reflexive for all values in the given range.
    /// See the file comment for an explanation of what "reflexive" means.
    /// For this test to be exact, 'values' must contain all values in the domain of R.
    /// Relation<T> R, ReadableRange<T> Rng
    template<typename R, typename Rng>
    bool isReflexive(R r, Rng values)
    {
      while (!isEmpty(values))
      {
        const auto& a = front(values);
        QI_TRUE_OR_RETURN_FALSE(r(a, a));
        pop(values);
      }
      return true;
    }

    /// Checks that the relation is symmetric for all values in the given range.
    /// See the file comment for an explanation of what "symmetric" means.
    /// For this test to be exact, 'values' must contain all values in the domain of R.
    /// Relation<T> R, ReadableForwardRange<T> Rng
    template<typename R, typename Rng>
    bool isSymmetric(R r, Rng values)
    {
      for (auto v0 = values; !isEmpty(v0); pop(v0))
      {
        const auto& a = front(v0);
        for (auto v1 = values; !isEmpty(v1); pop(v1))
        {
          const auto& b = front(v1);
          const auto ab = r(a, b);
          const auto ba = r(b, a);
          if (ab) QI_TRUE_OR_RETURN_FALSE(ba);
          if (ba) QI_TRUE_OR_RETURN_FALSE(ab);
        }
      }
      return true;
    }

    /// Checks that the relation is an equivalence for all values in the given range.
    /// See the file comment for an explanation of what "equivalence" means.
    /// For this test to be exact, 'values' must contain all values in the domain of R.
    /// Relation<T> R, ReadableForwardRange<T> Rng
    template<typename R, typename Rng>
    bool isEquivalence(R r, Rng values)
    {
      // isTransitive is the last because it is the most complex (O^3).
      return isReflexive(r, values) && isSymmetric(r, values) && isTransitive(r, values);
    }

    /// Checks that the relation is an equivalence for all values in the given range.
    /// See the file comment for an explanation of what "equivalence" means.
    /// For this test to be exact, 'values' must contain all values in the domain of R.
    /// Relation<T>
    template<typename R, typename T>
    bool isEquivalence(R r, std::initializer_list<T> values)
    {
      return isEquivalence(r, boundedRange(values));
    }

    /// Checks that the relation is weak trichotomic for all values in the given range.
    /// See the file comment for an explanation of what "weak trichotomic" means.
    /// For this test to be exact, 'values' must contain all values in the domain of R.
    /// Precondition: equivalence(e)
    /// Relation<T> R0, Relation<T> R1, ReadableForwardRange<T> Rng
    template<typename R0, typename R1, typename Rng>
    bool isWeakTrichotomic(R0 r, R1 e, Rng values)
    {
      for (auto v0 = values; !isEmpty(v0); pop(v0))
      {
        const auto& a = front(v0);
        for (auto v1 = values; !isEmpty(v1); pop(v1))
        {
          const auto& b = front(v1);
          bool x = r(a, b), y = r(b, a), z = e(a, b);
          QI_TRUE_OR_RETURN_FALSE(( x && !y && !z)
                               || (!x &&  y && !z)
                               || (!x && !y &&  z));
        }
      }
      return true;
    }

    /// Checks that the relation is trichotomic for all values in the given range.
    /// See the file comment for an explanation of what "trichotomic" means.
    /// For this test to be exact, 'values' must contain all values in the domain of R.
    /// Relation<T> R0, ReadableForwardRange<T> Rng
    template<typename R0, typename Rng>
    bool isTrichotomic(R0 r, Rng values)
    {
      using T = traits::RemoveCvRef<decltype(front(values))>;
      return isWeakTrichotomic(r, std::equal_to<T>{}, values);
    }

    /// Checks that the relation is a weak ordering for all values in the given range.
    /// See the file comment for an explanation of what "weak ordering" means.
    /// For this test to be exact, 'values' must contain all values in the domain of R.
    /// Precondition: equivalence(e)
    /// Relation<T> R0, Relation<T> R1, ReadableForwardRange<T> Rng
    template<typename R0, typename R1, typename Rng>
    bool isWeakOrdering(R0 r, R1 e, Rng values)
    {
      return isWeakTrichotomic(r, e, values) && isTransitive(r, values);
    }

    /// Checks that the relation is a total ordering for all values in the given range.
    /// See the file comment for an explanation of what "total ordering" means.
    /// For this test to be exact, 'values' must contain all values in the domain of R.
    /// Relation<T> R, ReadableForwardRange<T> Rng
    template<typename R, typename Rng>
    bool isTotalOrdering(R r, Rng values)
    {
      using T = traits::RemoveCvRef<decltype(front(values))>;
      return isWeakOrdering(r, std::equal_to<T>{}, values);
    }

    /// Checks that the relation is a total ordering for all values in the given range.
    /// See the file comment for an explanation of what "total ordering" means.
    /// For this test to be exact, 'values' must contain all values in the domain of R.
    /// Relation<T> R, ReadableForwardRange<T> Rng
    /// Relation<T>
    template<typename R, typename T>
    bool isTotalOrdering(R r, std::initializer_list<T> values)
    {
      return isTotalOrdering(r, boundedRange(values));
    }
  } // namespace detail

} // namespace qi

#endif // _QI_RELATIONPREDICATE_HPP_
