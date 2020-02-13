#ifndef KA_PRODUCTITER_HPP
#define KA_PRODUCTITER_HPP
#pragma once
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <boost/iterator/iterator_facade.hpp>
#include "integersequence.hpp"
#include "macro.hpp"
#include "macroregular.hpp"
#include "typetraits.hpp"
#include "utility.hpp"

/// @file
///
/// Contains a type template that implements the product of iterators, that is
/// an iterator that groups sub-iterators and advances, dereferences, etc., them
/// all at once.
///
/// # Added components
///
/// - Type contructors:
///     + `product_iter_t: InputIterator... -> InputIterator`
///
/// - Constructor functions (with InputIterator... I, Linearizable... L):
///     + `product_iter: I... -> product_iter_t<I...>`
///     + `product_begin: L... -> product_iter_t<Iterator<L>...>`
///     + `product_end: L... -> product_iter_t<Iterator<L>...>`
///     + `product_end_same_size: L... -> product_iter_t<Iterator<L>...>`
///
/// - Functions (with InputIterator... I, InputIterator J, K):
///     + `proj<n>: product_iter_t<I...> -> I_n` (i.e. nth type in I...)
///     + `iterator_cat_ops::operator*`: J * K -> product_iter_t<J, K>
///
/// - Types:
///     + `unit_iter_t: InputIterator`
///
/// - Constants:
///     + `unit_iter: unit_iter_t`

namespace ka {

namespace product_iter_detail {
  // The iterator category of a product of iterators is the lowest of all these
  // iterators. The order is defined as: input < forward < bidirectional <
  // random access.
  //
  // We need to implement a compile-time minimum algorithm, returning the
  // minimum iterator category from a compile-time list. To do this, we first
  // convert iterator categories to integers, then find the minimum integer, and
  // finally convert back this integer to the corresponding iterator category.
  // It is easier to order integers than to define the ordering for all pairs of
  // iterator categories.

  // Alias to represent a compile-time integer.
  template<int i>
  using Int = int_constant_t<i>;

  // Alias to represent a compile-time boolean.
  template<bool b>
  using Bool = bool_constant_t<b>;

  // Converts a type to a compile-time integer.
  template<typename A>
  struct ToIntImpl;

  // Converts a compile-time integer to a type.
  template<typename A>
  struct FromIntImpl;

  template<typename T>
  using ToInt = typename ToIntImpl<T>::type;

  template<typename T>
  using FromInt = typename FromIntImpl<T>::type;

// Maps a type to a compile-time integer, and vice-versa.
#define KA_DEFINE_TYPE_TO_FROM_INT(T, i) \
    template<>                           \
    struct ToIntImpl<T> {                \
      using type = Int<i>;               \
    };                                   \
    template<>                           \
    struct FromIntImpl<Int<i>> {         \
      using type = T;                    \
    };

  // Maps iterator categories to compile-time integer, and vice-versa.
  KA_DEFINE_TYPE_TO_FROM_INT(std::input_iterator_tag, 0)
  KA_DEFINE_TYPE_TO_FROM_INT(std::forward_iterator_tag, 1)
  KA_DEFINE_TYPE_TO_FROM_INT(std::bidirectional_iterator_tag, 2)
  KA_DEFINE_TYPE_TO_FROM_INT(std::random_access_iterator_tag, 3)
#undef KA_DEFINE_TYPE_TO_FROM_INT

  // Minimum of two integers.
  template<typename A, typename B, typename ALessThanB>
  struct MinInt2Impl;

  template<int i, int j>
  struct MinInt2Impl<Int<i>, Int<j>, true_t> {
    using type = Int<i>;
  };

  template<int i, int j>
  struct MinInt2Impl<Int<i>, Int<j>, false_t> {
    using type = Int<j>;
  };

  template<typename A, typename B, typename ALessThanB>
  using MinInt2 = typename MinInt2Impl<A, B, ALessThanB>::type;

