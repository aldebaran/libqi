#ifndef KA_PARSE_HPP
#define KA_PARSE_HPP
#pragma once

#include "macroregular.hpp"
#include "utility.hpp"
#include "typetraits.hpp"
#include "indexed.hpp"
#include "opt.hpp"
#include "functional.hpp"
#include "zero.hpp"
#include "functor.hpp"

#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/apply_visitor.hpp>

#include <vector>
#include <utility>
#include <initializer_list>

namespace ka {
namespace parse {
/// Semantics is provided through 'meaning' expressions, of the form
/// 'meaning(<C++ construct>) = <denotation>'. A denotation is an entity that is
/// the *interpretation* of a C++ construct.
///
/// See formal specification `spec:/sbre/framework/2020/d`.

/// The result of some parsing operation.
///
/// It is an optional parsed element (if parsing succeeded), paired with the
/// string that remains to parse. Two sets are therefore involved: the
/// "parsed set" and the "symbol set" (aka "character set").
///
/// meaning(std::iterator_traits<I>::value_type) = C
///   C is the 'character' or 'symbol' set, whom strings are parsed.
///
/// meaning(pair<res_t<A, I>, I>) = R(A, C)
///   `res_t` contains only the iterator until the parser succeeded to parse.
///   Therefore, the *string* C^* that remains to parse is represented by the
///   bounded range whose `res_t`'s iterator is the beginning and the end
///   iterator is known from the context.
///
/// ForwardIterator I
template<typename A, typename I>
struct res_t {
  using value_type = A;
  using iterator_type = I;

  opt_t<A> value;
  I iter;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_2(res_t, value, iter)

// Readable:
  auto operator*() const& noexcept -> decltype(*value) {
    return *value;
  }

// Mutable:
  auto operator*() & noexcept -> decltype(*value) {
    return *value;
  }
  auto operator*() && noexcept -> decltype(*mv(value)) {
    return *mv(value);
  }

// EmptyMutable:
  constexpr
  auto empty() const noexcept -> bool {
    return value.empty();
  }

  template<typename F, typename V>
  using ValueOfResultOfFmap = typename Decay<CodomainFor<fmap_fn_t, F, V>>::value_type;

// Functor:
  /// Function<U (A)> F
  template<typename F>
  auto fmap(F&& f) const&
    -> res_t<ValueOfResultOfFmap<F, const opt_t<A>&>, I> {
    return { ka::fmap(fwd<F>(f), value), iter };
  }

