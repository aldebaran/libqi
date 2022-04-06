#ifndef KA_RANGE_HPP
#define KA_RANGE_HPP
#pragma once
#include <iterator>
#include <limits>
#include <utility>
#include "functional.hpp"
#include "macroregular.hpp"
#include "src.hpp"
#include "typetraits.hpp"
#include "iterator.hpp"
#include "unit.hpp"
#include "utility.hpp"

/// @file
/// This file formally defines range properties where ranges are a pair of
/// iterators, or an iterator with a distance.
///
/// It also defines a family of Range concepts which does not expose iterators.
/// For now, it is only concerned with forward ranges, but other traversal
/// (bidirectional, random) and access (write-only, read-write) can be added on top.
///
/// See relationpredicate.hpp for the property notation.
///
/// See conceptpredicate.hpp for the f^n(x) notation.
///
/// Properties
/// =============================================
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Iterator I, Integer N)
/// weakRange: I x N
/// (b, n) |-> (forall i in N) 0 <= i <= n implies (++)^i(b) is defined
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This means that weakRange is a property taking an Iterator and an Integer
/// and being true if it is defined to increment the iterator up to n times
/// (n included).
/// It is possible to have a cycle.
///
/// Note: properties are comment-only because it's not always possible to
/// efficiently implement them as real code.
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Iterator I, Integer N)
/// countedRange: I x N
/// (b, n) |-> weakRange(b, n)
///    && (forall i, j in N) 0 <= i < j <= n implies (++)^i(b) != (++)^j(b)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A counted range is a weak range without cycle.
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Property(Iterator I)
/// boundedRange: I x I
/// (b, e) |-> (there exists i in Distance<I>) countedRange(b, i) && (++)^i(b) == e
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A bounded range is a counted range with the end defined with an iterator
/// instead of a distance.

namespace ka {
  // If these usings are not specified, unqualified begin will not work on native
  // arrays.
  // If an array-specialized begin is added in namespace ka, it will cause
  // ambiguity for arrays of std types because of ADL: ka array-specialized or
  // std array-specialized?
  // With these usings, the right behaviour is obtained:
  // - if a ka type specifies a free function begin, it is called
  // - if a ka type specifies a member begin, it is called
  // - begin is ok with std types
  // - begin is ok with array types (std or ka)
  // Same situation for end of course.
  using std::begin;
  using std::end;

  template<typename T, typename A = incr_t>
  struct bounded_range_t;

  namespace bounded_range_detail {
    using iterator_ops::operator+;

    template<typename I>
    using Value = typename std::iterator_traits<I>::value_type;

    // `at` is the implementation of `operator[]`, which is part of the
    // `Linearizable` concept. This concept is modeled only by
    // `bounded_range_t<T>` types where `T` is an iterator type.
    template<typename T, typename A>
    auto at(bounded_range_t<T, A> const& x, std::size_t i, /* IsIterator */ true_t)
        -> Value<T> const& {
      return *(x.b + i);
    }

    template<typename T, typename A>
    auto at(bounded_range_t<T, A>& x, std::size_t i, /* IsIterator */ true_t)
        -> Value<T>& {
      return *(x.b + i);
    }

    template<typename T, typename A>
    auto at(bounded_range_t<T, A>&& x, std::size_t i, /* IsIterator */ true_t)
        -> Value<T> {
      return mv(*(x.b + i));
    }

    template<typename T, typename A>
    auto at(bounded_range_t<T, A> const&, std::size_t, /* IsIterator */ false_t)
        -> void {
    }
  } // namespace bounded_range_detail

  /// Half-open bounded range defined with a pair of iterators.
  /// Free functions are used because it is more powerful in a generic context
  /// (non-intrusive so ok with native types and with types you have no control over).
  /// Also, some functions (front) cannot be defined inside the class,
  /// because of the trailing return type that is always compiled, even if the
  /// function is never used. This in turn force the members to be public,
  /// because there is no way in C++11 to friend-declare an auto-returning function.
  ///
  /// Note: In the examples below, `find_best` takes a `ReadableRange` as input.
  ///
  /// Example: ranging over a whole container
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto memory = find_best(bounded_range(memories));
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: ranging over a sub-range of a container
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto b = begin(memories);
  /// auto memory = find_best(bounded_range(b, b + memories.size() / 2u));
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: ranging over a sub-range of integers
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto res = find_best(bounded_range(-10, 10)); // from -10 to 10 excluded
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: ranging over all integers
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // from std::numeric_limits<int>::min()
  /// // to std::numeric_limits<int>::max() excluded
  /// auto res = find_best(bounded_range<int>());
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: ranging over a sub-range of a container, skipping on element
  ///   every two elements (dangerous: end iterator could be skipped over)
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// incr_t incr;
  /// auto memory = find_best(bounded_range(memories, incr * incr));
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Regular T, Action<T> A
  template<typename T, typename A>
  struct bounded_range_t {
    using self = bounded_range_t;
    T b, e;
    A incr;