  // Minimum of n integers.
  // Algorithm:
  //  Incrementally replace the two head integers by the minimum one.
  //  When the list contains one element only, it is the minimum.
  template<typename... T>
  struct MinIntNImpl;

  template<typename T>
  struct MinIntNImpl<T> {
    using type = T;
  };

  template<int i, int j, typename... T>
  struct MinIntNImpl<Int<i>, Int<j>, T...>
    : MinIntNImpl<MinInt2<Int<i>, Int<j>, Bool<(i < j)>>, T...> {
  };

  template<typename... T>
  using MinIntN = typename MinIntNImpl<T...>::type;

  // Returns the lowest iterator category.
  template<typename... T>
  struct MinIterCatImpl {
    using type = FromInt<MinIntN<ToInt<T>...>>;
  };

  // Returns the smallest of two types.
  template<typename T, typename U, typename TLessThanU>
  struct MinSize2 {
    using type = T;
  };

  template<typename T, typename U>
  struct MinSize2<T, U, false_t> {
    using type = U;
  };

  // Returns the smallest of n types, according to `sizeof`.
  // The algorithm is similar to `MinInt`.
  template<typename... T>
  struct MinSize;

  template<typename T>
  struct MinSize<T> {
    using type = T;
  };

  template<typename T, typename U, typename... V>
  struct MinSize<T, U, V...>
    : MinSize<typename MinSize2<T, U, Bool<(sizeof(T) < sizeof(U))>>::type, V...> {
  };

  template<typename... T>
  using MinIterCat = typename MinIterCatImpl<T...>::type;

  // Convenient aliases to iterator affiliated types.
  template<typename T>
  using Value = typename std::iterator_traits<T>::value_type;

  template<typename T>
  using Reference = typename std::iterator_traits<T>::reference;

  template<typename T>
  using Difference = typename std::iterator_traits<T>::difference_type;