  /// Function<U (A)> F
  template<typename F>
  auto fmap(F&& f) &&
    -> res_t<ValueOfResultOfFmap<F, opt_t<A>&&>, I> {
    return { ka::fmap(fwd<F>(f), mv(value)), mv(iter) };
  }
};

/// Convenience function to construct a result in success, deducing argument types.
///
/// meaning(ok(a, i)) = (a, s), where s = meaning(pair{i, end})
struct ok_fn_t {
// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_0(ok_fn_t)
// Function:
  /// ForwardIterator I
  template<typename A, typename I> constexpr
  auto operator()(A&& a, I&& i) const noexcept -> res_t<Decay<A>, Decay<I>> {
    return {ka::opt(fwd<A>(a)), fwd<I>(i)};
  }
};

static constexpr auto const& ok = static_const_t<ok_fn_t>::value;

/// Convenience function to construct a result in error, deducing argument types.
///
/// meaning(err(t, i)) = (none, s), where s = meaning(pair{i, end})
///
/// ForwardIterator I
struct err_fn_t {
// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_0(err_fn_t)
// Function:
  template<typename A, typename I> constexpr
  auto operator()(type_t<A>, I i) const noexcept -> res_t<A, I> {
    return {opt_t<A>{}, mv(i)};
  }
};

static constexpr auto const& err = static_const_t<err_fn_t>::value;

/// Convenience function to construct a result, deducing argument types.
///
/// meaning(res(a, i)) = (a, s), where s = meaning(pair{i, end})
///
/// ForwardIterator I
template<typename A, typename I> constexpr
auto res(opt_t<A> opt_val, I i) noexcept -> res_t<A, I> {
  return {mv(opt_val), mv(i)};
}

/// The result iterator up to which the parsing was successful.
///
/// meaning(iter(res(a, i))) = s, where s = meaning(pair{i, end})
///
/// ForwardIterator I
template<typename A, typename I> constexpr
auto iter(res_t<A, I> const& r) noexcept -> I const& {
  return r.iter;
}

/// Overload for non-const references.
template<typename A, typename I> constexpr
auto iter(res_t<A, I>& r) noexcept -> I& {
  return r.iter;
}

/// Overload for rvalue-references.
template<typename A, typename I> constexpr
auto iter(res_t<A, I>&& r) noexcept -> I {
  return mv(r.iter);
}

/// The optional value of the result.
///
/// invariant: ka::empty(r) == ka::empty(to_opt(r))
///         && ka::src(r) == ka::src(to_opt(r))
///
/// meaning(to_opt)(a, s) = a
///
/// ForwardIterator I
template<typename A, typename I> constexpr
auto to_opt(res_t<A, I> const& r) noexcept -> ka::opt_t<A> const& {
  return r.value;
}

/// Overload for non-const references.
template<typename A, typename I> constexpr
auto to_opt(res_t<A, I>& r) noexcept -> ka::opt_t<A>& {
  return r.value;
}

/// Overload for rvalue-references.
template<typename A, typename I> constexpr
auto to_opt(res_t<A, I>&& r) noexcept -> ka::opt_t<A> {
  return mv(r.value);
}

/// Derives the implementation of the functor part of a parser type.
#define KA_PARSER_DERIVE_FUNCTOR()                                 \
  /* Procedure<B (A)> PDFF */                                      \
  template<typename PDFF> constexpr                                \
  auto fmap(PDFF&& f) const & noexcept                             \
    -> ka::parse::fmapped_t<Decay<PDFF>, Decay<decltype(*this)>> { \
    return { fwd<PDFF>(f), *this };                                \
  }                                                                \
  template<typename PDFF>                                          \
  auto fmap(PDFF&& f) && noexcept                                  \
    -> ka::parse::fmapped_t<Decay<PDFF>, Decay<decltype(*this)>> { \
    return { fwd<PDFF>(f), std::move(*this) };                     \
  }

/// meaning(ResultOf<PA, I>) = R(A, C^*), where C = meaning(I::value_type)
///
/// Parser<A> PA
/// ForwardIterator I
template<typename PA, typename I>
using ResultOf = Decay<CodomainFor<PA, I, I>>;

/// meaning(ValueOf<PA, I>) = A
///
/// Parser<A> PA
/// ForwardIterator I
template<typename PA, typename I>
using ValueOf = typename ResultOf<PA, I>::value_type;

/// Mapping of a procedure through the parser functor.
///
/// I.e. new parser whose result is obtained by mapping the function to the
/// original parser's result.
///
/// meaning(fmap) = P: B^A → (PB)^(PA) // function-part of the functor
///
/// Procedure<B (A)> F
/// Parser PA
template<typename F, typename PA>
struct fmapped_t {
  F f;
  PA pa;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_2(fmapped_t, f, pa)

// Functor (up to isomorphism):
  KA_PARSER_DERIVE_FUNCTOR()

// Parser<A>:
  /// @pre readable_bounded_range(b, e)
  template<typename I> constexpr
  auto operator()(I b, I e) const -> decltype(pa(b, e).fmap(f)) {
    // Workaround MSVC:
    // Using `ka::fmap()` does not instantiate the right `fmap_dispatch` function.
    // This seems to be a compiler bug.
    return pa(b, e).fmap(f);
  }
};

/// Parser of a single element.
///
/// meaning(symbol_t<I>) = PC, where C = meaning(I::value_type)
/// (C is the 'character' or 'symbol' set)
struct symbol_t {
  template<typename I>
  using Value = typename std::iterator_traits<I>::value_type;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_0(symbol_t)

// Functor (up to isomorphism):
  KA_PARSER_DERIVE_FUNCTOR()

// Parser<C>:
  /// @pre readable_bounded_range(b, e)
  template<typename I> constexpr
  auto operator()(I b, I e) const -> res_t<Value<I>, I> {
    return (b == e) ? err(type_t<Value<I>>{}, b) : ok(*b, std::next(b));
  }
};

static constexpr auto const& symbol = static_const_t<symbol_t>::value;

/// Combines multiple parsers into one, which returns the product of all results.
/// Succeeds if all parsers succeed.
///
/// meaning(product_t) = ⊗
///
/// Parser<A>... PA
template<typename... PA>
struct product_t {
  ka::product_t<PA...> pa;

