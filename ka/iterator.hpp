#ifndef KA_ITERATOR_HPP
#define KA_ITERATOR_HPP
#pragma once
#include <iterator>
#include "typetraits.hpp"
#include "utility.hpp"

/// @file
/// This file defines operators for iterators:
///
/// - `b + n` increments iterator `b` `n` times
/// - `e - b` gets the distance between `b` and `e`
///
/// The complexity of these operations depends on the iterator category.
///
/// For a formal justification of these operators, see
/// [Elements of Programming (Stepanov-MacJones, 2009)](http://elementsofprogramming.com/),
/// section 6.3.

namespace ka {
  namespace iterator_ops {
    template<typename I>
    using Difference = typename std::iterator_traits<Decay<I>>::difference_type;

    /// `std::next` does not accept input iterators until C++17, but
    /// `std::advance` does...
    ///
    /// TODO: Use `std::next` when C++17 is used.
    ///
    /// InputIterator I
    template<typename I>
    I operator+(I i, Difference<I> n) {
      std::advance(i, n);
      return i;
    }

    /// InputIterator I
    template<typename I>
    void operator+=(I& i, Difference<I> n) {
      std::advance(i, n);
    }

    /// BidirectionalIterator I
    template<typename I>
    I operator-(I i, Difference<I> n) {
      return std::next(std::move(i), -n);
    }

    /// BidirectionalIterator I
    template<typename I>
    void operator-=(I& i, Difference<I> n) {
      std::advance(i, -n);
    }

    /// InputIterator I
    template<typename I, typename = EnableIfInputIterator<I>>
    auto operator-(I const& e, I const& b) -> decltype(std::distance(b, e)) {
      return std::distance(b, e);
    }
  } // namespace iterator_ops
} // namespace ka

#endif // KA_ITERATOR_HPP