  template<typename T>
  using Category = typename std::iterator_traits<T>::iterator_category;
} // namespace product_iter_detail

// Base of `product_iter_t`. This macro avoids manual duplication of the code.
// It is undefined below.
#define KA_PRODUCT_ITER_BASE(I)                                                        \
  boost::iterator_facade<                                                              \
    product_iter_t<I...>,                                                              \
    std::tuple<product_iter_detail::Value<I>...>,                                      \
    product_iter_detail::MinIterCat<product_iter_detail::Category<I>...>,              \
    std::tuple<product_iter_detail::Reference<I>...>,                                  \
    typename product_iter_detail::MinSize<product_iter_detail::Difference<I>...>::type \
  >

/// Product of iterators: groups sub-iterators and advances, dereferences, etc.,
/// them all at once.
///
/// This type is a product in the categorical sense: it strictly groups
/// sub-iterators with no extra-information, and comes with projections to get
/// them back. The underlying category is constructed over those of discrete
/// dynamical systems (sets equipped with a transformation), with the additional
/// structure of the dereferencement.
/// See [Conceptual Mathematics, (Lawvere-Schanuel, 2009)](https://www.cambridge.org/core/books/conceptual-mathematics/00772F4CC3D4268200C5EC86B39D415A), article III
///
/// This iterator has the lowest iterator category of its sub-iterators, the
/// order being: input < forward < bidirectional < random access.
///
/// To construct product of iterators, use the dedicated constructor functions:
/// `product_iter`, `product_begin`, `product_end`, `product_end_same_size`
///
/// Note: The operations of the product of iterators are specified in terms of
///   laws. The following notation is used:
///     - `i * j` denotes product of iterators `i` and `j`
///     - `t * u` denotes product of values `t` and `u`, i.e. tuple of `t` and `u`
///     - `n` denotes a signed integer
///     - `i + n` denotes iterator `i` moved forward by `n` steps
///     - `j - i` denotes distance from iterator `i` to iterator `j`
///     - `1i` denotes iterator unit
///     - `1` denotes unit
///     - `=` denotes equality
///     - `≅` denotes isomorphism
///
/// Laws:
///     *(i * ...)  =  ((*i) * ...)             (dereferencement)
///  && (i * ...) + n  =  (i + n) * ...         (forward / backward move)
///  && (j * ...) - (i * ...)  =  n, such that
///       (j * ...)  =  (i * ...) + n, that is
///       + is a right group action.            (difference)
///  && i * 1i  ≅  1i * i  ≅  i                 (multiplication by unit)
///  && 1i + n  =  1i                           (rest state 1)
///  && 1i - 1i  =  0                           (rest state 2)
///  && *1i  =  1                               (dereferencement of unit)
///
/// Note: Output iterators are not supported as their behavior differs too much
///   from other iterator categories (`void` value type, etc.).
///
/// Example: Iterating through several containers at once.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// // Containers are assumed to be large enough.
/// // std::vector<X> v;
/// // std::array<Y, 10> a;
/// auto b = product_iter(v.begin(), a.begin());
/// auto e = product_iter(v.begin() + 2, a.begin() + 2);
/// while (b != e) {
///   auto values = *b; // `values` has type `std::tuple<X, Y>`.
///   ... // Use `values` (`ka::apply` can be used to call non-tuple input functions.)
///   ++b;
/// }
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Example: Iterating through several containers with different sizes.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// // Computing the end product iterator is O(n) since we have to determine the
/// // smallest container first.
/// // std::vector<X> v;  // size = 4
/// // std::list<Y> l;    // size = 2
/// // std::deque<Z> d;   // size = 10
/// // `product_begin(v, l, d) == product_iter(v.begin(), l.begin(), d.begin())`
/// auto b = product_begin(v, l, d);
/// auto e = product_end(v, l, d); // Ends depends on the smallest container.
/// // Will loop twice since the smallest container has size 2.
/// while (b != e) {
///   auto values = *b; // `values` has type `std::tuple<X, Y, Z>`.
///   ... // Use `values` (`ka::apply` can be used to call non-tuple input functions.)
///   ++b;
/// }
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Example: Iterating through several containers with same size.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// // Computing the end product iterator is O(1) since we know all containers
/// // have same size.
/// // std::vector<X> v;  // size = 4
/// // std::list<Y> l;    // size = 4
/// // std::deque<Z> d;   // size = 4
/// auto b = product_begin(v, l, d);
/// auto e = product_end_same_size(v, l, d); // Takes end iterator of each container.
/// // Will loop four times.
/// while (b != e) {
///   auto values = *b; // `values` has type `std::tuple<X, Y, Z>`.
///   ... // Use `values` (`ka::apply` can be used to call non-tuple input functions.)
///   ++b;
/// }
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// InputIterator... I
template<typename... I>
struct product_iter_t : KA_PRODUCT_ITER_BASE(I) {
  using base = KA_PRODUCT_ITER_BASE(I);
#undef KA_PRODUCT_ITER_BASE
  using typename base::reference;
  using typename base::difference_type;
  using indexes_type = index_sequence_for<I...>;
  std::tuple<I...> iters;

  explicit
  product_iter_t(std::tuple<I...> const& t) : iters(t) {
  }
  explicit
  product_iter_t(std::tuple<I...>&& t) : iters(std::move(t)) {
  }
// Regular:
  explicit
  product_iter_t() = default;

  bool equal(product_iter_t const& x) const KA_NOEXCEPT_EXPR(iters == x.iters) {
    return iters == x.iters;
  }
// InputIterator:

  //     *(i * ...) = ((*i) * ...)
  reference dereference() const {
    return do_dereference(indexes_type{});
  }
  //  && (i * ...) + n = (i + n) * ...
  void increment() {
    do_increment(indexes_type{});
  }
// BidirectionalIterator (if BidirectionalIterator... I):
  void decrement() {
    do_decrement(indexes_type{});
  }
// RandomAccessIterator (if RandomAccessIterator... I):
  /// Integral N
  template<typename N>
  void advance(N n) {
    do_advance(n, indexes_type{});
  }
  difference_type distance_to(product_iter_t x) const {
    // There may be an algorithm better than this O(n) one...
    // But it strictly follows the law of difference stated above, and therefore
    // is correct for any orbit-shapes (infinite, circular, etc.).
    auto n = difference_type(0);
    while (x.iters != iters) {
      ++n;
      x.do_increment(indexes_type{});
    }
    return -n;
  }
private:
  template<std::size_t... i>
  reference do_dereference(index_sequence<i...>) const {
    return reference{*std::get<i>(iters)...};
  }