  template<typename I>
  using Value = ka::product_t<ValueOf<PA, I>...>;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_1(product_t, pa)

// Functor (up to isomorphism):
  KA_PARSER_DERIVE_FUNCTOR()

// Parser<ka::product_t<A...>>:
  /// @pre readable_bounded_range(b, e)
  template<typename I>
  auto operator()(I b, I e) const -> res_t<Value<I>, I> {
    return impl(index_sequence_for<PA...>{}, b, e);
  }

private:
  template<std::size_t Idx, typename I>
  auto do_one_at(ka::product_t<opt_t<ResultOf<PA, I>>...>& res, I b, I e) const
    -> std::pair<bool, I> {
    auto& opt_res_at = std::get<Idx>(res);
    opt_res_at.emplace_front(std::get<Idx>(pa)(b, e));
    auto& res_at = opt_res_at.front();
    return { !res_at.empty(), iter(res_at) };
  }

  template<std::size_t... Idx, typename I>
  auto impl(index_sequence<Idx...>, I b, I e) const -> res_t<Value<I>, I> {
    auto success = true;
    auto it = b;
    auto opt_res = ka::product_t<opt_t<ResultOf<PA, I>>...>{};
    // The following use of `std::initializer_list` is a C++11 hack to
    // repeatedly apply a function on arguments of heterogeneous types, avoiding
    // function recursion.
    // TODO: Factorize in a named function, and replace in C++17 or above.

    // Ignore unused variables (happens with product_t<>).
    (void) e; (void) opt_res;
    (void) std::initializer_list<bool>{ (
      success = success && (
          std::tie(success, it) = do_one_at<Idx>(opt_res, it, e),
          success
        )
      )... };

    if (!success) {
      return err(type_t<Value<I>>{}, b);
    }

    auto values = ka::product(mv(**std::get<Idx>(opt_res))...);
    return ok(mv(values), it);
  }
};

/// Constructs a product of parsers, performing argument type deduction.
///
/// Parser<A>... PA
struct product_fn_t {
  template<typename... PA> constexpr
  auto operator()(PA&&... pa) const noexcept -> product_t<Decay<PA>...> {
    return { ka::product(fwd<PA>(pa)...) };
  }
};
static constexpr auto const& product = static_const_t<product_fn_t>::value;

/// Parser that always succeeds, returning unit and the string as-is.
///
/// meaning(unit_t) = 1
using unit_t = product_t<>;
static constexpr auto const& unit = static_const_t<unit_t>::value;

namespace detail {
  // `product_t` is `parse::product_t`, by opposition to `ka::product_t` (tuple
  // construction).
  template<typename PA> constexpr
  auto unwrap_product(PA&& pa) noexcept -> ka::product_t<PA&&> {
    return ka::product_t<PA&&>{ fwd<PA>(pa) };
  }

  template<typename... PA> constexpr
  auto unwrap_product(product_t<PA...> const& prod) noexcept ->
  ka::product_t<PA...> const& {
    return prod.pa;
  }