    /// Precondition: (exists n >= 0) incr^n(b) == e
    bounded_range_t(T b, T e, A incr = {}) : b(b), e(e), incr(incr) {
    }
  // Regular:
    bounded_range_t() = default;
    // copy, assignment, destruction by default.
    KA_GENERATE_FRIEND_REGULAR_OPS_2(self, b, e) // ==, !=, <, <=, >, >=
  // Range:
    friend bool is_empty(self const& x) {
      return x.b == x.e;
    }
    /// Precondition: !is_empty(x)
    friend void pop(self& x) {
      x.incr(x.b);
    }
  // ReadableRange:
    // front defined outside the class, because otherwise the trailing return type
    // causes a compilation failure. This is because it is always compiled, even
    // if front is never called. This is problematic if the user wants a Range
    // (not a ReadableRange).
  // MutableRange:
    // front defined outside the class.

  // Linearizable (if InputIterator(T)):
    friend T begin(self const& x) {
      return x.b;
    }
    friend T end(self const& x) {
      return x.e;
    }
    bool empty() const {
      return is_empty(*this);
    }
    auto operator[](std::size_t i) const&
         -> decltype(bounded_range_detail::at(*this, i, IsInputIterator<T>{})) {
       return bounded_range_detail::at(*this, i, IsInputIterator<T>{});
    }
    auto operator[](std::size_t i) &
         -> decltype(bounded_range_detail::at(*this, i, IsInputIterator<T>{})) {
       return bounded_range_detail::at(*this, i, IsInputIterator<T>{});
    }
    auto operator[](std::size_t i) &&
         -> decltype(bounded_range_detail::at(*this, i, IsInputIterator<T>{})) {
       return bounded_range_detail::at(mv(*this), i, IsInputIterator<T>{});
    }
    // size: See outside class.
  };

  template<typename T, typename A>
  auto size(bounded_range_t<T, A> const& x) -> decltype(std::distance(x.b, x.e)) {
    return std::distance(x.b, x.e);
  }

  /// Precondition: !is_empty(x)
  template<typename T, typename A>
  auto front(bounded_range_t<T, A> const& x) -> decltype(src(x.b)) {
    return src(x.b);
  }

  /// Precondition: !is_empty(x)
  template<typename T, typename A>
  auto front(bounded_range_t<T, A>& x) -> decltype(src(x.b)) {
    return src(x.b);
  }

  /// Precondition: (exists n >= 0) incr(b)^n == e
  template<typename T, typename A = incr_t>
  bounded_range_t<T, A> bounded_range(T b, T e, A incr = {}) {
    return {b, e, incr};
  }

  /// Integral N
  template<typename N>
  bounded_range_t<N> bounded_range() {
    return {std::numeric_limits<N>::min(), std::numeric_limits<N>::max(), incr_t{}};
  }

  /// Linearizable<T> L, Action<T> A
  template<typename L, typename A = incr_t>
  auto bounded_range(L const& l, A incr = {}) -> decltype(bounded_range(begin(l), end(l), incr)) {
    return bounded_range(begin(l), end(l), incr);
  }

  /// Linearizable<T> L, Action<T> A
  template<typename L, typename A = incr_t>
  auto bounded_range(L& l, A incr = {}) -> decltype(bounded_range(begin(l), end(l), incr)) {
    return bounded_range(begin(l), end(l), incr);
  }

  /// Repeat the same value n times.
  ///
  /// Use example (with concat taking a Range) :
  /// auto s = concat(repeat_range(string("Y'a quelqu'un ? "), 3));
  ///
  /// Regular T, Incrementable N
  template<typename T, typename N>
  struct repeat_range_t {
    using self = repeat_range_t;
    T value;
    N b, e; /// Preconditions: boundedRange(b, e)
  // Regular:
    // Default construction, copy, assignment, destruction by default.
    KA_GENERATE_FRIEND_REGULAR_OPS_3(self, value, b, e) // ==, !=, <, <=, >, >=
  // Range:
    friend bool is_empty(self const& x) {
      return x.b == x.e;
    }
    /// Precondition: !is_empty(x)
    friend void pop(self& x) {
      ++x.b;
    }
  // ReadableForwardRange:
    /// Precondition: !is_empty(x)
    friend const T& front(self const& x) {
      return x.value;
    }
  };

  /// Preconditions: boundedRange(N{}, end)
  /// Regular T, Iterator N
  template<typename T, typename N>
  repeat_range_t<RemoveCvRef<T>, RemoveCvRef<N>> repeat_range(T&& a, N&& end) {
    return {fwd<T>(a), N{}, fwd<N>(end)};
  }
} // namespace ka

#endif // KA_RANGE_HPP
