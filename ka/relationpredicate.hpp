#ifndef KA_RELATIONPREDICATE_HPP
#define KA_RELATIONPREDICATE_HPP
#pragma once
#include <initializer_list>
#include <functional>
#include <utility>
#include "macro.hpp"
#include "range.hpp"
#include "typetraits.hpp"

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
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Relation R0, Relation R1)
///   requires(Domain(R0) = Domain(R1))
/// are_complement: R0 x R1
///  (r0, r1) |-> (forall a, b in the domain of R0) r0(a, b) != r1(a, b)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A relation r0 is the complement of another relation r1 iff for all pairs of
/// element exactly one of these two relations hold at a time.
///
/// Example: "equal" and "different", "less than" and "greater or equal"
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

namespace ka {
  // TODO C++14: Make the functions in this file constexpr.

  /// Checks that the relation is transitive for all values in the given range.
  /// See the file comment for an explanation of what "transitive" means.
  /// For this test to be exact, 'values' must contain all values in the domain of R.
  /// Relation<T> R, ReadableForwardRange<T> Rng
  template<typename R, typename Rng>
  bool is_transitive(R r, Rng values) {
    for (auto v0 = values; !is_empty(v0); pop(v0)) {
      auto const& a = front(v0);
      for (auto v1 = values; !is_empty(v1); pop(v1)) {
        auto const& b = front(v1);
        for (auto v2 = values; !is_empty(v2); pop(v2)) {
          auto const& c = front(v2);
          auto const ab = r(a, b);
          auto const ac = r(a, c);
          auto const ba = r(b, a);
          auto const bc = r(b, c);
          auto const ca = r(c, a);
          auto const cb = r(c, b);
          if (ab && bc) KA_TRUE_OR_RETURN_FALSE(ac);
          if (ac && cb) KA_TRUE_OR_RETURN_FALSE(ab);
          if (ba && ac) KA_TRUE_OR_RETURN_FALSE(bc);
          if (bc && ca) KA_TRUE_OR_RETURN_FALSE(ba);
          if (ca && ab) KA_TRUE_OR_RETURN_FALSE(cb);
          if (cb && ba) KA_TRUE_OR_RETURN_FALSE(ca);
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
  bool is_reflexive(R r, Rng values) {
    while (!is_empty(values)) {
      auto const& a = front(values);
      KA_TRUE_OR_RETURN_FALSE(r(a, a));
      pop(values);
    }
    return true;
  }

  /// Checks that the relation is symmetric for all values in the given range.
  /// See the file comment for an explanation of what "symmetric" means.
  /// For this test to be exact, 'values' must contain all values in the domain of R.
  /// Relation<T> R, ReadableForwardRange<T> Rng
  template<typename R, typename Rng>
  bool is_symmetric(R r, Rng values) {
    for (auto v0 = values; !is_empty(v0); pop(v0)) {
      auto const& a = front(v0);
      for (auto v1 = values; !is_empty(v1); pop(v1)) {
        auto const& b = front(v1);
        auto const ab = r(a, b);
        auto const ba = r(b, a);
        if (ab) KA_TRUE_OR_RETURN_FALSE(ba);
        if (ba) KA_TRUE_OR_RETURN_FALSE(ab);
      }
    }
    return true;
  }

  /// Checks that the relation is an equivalence for all values in the given range.
  /// See the file comment for an explanation of what "equivalence" means.
  /// For this test to be exact, 'values' must contain all values in the domain of R.
  /// Relation<T> R, ReadableForwardRange<T> Rng
  template<typename R, typename Rng>
  bool is_equivalence(R r, Rng values) {
    // is_transitive is the last because it is the most complex (O^3).
    return is_reflexive(r, values) && is_symmetric(r, values) && is_transitive(r, values);
  }

  /// Checks that the relation is an equivalence for all values in the given range.
  /// See the file comment for an explanation of what "equivalence" means.
  /// For this test to be exact, 'values' must contain all values in the domain of R.
  /// Relation<T>
  template<typename R, typename T>
  bool is_equivalence(R r, std::initializer_list<T> values) {
    return is_equivalence(r, bounded_range(values));
  }

  /// Checks that the relations are complement of one another for all values in
  /// the given range.
  /// See the file comment for an explanation of what a "complement" means.
  /// For this test to be exact, 'values' must contain all values in the domain
  /// of R0.
  /// Relation<T> R0, Relation<T> R1, ReadableForwardRange<T> Rng
  template<typename R0, typename R1, typename Rng>
  bool are_complement(R0 r0, R1 r1, Rng values) {
    for (auto v0 = values; !is_empty(v0); pop(v0)) {
      auto const& a = front(v0);
      for (auto v1 = values; !is_empty(v1); pop(v1)) {
        auto const& b = front(v1);
        KA_TRUE_OR_RETURN_FALSE(r0(a, b) != r1(a, b));
      }
    }
    return true;
  }

  /// Checks that the relation is weak trichotomic for all values in the given range.
  /// See the file comment for an explanation of what "weak trichotomic" means.
  /// For this test to be exact, 'values' must contain all values in the domain of R.
  /// Precondition: equivalence(e)
  /// Relation<T> R0, Relation<T> R1, ReadableForwardRange<T> Rng
  template<typename R0, typename R1, typename Rng>
  bool is_weak_trichotomic(R0 r, R1 e, Rng values) {
    for (auto v0 = values; !is_empty(v0); pop(v0)) {
      auto const& a = front(v0);
      for (auto v1 = values; !is_empty(v1); pop(v1)) {
        auto const& b = front(v1);
        bool x = r(a, b), y = r(b, a), z = e(a, b);
        KA_TRUE_OR_RETURN_FALSE(( x && !y && !z)
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
  bool is_trichotomic(R0 r, Rng values) {
    using T = RemoveCvRef<decltype(front(values))>;
    return is_weak_trichotomic(r, std::equal_to<T>{}, values);
  }

  /// Checks that the relation is a weak ordering for all values in the given range.
  /// See the file comment for an explanation of what "weak ordering" means.
  /// For this test to be exact, 'values' must contain all values in the domain of R.
  /// Precondition: equivalence(e)
  /// Relation<T> R0, Relation<T> R1, ReadableForwardRange<T> Rng
  template<typename R0, typename R1, typename Rng>
  bool is_weak_ordering(R0 r, R1 e, Rng values) {
    return is_weak_trichotomic(r, e, values) && is_transitive(r, values);
  }

  /// Checks that the relation is a total ordering for all values in the given range.
  /// See the file comment for an explanation of what "total ordering" means.
  /// For this test to be exact, 'values' must contain all values in the domain of R.
  /// Relation<T> R, ReadableForwardRange<T> Rng
  template<typename R, typename Rng>
  bool is_total_ordering(R r, Rng values) {
    using T = RemoveCvRef<decltype(front(values))>;
    return is_weak_ordering(r, std::equal_to<T>{}, values);
  }

  /// Checks that the relation is a total ordering for all values in the given range.
  /// See the file comment for an explanation of what "total ordering" means.
  /// For this test to be exact, 'values' must contain all values in the domain of R.
  /// Relation<T> R, ReadableForwardRange<T> Rng
  /// Relation<T>
  template<typename R, typename T>
  bool is_total_ordering(R r, std::initializer_list<T> values) {
    return is_total_ordering(r, bounded_range(values));
  }
} // namespace ka

#endif // KA_RELATIONPREDICATE_HPP
