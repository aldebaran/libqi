#ifndef KA_SRC_HPP
#define KA_SRC_HPP
#pragma once
#include <type_traits>
#include "macro.hpp"
#include "macroregular.hpp"
#include "typetraits.hpp"
#include "utility.hpp"

/// @file
/// Contains functions and types related to the Readable concept.
///
/// See concept.hpp

namespace ka {
  /// Sources the parameter, that is gets its underlying value.
  ///
  /// `src` algorithm:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// if (*x is defined)
  ///   return *x
  /// else
  ///   return x
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// The meaning of this behavior is to extend the notion of indirection so as
  /// to accept "zero-length indirection", where the parameter *is* its own
  /// underlying value.
  ///
  /// For example, `int` can be considered as a "pointer" on its own value. This
  /// in turn allows code simplification by turning `int` into a full-fledged
  /// iterator type.
  ///
  /// Consider this code:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// std::vector<int> v(0, 100); // iterator constructor: _THIS IS INCORRECT_
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// What we want here is to insert the integer sequence 0, 1, 2, ..., 99 into
  /// the vector. The problem is, even if `int` models the navigational part of a
  /// random-access iterator (`++i`, `--i`, `i + n`, `i - n`, etc.), it does not
  /// model the value access part of an iterator (`*` is not defined on `int`).
  /// If `std::vector`'s constructor would use `src` instead of `*`, it would
  /// work.
  ///
  /// The reason is, if generic code uses `src` instead of `*`, `int` becomes a
  /// real iterator type.
  /// It then becomes possible to write the following code:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // ReadableIterator<Integral> I
  /// template<typename I>
  /// auto my_sum(I begin, I end) -> decltype(src(std::declval<I>())) {
  ///   // Precondition: begin != end
  ///   auto x = src(begin);
  ///   while (begin != end) {
  ///     x = x + src(begin);
  ///     ++begin;
  ///   }
  ///   return x;
  /// }
  ///
  /// // Classical usage:
  /// std::array<float, 3> v;
  /// // fill `v`...
  /// float f = my_sum(begin(v), end(v));
  ///
  /// // New usage (`i` and `j` are `int` with `i <= j`):
  /// int k = my_sum(i, j);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// See [Elements of Programming (Stepanov-MacJones, 2009)](http://elementsofprogramming.com/)
  /// section 6.1 for more information.
  ///
  /// Warning: When making a type model the concepts `Readable` / `Mutable`, make
  /// sure to implement ref _and_ const ref version if you want so, otherwise the
  /// default template version may take precedence.
  ///
  /// Remark: The general template overloads do not accept r-value references to
  ///   avoid excessive precedence.
  ///
  /// Warning: Do not remove the trailing return as it has a SFINAE role.
  ///
  /// Readable R
  template<typename R>
  BOOST_CONSTEXPR
  auto src(R&& r) KA_NOEXCEPT_EXPR(*fwd<R>(r)) -> decltype(*fwd<R>(r)) {
    return *fwd<R>(r);
  }

  template<typename R, typename =
    EnableIf<!HasOperatorStar<Decay<R>>::value>>
  BOOST_CONSTEXPR
  R&& src(R&& r) KA_NOEXCEPT(true) {
    return fwd<R>(r);
  }

  /// Polymorphic function object that returns the source of a Readable.
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int const i = 42;
  /// src_t src;
  /// ASSERT_EQ(i, src(Future<int>{i}));
  ///
  /// std::unique_ptr<int> p{new int{}};
  /// ASSERT_EQ(i, src(p));
  ///
  /// ASSERT_EQ(i, src(i));
  ///
  /// auto f = compose(src, UnitFuture{});
  /// ASSERT_EQ(i, f(i));
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct src_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(src_t)
  // PolymorphicFunction<S (R)>:
    /// Readable R
    template<typename R>
    auto operator()(R&& r) const KA_NOEXCEPT_EXPR(src(fwd<R>(r)))
        -> decltype(src(fwd<R>(r))) {
      return src(fwd<R>(r));
    }
  };
} // namespace ka

#endif // KA_SRC_HPP