  template<typename... PA> constexpr
  auto unwrap_product(product_t<PA...>&& prod) noexcept -> ka::product_t<PA...>&& {
    return mv(prod.pa);
  }

  constexpr
  auto unwrap_product(product_t<>) noexcept -> ka::product_t<> {
    return {};
  }
} // namespace detail

/// Constructs a product of parsers, flattening any product of product in the
/// result and performing argument type deduction.
///
/// Parser<A>... PA
template<typename... PA> constexpr
auto flat_product(PA&&... pa) noexcept
  -> decltype(ka::apply(product, std::tuple_cat(detail::unwrap_product(fwd<PA>(pa))...))) {
  return ka::apply(product, std::tuple_cat(detail::unwrap_product(fwd<PA>(pa))...));
}

/// Convenience type for the value type of the result of a sum of parsers.
template<typename... A>
using SumValue = ApplyIndexed<boost::variant, A...>;

/// Combines multiple parsers into one, which returns the result of the first
/// parser that succeeds, in the order of declaration. Succeeds if any parser
/// succeeds.
///
/// meaning(sum_t) = ⊕
///
/// Parser<A>... PA
template<typename... PA>
struct sum_t {
  ka::product_t<PA...> pa;

  template<typename I>
  using Value = SumValue<ValueOf<PA, I>...>;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_1(sum_t, pa)

// Functor (up to isomorphism):
  KA_PARSER_DERIVE_FUNCTOR()

// Parser<SumValue<A>>:
  /// @pre readable_bounded_range(b, e)
  template<typename I>
  auto operator()(I b, I e) const -> res_t<Value<I>, I> {
    return impl(index_sequence_for<PA...>{}, b, e);
  }

private:
  template<std::size_t Idx, typename I>
  auto do_one_at(opt_t<Value<I>>& opt_val, I b, I e) const -> std::pair<bool, I> {
    auto res = std::get<Idx>(pa)(b, e);
    auto const ok = !res.empty();
    if (ok) {
      opt_val.emplace_front(indexed<Idx>(*mv(res)));
    }
    // Ok to use `res` here even after move, since we're not accessing the moved
    // part.
    return { ok, iter(res) };
  }

  template<std::size_t... Idx, typename I>
  auto impl(index_sequence<Idx...>, I b, I e) const -> res_t<Value<I>, I> {
    auto success = false;
    auto it = b;
    auto opt_val = opt_t<Value<I>>{};

    // The following use of `std::initializer_list` is a C++11 hack to
    // repeatedly apply a function on arguments of heterogeneous types, avoiding
    // function recursion.
    // TODO: Factorize in a named function, and replace in C++17 or above.
    (void) std::initializer_list<bool>{(
      success = success || (
          std::tie(success, it) = do_one_at<Idx>(opt_val, it, e),
          success
        )
      )...};

    if (!success) {
      return err(type_t<Value<I>>{}, b);
    }
    assert(!opt_val.empty());
    return ok(*mv(opt_val), it);
  }
};

/// Parser that always fails to return an element of the empty set 0 (since
/// there's none).
///
/// meaning(sum_t<>) = 0
// TODO: Remove this specialization when having a `ka::sum_t` type that can be empty.
template<>
struct sum_t<> {
  using Value = ka::zero_t;
  ka::product_t<> pa;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_0(sum_t)

// Functor (up to isomorphism):
  KA_PARSER_DERIVE_FUNCTOR()

// Parser<>:
  /// @pre readable_bounded_range(b, e)
  template<typename I> constexpr
  auto operator()(I b, I) const noexcept -> res_t<Value, I> {
    return err(type_t<Value>{}, b);
  }
};
using zero_t = sum_t<>;
static constexpr auto const& zero = static_const_t<zero_t>::value;

/// Constructs a sum of parsers, performing argument type deduction.
///
/// Parser<A>... PA
struct sum_fn_t {
  template<typename... PA> constexpr
  auto operator()(PA&&... pa) const noexcept -> sum_t<Decay<PA>...> {
    return { ka::product(fwd<PA>(pa)...) };
  }
};
static constexpr auto const& sum = static_const_t<sum_fn_t>::value;

namespace detail {
  template<typename PA> constexpr
  auto unwrap_sum(PA&& pa) noexcept -> ka::product_t<PA&&> {
    return ka::product_t<PA&&>{ fwd<PA>(pa) };
  }

