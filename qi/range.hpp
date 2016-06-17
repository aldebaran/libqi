#pragma once

#ifndef _QI_RANGE_HPP_
#define _QI_RANGE_HPP_

#include <iterator>
#include <limits>
#include <utility>
#include <qi/type/traits.hpp>
#include <qi/macroregular.hpp>

/// @file range.hpp
/// This file formally defines range properties where ranges are a pair of
/// iterators, or an iterator with a distance.
/// It also defines a family of Range concepts which does not expose iterators.
/// For now, it is only concerned with forward ranges, but other traversal
/// (bidirectional, random) and access (write-only, read-write) can be added on top.
/// See conceptpredicates.hpp for the f^n(x) notation.
///
/// Property(Iterator I, Integer N)
/// weakRange: I x N
/// (b, n) |-> (forall i in N) 0 <= i <= n implies (++)^i(b) is defined
/// This means that weakRange is a property taking an Iterator and an Integer
/// and being true if it is defined to increment the iterator up to n times
/// (n included).
/// It is possible to have a cycle.
///
/// Properties are comment-only because it's not always possible to efficiently
/// implement them as real code.
///
/// Property(Iterator I, Integer N)
/// countedRange: I x N
/// (b, n) |-> weakRange(b, n)
///    && (forall i, j in N) 0 <= i < j <= n implies (++)^i(b) != (++)^j(b)
///
/// A counted range is a weak range without cycle.
///
/// Property(Iterator I)
/// boundedRange: I x I
/// (b, e) |-> (there exists i in Distance<I>) countedRange(b, i) && (++)^i(b) == e
///
/// A bounded range is a counted range with the end defined with an iterator
/// instead of a distance.
///
/// Range(R) =
///     Regular(R)
///  && isEmpty: R -> bool
///  && pop:     R -> void
///  && pop is not necessarily regular
///
/// In this basic concept, you can "iterate" through the range but not
/// access the values. Which can be useful if you're only interested in
/// advancing the front of the range.
/// The typical use is :
/// while (!isEmpty(myRange)) {
///   <some code>
///   pop(myRange);
/// }
/// Note that isEmpty is not guaranteed to ever return false, i.e. the
/// range could be infinite.
/// Also, pop being not necessarily regular, the traversal on a copy of a range
/// is not guaranteed to yield the same result (e.g. useful for input streams).
///
/// ForwardRange(R) =
///     Range(R)
///  && pop is regular (i.e. the range is multipass)
///
/// ReadableRange(R) =
///     Range(R)
///  && front: R -> U where Regular(U)
///
/// ReadableForwardRange(R) =
///     ForwardRange(R)
///  && ReadableRange(R)
///
/// MutableForwardRange(R) =
///     ReadableForwardRange(R)
///  && (forall r in R where front(r) is defined) front(r) = x establishes front(r) == x

namespace qi
{
  // If these usings are not specified, unqualified begin will not work on native
  // arrays.
  // If an array-specialized begin is added in namespace qi, it will cause
  // ambiguity for arrays of std types because of ADL : qi array-specialized or
  // std array-specialized ?
  // With these usings, the right behaviour is obtained :
  // - if a qi type specifies a free function begin, it is called
  // - if a qi type specifies a member begin, it is called
  // - begin is ok with std types
  // - begin is ok with array types (std or qi)
  // Same situation for end of course.
  using std::begin;
  using std::end;

  /// Half-open bounded range defined with a pair of iterators.
  /// Free functions are used because it is more powerful in a generic context
  /// (non-intrusive so ok with native types and with types you have no control over).
  /// Also, some functions (front) cannot be defined inside the class,
  /// because of the trailing return type that is always compiled, even if the
  /// function is never used. This in turn force the members to be public,
  /// because there is no way in C++11 to friend-declare an auto-returning function.
  ///
  /// Use example (findBest taking a Range) :
  /// auto b = begin(memories);
  /// auto memory = findBest(boundedRange(b, b + memories.size() / 2u));
  ///
  /// Iterator I
  template<typename I>
  struct BoundedRange
  {
    using Self = BoundedRange;
    I b, e; /// Precondition: boundedRange(b, e)
  // Regular:
    // Default construction, copy, assignment, destruction by default.
    QI_GENERATE_FRIEND_REGULAR_OPS_2(Self, b, e) // ==, !=, <, <=, >, >=
  // Range:
    friend bool isEmpty(const Self& x)
    {
      return x.b == x.e;
    }
    /// Precondition: !isEmpty(x)
    friend void pop(Self& x)
    {
      ++x.b;
    }
  // ReadableRange:
    // front defined outside the class, because otherwise the trailing return type
    // causes a compilation failure. This is because it is always compiled, even
    // if front is never called. This is problematic if the user wants a Range
    // (not a ReadableRange).
  // MutableRange:
    // front defined outside the class.
  };

  /// Precondition: !isEmpty(x)
  template<typename I>
  auto front(const BoundedRange<I>& x) -> decltype(*x.b)
  {
    return *x.b;
  }

  /// Precondition: !isEmpty(x)
  template<typename I>
  auto front(BoundedRange<I>& x) -> decltype(*x.b)
  {
    return *x.b;
  }

  /// Sequence S
  template<typename S>
  auto boundedRange(S& s) -> BoundedRange<decltype(begin(s))>
  {
    return {begin(s), end(s)};
  }

  /// Precondition: boundedRange(b, e) (we're talking about the property here)
  /// Iterator I
  template<typename I>
  BoundedRange<I> boundedRange(I b, I e)
  {
    return {b, e};
  }