  template<std::size_t... i>
  void do_increment(index_sequence<i...>) {
    (void)std::initializer_list<int>{0, (void(++std::get<i>(iters)), 0)...};
  }

  template<std::size_t... i>
  void do_decrement(index_sequence<i...>) {
    (void)std::initializer_list<int>{0, (void(--std::get<i>(iters)), 0)...};
  }

  template<std::size_t... i, typename N>
  void do_advance(N n, index_sequence<i...>) {
    (void)std::initializer_list<int>{0, (std::advance(std::get<i>(iters), n), 0)...};
  }
};

/// Empty specialization.
/// This is the unit type of the category of iterators.
///
/// See non-specialized version for info.
template<>
struct product_iter_t<> : boost::iterator_facade<
    product_iter_t<>,
    std::tuple<>,
    std::random_access_iterator_tag,
    std::tuple<>,
    std::ptrdiff_t
  >
{
// InputIterator:
  constexpr
  reference dereference() const noexcept {
    return std::tuple<>{};
  }
  constexpr
  bool equal(product_iter_t const&) const noexcept {
    return true;
  }
  // `void` cannot be returned for the method to be `constexpr`...
  // `iterator_facade` impose no constraint on the return type, so `int` is ok.
  constexpr
  int increment() const noexcept {
    return 0;
  }
// BidirectionalIterator (if BidirectionalIterator... I):
  constexpr
  int decrement() const noexcept {
    return 0;
  }
// RandomAccessIterator (if RandomAccessIterator... I):
  /// Integral N
  template<typename N> constexpr
  int advance(N) const noexcept {
    return 0;
  }
  constexpr
  difference_type distance_to(product_iter_t const&) const noexcept {
    return 0;
  }
};

/// Random access iterator that is its own successor and predecessor.
/// Its value type is `unit_t`.
/// As `unit_t`, this iterator type does not hold any information.
using unit_iter_t = product_iter_t<>;

/// Unique value of `unit_iter_t`.
constexpr unit_iter_t unit_iter;

/// Contructs a product of iterators, performing argument type deduction.
///
/// InputIterator... I
template<typename... I>
auto product_iter(I&&... i) -> product_iter_t<Decay<I>...> {
  return product_iter_t<Decay<I>...>{product(fwd<I>(i)...)};
}

// Put `begin`/`end` functions in the detail namespace to be able to leverage
// `ADL` in `decltype` clauses.
namespace product_iter_detail {
  using std::begin;
  using std::end;

  template<typename L>
  auto adl_begin(L&& l) -> decltype(begin(fwd<L>(l))) {
    return begin(fwd<L>(l));
  }

  template<typename L>
  auto adl_end(L&& l) -> decltype(end(fwd<L>(l))) {
    return end(fwd<L>(l));
  }

  // Some containers have a `size` method, returning `size_type` which is
  // typically `std::size_t` (unsigned), while others don't have such a method.
  // In the latter case `std::distance` is used, but it returns the iterator
  // `difference_type`, which is typically `std::ptrdiff_t` (signed). A cast
  // must be done. Fortunately, the context guarantees a non-null
  // `difference_type`, so the only problem could be in a difference of ranges
  // between `size_type` and `difference_type`.
  template<typename T>
  auto total_size(true_t /* HasMemberSize */, T const& t) -> decltype(t.size()) {
    return t.size();
  }