  template<typename... PA> constexpr
  auto unwrap_sum(sum_t<PA...> const& s) noexcept -> ka::product_t<PA...> const& {
    return s.pa;
  }

  template<typename... PA> constexpr
  auto unwrap_sum(sum_t<PA...>&& s) noexcept -> ka::product_t<PA...>&& {
    return mv(s.pa);
  }
} // namespace detail

/// Constructs a sum of parsers, flattening any sum of sum in the result and
/// performing argument type deduction.
///
/// Parser<A>... PA
template<typename... PA> constexpr
auto flat_sum(PA&&... pa) noexcept
  -> decltype(ka::apply(sum, std::tuple_cat(detail::unwrap_sum(fwd<PA>(pa))...))) {
  return ka::apply(sum, std::tuple_cat(detail::unwrap_sum(fwd<PA>(pa))...));
}

namespace detail {
  /// Visitor for `boost::variant` that contains either `indexed_t<T>` or
  /// indexed_t<ka::unit_t>` and converts it into an `opt_t<T>`.
  struct to_opt_t {
    template<typename V, typename... A>
    struct visitor_t : boost::static_visitor<opt_t<V>> {
      template<std::size_t I, typename U>
      auto operator()(indexed_t<I, U> const& u) const -> ka::opt_t<V> {
        return ka::opt(V(src(u)));
      }
      template<std::size_t I>
      auto operator()(indexed_t<I, ka::unit_t>) const -> ka::opt_t<V> {
        return {};
      }
    };

    template<typename A> constexpr
    auto operator()(boost::variant<indexed_t<0, A>, indexed_t<1, ka::unit_t>> const& v) const
      -> opt_t<A> {
      return boost::apply_visitor(visitor_t<A>{}, v);
    }
  };
} // namespace detail

/// Convenience function to construct a parser of optional that always succeeds,
/// producing an optional value.
///
/// meaning(opt(pa)) = pa ⊕ 1
///
/// Parser<A> PA
template<typename PA> constexpr
auto opt(PA&& pa) noexcept
  -> decltype(sum(pa, unit).fmap(detail::to_opt_t{})) {
  return sum(pa, unit).fmap(detail::to_opt_t{});
}

/// Parser that applies another one between min and max times (included). Its
/// result is a list of `A` containing between min and max (included) elements.
///
/// meaning(quantify_t) = quantify
///
/// Parser<A> PA
template<typename PA>
struct quantify_t {
  PA pa;
  std::size_t min;
  ka::opt_t<std::size_t> max; // Precondition: max.empty() || min <= src(max)