  /// Half-open bounded range defined with an ordered pair of values,
  /// the first one being less than the second one.
  /// Values can be of arithmetic types for example.
  /// Note: If operator* would be defined on all builtin types, returning
  /// the value itself if not a pointer, BoundedRange could be used instead.
  ///
  /// Use example (with sum taking a Range) :
  /// auto n = sum(incrRange(2, 10));
  /// auto m = sum(incrRange(10)); // The begin value is default-initialized (here, 0).
  ///
  /// Incrementable N
  template<typename N>
  struct IncrBoundedRange
  {
    using Self = IncrBoundedRange;
    N b, e; /// Precondition: boundedRange(b, e)
  // Regular:
    // Default construction, copy, assignment, destruction by default.
    QI_GENERATE_FRIEND_REGULAR_OPS_2(Self, b, e) // ==, !=, <, <=, >, >=
  // Range:
    friend bool isEmpty(const Self& x)
    {
      return x.b == x.e;
    }
    /// Precondition: !isEmpty(x)
    friend void pop(Self& x)
    {
      ++x.b;
    }
  // ReadableRange:
    /// Precondition: !isEmpty(x)
    friend const N& front(const Self& x)
    {
      return x.b;
    }
  // MutableRange:
    /// Precondition: !isEmpty(x)
    friend N& front(Self& x)
    {
      return x.b;
    }
  };

  /// Preconditions: boundedRange(b, e)
  /// Incrementable N
  template<typename N>
  IncrBoundedRange<traits::RemoveCvRef<N>> incrRange(N&& b, N&& e)
  {
    return {std::forward<N>(b), std::forward<N>(e)};
  }

  /// Preconditions: boundedRange(N{}, e)
  /// Incrementable N
  template<typename N>
  IncrBoundedRange<traits::RemoveCvRef<N>> incrRange(N&& e)
  {
    return {N{}, std::forward<N>(e)};
  }

  /// Arithmetic N
  template<typename N>
  IncrBoundedRange<N> incrRange()
  {
    return {std::numeric_limits<N>::min(), std::numeric_limits<N>::max()};
  }

  /// Idem IncrBoundedRange but an Action performs the increment.
  /// Useful if you don't want to add an operator++ to N.
  ///
  /// Use example (with sum taking a Range) :
  /// auto n = sum(incrRange(1, 64, [](int& i){i *= 2;})); // be careful to not step over the limit
  ///
  /// Incrementable N, Action<N> A
  template<typename N, typename A>
  struct IncrBoundedRangeAction
  {
    using Self = IncrBoundedRangeAction;
    N b, e; /// Precondition: boundedRange(b, e)
    A incr;
  // Regular:
    // Default construction, copy, assignment, destruction by default.
    QI_GENERATE_FRIEND_REGULAR_OPS_3(Self, b, e, incr) // ==, !=, <, <=, >, >=
  // Range:
    friend bool isEmpty(const Self& x)
    {
      return x.b == x.e;
    }
    /// Precondition: !isEmpty(x)
    friend void pop(Self& x)
    {
      x.incr(x.b);
    }
  // ReadableRange:
    /// Precondition: !isEmpty(x)
    friend const N& front(const Self& x)
    {
      return x.b;
    }
  // MutableRange:
    /// Precondition: !isEmpty(x)
    friend N& front(Self& x)
    {
      return x.b;
    }
  };

  /// Preconditions: boundedRange(b, e)
  /// Iterator N, Action<N> A
  template<typename N, typename A>
  IncrBoundedRangeAction<traits::RemoveCvRef<N>, traits::RemoveCvRef<A>> incrRange(N&& b, N&& e, A&& incr)
  {
    return {std::forward<N>(b), std::forward<N>(e), std::forward<A>(incr)};
  }

  /// Preconditions: boundedRange(N{}, e)
  /// Iterator N, Action<N> A
  template<typename N, typename A>
  IncrBoundedRangeAction<traits::RemoveCvRef<N>, traits::RemoveCvRef<A>> incrRange(N&& e, A&& incr)
  {
    return {traits::RemoveCvRef<N>{}, std::forward<N>(e), std::forward<A>(incr)};
  }

  /// Repeat the same value n times.
  ///
  /// Use example (with concat taking a Range) :
  /// auto s = concat(repeatRange(string("Y'a quelqu'un ? "), 3));
  ///
  /// Regular T, Incrementable N
  template<typename T, typename N>
  struct RepeatRange
  {
    using Self = RepeatRange;
    T value;
    N b, e; /// Preconditions: boundedRange(b, e)
  // Regular:
    // Default construction, copy, assignment, destruction by default.
    QI_GENERATE_FRIEND_REGULAR_OPS_3(Self, value, b, e) // ==, !=, <, <=, >, >=
  // Range:
    friend bool isEmpty(const Self& x)
    {
      return x.b == x.e;
    }
    /// Precondition: !isEmpty(x)
    friend void pop(Self& x)
    {
      ++x.b;
    }
  // ReadableForwardRange:
    /// Precondition: !isEmpty(x)
    friend const T& front(const Self& x)
    {
      return x.value;
    }
  };

  /// Preconditions: boundedRange(N{}, end)
  /// Regular T, Iterator N
  template<typename T, typename N>
  RepeatRange<traits::RemoveCvRef<T>, traits::RemoveCvRef<N>> repeatRange(T&& a, N&& end)
  {
    return {std::forward<T>(a), N{}, std::forward<N>(end)};
  }
} // namespace qi

#endif // _QI_RANGE_HPP_
