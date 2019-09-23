#ifndef KA_FUNCTORCONTAINER_HPP
#define KA_FUNCTORCONTAINER_HPP
#pragma once
#include <array>
#include <forward_list>
#include <boost/iterator/transform_iterator.hpp>
#include "functional.hpp"
#include "productiter.hpp"
#include "typetraits.hpp"
#include "utility.hpp"

/// @file Models `Functor` / `FunctorApp` for standard containers.
///
/// # Algorithms
///
/// - `fmap` algorithm for containers parametrized by *one value type*
//    (`vector`, `list`, `set`, ...):
///   Create a new container by applying the mapped function to each value.
///
///   Specification (`{x...}` denotes a container with values `x...`):
///     `fmap(f, {x...}) = {f(x)...}`
///
///   In the `FunctorApp` case, several containers are passed:
///     `fmap(f, {x...}, {y...}, ...) = {f(x, y, ...)...}`
///
///   Note: The resulting container's size is the smallest of the input
///     containers' sizes. E.g.
///     `fmap(f,
///         {x0, x1, x2},
///         {y0, y1},
///         {z0, z1, z2, z3}) =
///       {f(x0, y0, z0), f(x1, y1, z1)}`
///
/// - `fmap` algorithm for containers parametrized by *one key type and one
///   value type* (`map`, `multimap`, `unordered_map`, ...):
///   Create a new container by copying keys and applying the mapped function to
///   each value only.
///
///   Specification (`{(k, x)...}` denotes a container with keys/value pairs
///     `(k, x)...`):
///     `fmap(f, {(k, x)...}) = {(k, f(x))...}`
///
///   I.e. the function is applied on values only (see justification in the
///   `Models` section below).
///
///   Note: At the moment, `FunctorApp` is not modeled by `key/value` containers,
///     due to a more difficult traversal and/or key reduction problems.
///     The specification (for `std::{unordered_,}map`) would be:
///     `fmap(f, {(k, x)...}, {(k, y)...}, ...) = {(k, f(x, y, ...))...}`
///
///     The resulting container's size would be the number of keys that are
///     common to all containers. E.g.
///     `fmap(f,
///         {(k, x0), (l, x1), (m, x2)},
///         {(k, y0), (l, y1)},
///         {(u, z0), (l, z1), (m, z2), (n, z3)}) =
///       {(l, f(x1, y1, z1))}
///
///     This behavior would be the same as containers parametrized by one value
///     type (`vector`, `list`, `set`, ...), if you consider that these
///     containers have integer keys.
///
/// # Models
///
/// Container            Models `Functor`    Models `FunctorApp`
/// -------------------- ------------------- -------------------
/// `array`                      x                   x
/// `vector`                     x                   x
/// `deque`                      x                   x
/// `forward_list`               x                   x
/// `list`                       x                   x
/// `set`                        x                   x
/// `multiset`                   x                   x
/// `unordered_set`              x                   x
/// `unordered_multiset`         x                   x
/// `map`                        x
/// `multimap`                   x
/// `unordered_map`              x
/// `unordered_multimap`         x
///
/// Note: `std::*map` containers are functors only when the key type is considered
///   bound. This is because a functor projects one type, not two. For instance,
///   `std::map<std::string, _>` is a functor (key type is bound to `std::string`,
///   value type remains free), not `std::map` by itself. Applying the "lifting" of
///   a function `int -> bool` on a `std::map<std::string, int>` will yield a
///   `std::map<std::string, bool>`.
///
/// Note: The reason `FunctorApp` is not modeled by `key/value` containers is that
///   there is no *a priori* way to reduce keys.

/// Defines the `ka::fmap` function that is part of the `Functor` and
/// `FunctorApp` concepts (see `concept.hpp` for the formal definition) for
/// standard containers.

namespace ka {

namespace fmap_ns {
  // Overloads are defined here and available for `fmap_fn_t`.

  template<typename C0, typename C1>
  void try_reserve(false_t /* HasMemberReserve */, C0&, C1 const&) {
  }

  template<typename C0, typename C1>
  void try_reserve(true_t /* HasMemberReserve */, C0& c0, C1 const& c1) {
    c0.reserve(c1.size());
  }

  template<typename F, typename T>
  auto call_or_apply(F&& f, T const& x) -> decltype(fwd<F>(f)(x)) {
    return fwd<F>(f)(x);
  }

  template<typename F, typename... T>
  auto call_or_apply(F&& f, std::tuple<T...> const& x)
      -> decltype(ka::apply(fwd<F>(f), x)) {
    return ka::apply(fwd<F>(f), x);
  }

  // Procedure<U (T...)> F
  template<typename F>
  struct call_or_apply_bound_t {
    F f;
  // Procedure<U (T...)>:
    template<typename... T>
    auto operator()(T&&... t) const -> decltype(call_or_apply(f, fwd<T>(t)...)) {
      return call_or_apply(f, fwd<T>(t)...);
    }
    template<typename... T>
    auto operator()(T&&... t) -> decltype(call_or_apply(f, fwd<T>(t)...)) {
      return call_or_apply(f, fwd<T>(t)...);
    }
  };