  template<typename I>
  using Value = std::vector<ValueOf<PA, I>>;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_3(quantify_t, pa, min, max)

// Functor (up to isomorphism):
  KA_PARSER_DERIVE_FUNCTOR()

// Parser<std::vector<A>>:
  /// @pre readable_bounded_range(b, e)
  template<typename I>
  auto operator()(I b, I e) const -> res_t<Value<I>, I> {
    auto b0 = b;
    auto values = Value<I>{};
    if (max.empty()) {
      while (parse_one(pa, b, e, values)); // unbounded
    } else {
      auto const m = src(max);
      for (std::size_t i = 0; i != m && parse_one(pa, b, e, values); ++i);
    }
    return min <= values.size()
      ? ok(mv(values), mv(b))
      : err(type_t<Value<I>>{}, mv(b0));
  }
private:
  template<typename I, typename V> static
  auto parse_one(PA const& pa, I& b, I const& e, V& values) -> bool {
    auto res = pa(b, e);
    if (res.empty()) return false;
    b = iter(res);
    values.push_back(*mv(res));
    return true;
  }
};

/// Constructs a quantified parser, performing argument type deduction.
/// Omitting or passing an empty maximum means there is no upper bound.
struct quantify_fn_t {
// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_0(quantify_fn_t)
// Function:
  /// Precondition: max.empty() || min <= src(max)
  /// Parser<A> PA
  template<typename PA>
  auto operator()(PA&& pa, std::size_t min, ka::opt_t<std::size_t> max) const noexcept -> quantify_t<Decay<PA>> {
    return {fwd<PA>(pa), min, mv(max)};
  }
  /// Precondition: min <= max
  /// Parser<A> PA
  template<typename PA>
  auto operator()(PA&& pa, std::size_t min, std::size_t max) const noexcept -> quantify_t<Decay<PA>> {
    return {fwd<PA>(pa), min, ka::opt(max)};
  }
  /// Parser<A> PA
  template<typename PA>
  auto operator()(PA&& pa, std::size_t min) const noexcept -> quantify_t<Decay<PA>> {
    return {fwd<PA>(pa), min, ka::opt_t<std::size_t>{}};
  }
};

static constexpr auto const& quantify = static_const_t<quantify_fn_t>::value;

/// Parser that applies another one as much as possible and always succeeds. Its
/// result is a list of `A`.
///
/// meaning(list) = ^* = quantify(_, 0, ∞)
///
/// Parser<A> PA
template<typename PA> constexpr
auto list(PA&& pa) noexcept -> quantify_t<Decay<PA>> {
  return quantify(fwd<PA>(pa), 0);
}

namespace detail {
  template<typename PA> constexpr
  auto unwrap_quantify(PA&& pa) noexcept -> PA&& {
    return fwd<PA>(pa);
  }

  template<typename PA> constexpr
  auto unwrap_quantify(quantify_t<PA> const& l) noexcept -> PA const& {
    return l.pa;
  }

  template<typename PA> constexpr
  auto unwrap_quantify(quantify_t<PA>&& l) noexcept -> PA&& {
    return mv(l.pa);
  }
} // namespace detail

/// Constructs a list of a parser, flattening the result if the argument is
/// already a list and performing argument type deduction.
///
/// Parser<A> PA
template<typename PA> constexpr
auto flat_list(PA&& pa) noexcept
  -> decltype(list(detail::unwrap_quantify(fwd<PA>(pa)))) {
  return list(detail::unwrap_quantify(fwd<PA>(pa)));
}

// TODO: Complete list monoid with concatenation and empty list.
// TODO: Add literal, filter and bounded quantification parsers (see
//  `spec:/sbre/framework/2020/d`)

namespace ops {
  /// Operator for flat product of parsers.
  ///
  /// Parser<A> PA
  /// Parser<B> PB
  template<typename PA, typename PB> constexpr
  auto operator*(PA&& pa, PB&& pb) noexcept
    -> decltype(flat_product(fwd<PA>(pa), fwd<PB>(pb))) {
    return flat_product(fwd<PA>(pa), fwd<PB>(pb));
  }

  /// Operator for flat sum of parsers.
  ///
  /// Parser<A> PA
  /// Parser<B> PB
  template<typename PA, typename PB> constexpr
  auto operator+(PA&& pa, PB&& pb) noexcept
    -> decltype(flat_sum(fwd<PA>(pa), fwd<PB>(pb))) {
    return flat_sum(fwd<PA>(pa), fwd<PB>(pb));
  }

  /// Operator for flat list (repetition) of parsers.
  ///
  /// Parser<A> PA
  /// Parser<B> PB
  template<typename PA> constexpr
  auto operator*(PA&& pa) noexcept
    -> decltype(flat_list(fwd<PA>(pa))) {
    return flat_list(fwd<PA>(pa));
  }
} // namespace ops

} // namespace parse
} // namespace ka

#endif // KA_PARSE_HPP