  template<typename T>
  auto total_size(false_t /* HasMemberSize */, T const& t) -> std::size_t {
    return static_cast<std::size_t>(std::distance(begin(t), end(t)));
  }

  // Precondition: n <= size
  template<typename L, typename N>
  auto iter_at(L&& l, N n, N size) -> decltype(end(fwd<L>(l))) {
    if (n == size) {
      return end(fwd<L>(l));
    }
    return std::next(begin(fwd<L>(l)), n);
  }

  // Computes the minimum size, then advances iterators accordingly.
  template<typename... L, std::size_t... i>
  auto product_end(index_sequence<i...>, L&&... l)
      -> decltype(ka::product_iter(end(fwd<L>(l))...)) {
    // This is a slight simplification: we should use internal `distance_type`
    // with a special case for `std::array`.
    std::array<std::size_t, sizeof...(L)> sizes =
      {{total_size(HasMemberSize<Decay<L>>{}, l)...}};
    auto const it = std::min_element(sizes.begin(), sizes.end());
    return product_iter(iter_at(fwd<L>(l), *it, sizes[i])...);
  }
} // namespace product_iter_detail

/// Product of begin iterators.
///
/// invariant: product_begin(l...) == product_iter(begin(l)...)
///
/// Linearizable... L
template<typename... L>
auto product_begin(L&&... l)
    -> decltype(product_iter(product_iter_detail::adl_begin(fwd<L>(l))...)) {
  return product_iter(product_iter_detail::adl_begin(fwd<L>(l))...);
}

/// Product of iterators aligned with the smallest end.
///
/// invariant: product_end(l...) == product_iter(begin(l) + n...), where
///            n == min(size(l)...) (min and size are specification-only)
///
/// Complexity: Constructing the end iterator requires finding the minimum size
///   of each range. Computing the size of a standard container is O(1), except
///   for `std::forward_list` which is O(n). Then, finding the minimum of these
///   sizes is O(n) in the number of containers.
///
/// Linearizable... L
template<typename... L>
auto product_end(L&&... l) -> decltype(product_iter_detail::product_end(
    index_sequence_for<L...>{}, fwd<L>(l)...)) {
  return product_iter_detail::product_end(
    index_sequence_for<L...>{}, fwd<L>(l)...);
}

/// Product of end iterators.
///
/// Precondition: (forall x,y in l...) x.size() == y.size()
/// Invariant: product_end(l...) == product_iter(end(l)...)
///
/// Note: If the size is known to be the same, an alternative to create an end
///   iterator and form a bounded range (a begin-end pair of iterators) is to
///   directly use a counted range (a begin iterator together with a distance).
///
/// Linearizable... L
template<typename... L>
auto product_end_same_size(L&&... l)
    -> decltype(product_iter(product_iter_detail::adl_end(fwd<L>(l))...)) {
  return product_iter(product_iter_detail::adl_end(fwd<L>(l))...);
}

/// Projection of n th iterator of the product.
template<std::size_t n, typename... I>
auto proj(product_iter_t<I...> const& x) -> decltype(std::get<n>(x.iters)) {
  return std::get<n>(x.iters);
}

/// Projection of n th iterator of the product.
template<std::size_t n, typename... I>
auto proj(product_iter_t<I...>& x) -> decltype(std::get<n>(x.iters)) {
  return std::get<n>(x.iters);
}

/// Projection of n th iterator of the product.
template<std::size_t n, typename... I>
auto proj(product_iter_t<I...>&& x) -> decltype(std::get<n>(std::move(x.iters))) {
  return std::get<n>(std::move(x.iters));
}

/// Categorical operators for iterators.
namespace iterator_cat_ops {

/// InputIterator I, InputIterator J
template<typename I, typename J,
  typename = EnableIfInputIterator<I>, typename = EnableIfInputIterator<J>>
auto operator*(I i, J j) -> decltype(product_iter(i, j)) {
  return product_iter(std::move(i), std::move(j));
}

} // namespace iterator_ops

} // namespace ka

#endif // KA_PRODUCTITER_HPP