  // Procedure F
  template<typename F>
  auto call_or_apply_bound(F&& f) -> call_or_apply_bound_t<Decay<F>> {
    return {fwd<F>(f)};
  }

  // SequenceContainer<T> C, InputIterator<T> I
  // Precondition: readable_bounded_range(b, e)
  template<typename C, typename I>
  void insert_range(true_t /* HasPositionalInsertRange<C> */,
      C& c, I b, I e) {
    using std::end;
    c.insert(end(c), b, e);
  }

  // SequenceContainer<T> C, InputIterator<T> I, Function<U (T)> F
  // Precondition: readable_bounded_range(b, e)
  template<typename C, typename I>
  void insert_range(false_t /* HasPositionalInsertRange<C> */,
      C& c, I b, I e) {
    c.insert(b, e);
  }

  // SequenceContainer<T> C, InputIterator<T> I, Function<U (T)> F
  // Precondition: readable_bounded_range(b, e)
  template<typename C, typename I, typename F>
  void insert_apply(C& c, I b, I e, F&& f) {
    using boost::iterators::make_transform_iterator;
    auto g = call_or_apply_bound(fwd<F>(f));
    insert_range(
      HasPositionalInsertRange<C>{},
      c,
      make_transform_iterator(b, g),
      make_transform_iterator(e, g));
  }

  // InputIterator<T> I, Function<U (T)> F
  // Precondition: readable_bounded_range(b, e)
  template<typename T, std::size_t N, typename I, typename F>
  void insert_apply(std::array<T, N>& c, I b, I e, F f) {
    std::size_t i = 0;
    while (b != e) {
      c[i] = call_or_apply(f, *b);
      ++i;
      ++b;
    }
  }

  // InputIterator<T> I, Function<U (T)> F
  // Precondition: readable_bounded_range(b, e)
  template<typename T, typename A, typename I, typename F>
  void insert_apply(std::forward_list<T, A>& c, I b, I e, F&& f) {
    using boost::iterators::make_transform_iterator;
    auto g = call_or_apply_bound(fwd<F>(f));
    c.insert_after(c.before_begin(),
      make_transform_iterator(b, g), make_transform_iterator(e, g));
  }

  // `Functor` model for key/value-containers (`std::map`, etc.).
  template<typename F, typename C>
  auto fmap_container(true_t /* HasMappedType<C> */, F&& f, C const& c)
      -> Rebind<C, CodomainFor<F, typename C::mapped_type>> {
    using std::end;
    Rebind<C, CodomainFor<F, typename C::mapped_type>> res;
    for (auto const& x: c) {
      res.insert(end(res), {x.first, f(x.second)});
    }
    return res;
  }

  // `Functor` model for non key/value containers (`std::vector`, etc.).
  //
  // Function<U (T)> F, Container<T> C
  template<typename F, typename C>
  auto fmap_container(false_t /* HasMappedType<C> */, F&& f, C const& c)
      -> Rebind<C, CodomainFor<F, typename C::value_type>> {
    using std::begin;
    using std::end;
    Rebind<C, CodomainFor<F, typename C::value_type>> res;
    try_reserve(HasMemberReserve<C>{}, res, c);
    insert_apply(res, begin(c), end(c), fwd<F>(f));
    return res;
  }

  // `FunctorApp` model for non key/value containers (`std::vector`, etc.).
  //
  // Function<Z (T, U...)> F, Container<T> C, Container<U>... Z
  template<typename F, typename C, typename... Z> // `D` confuses vs2015 with type `test::D` apparently...
  auto fmap_container(false_t /* HasMappedType<C> */, F&& f, C const& c, Z const&... z)
      -> Rebind<C, CodomainFor<F, typename C::value_type, typename Z::value_type...>> {
    Rebind<C, CodomainFor<F, typename C::value_type, typename Z::value_type...>> res;
    try_reserve(HasMemberReserve<C>{}, res, c);
    insert_apply(res, product_begin(c, z...), product_end(c, z...), fwd<F>(f));
    return res;
  }

  // Type has no member fmap: assume it is a container type.
  // Function<B (A)> F, Container<A> C
  template<typename F, typename T, typename... U>
  auto fmap_dispatch(false_t /* HasMemberFmap<C> */, F&& f, T&& t, U&&... u)
      -> decltype(fmap_container(HasMappedType<Decay<T>>{}, fwd<F>(f), fwd<T>(t), fwd<U>(u)...)) {
    return fmap_container(HasMappedType<Decay<T>>{}, fwd<F>(f), fwd<T>(t), fwd<U>(u)...);
  }
} // namespace fmap_ns

} // namespace ka

#endif // KA_FUNCTORCONTAINER_HPP
