#ifndef KA_URI_PARSING_HPP
#define KA_URI_PARSING_HPP
#pragma once

#include "uri_fwd.hpp"
#include "io.hpp"
#include "../macroregular.hpp"
#include "../utility.hpp"
#include "../typetraits.hpp"
#include "../indexed.hpp"
#include "../opt.hpp"
#include "../functional.hpp"
#include "../flatten.hpp"
#include "../range.hpp"
#include "../functorcontainer.hpp"
#include "../parse.hpp"
#include "../functor.hpp"

#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/preprocessor/comma.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/cxx14/equal.hpp>

#include <algorithm>
#include <vector>
#include <utility>
#include <initializer_list>
#include <sstream>
#include <numeric>

/// @file Semantics is provided through 'meaning' expressions, of the form
/// 'meaning(<C++ construct>) = <denotation>'. A denotation is an entity that is the
/// *interpretation* of a C++ construct.
///
/// URI are normalized during parsing according to the RFC 3986 "Syntax-Based Normalization"
/// (section 6.2.2).
///
/// See formal specification `spec:/sbre/framework/2020/c` for definitions of denotations.
///                          `spec:/sbre/framework/2020/d` for definitions of parsers.

namespace ka {
namespace detail_uri {
namespace parsing {

/// Parser that transforms a parser so that it succeeds only when a predicate
/// holds.
///
/// meaning(filter_t) = filter
///
/// TODO: Move to `ka/parse.hpp` and add appropriate tests.
///
/// Parser<A> PA
/// Predicate<A> Pred
template<typename PA, typename Pred>
struct filter_t {
  PA pa;
  Pred pred;

  template<typename I>
  using Value = parse::ValueOf<PA, I>;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_2(filter_t, pa, pred)

// Functor (up to isomorphism):
  KA_PARSER_DERIVE_FUNCTOR()

private:
  template<typename Self, typename I>
  static auto impl(Self&& self, I b, I e)
    -> parse::res_t<Value<I>, I> {
    // The several `fwd<Self>(self)` are ok, since orthogonal parts of the
    // objects may be moved.
    auto res = fwd<Self>(self).pa(b, e);
    if (!res.empty() && fwd<Self>(self).pred(*res)) {
      auto& it = iter(res);
      return parse::ok(*mv(res), mv(it));
    }
    return parse::err(type_t<Value<I>>{}, mv(b));
  }

public:
// Parser<A>:
  /// @pre readable_bounded_range(b, e)
  template<typename I>
  auto operator()(I b, I e) -> decltype(this->impl(*this, mv(b), mv(e))) {
    return this->impl(*this, mv(b), mv(e));
  }

  /// @pre readable_bounded_range(b, e)
  template<typename I>
  auto operator()(I b, I e) const -> decltype(this->impl(*this, mv(b), mv(e))) {
    return this->impl(*this, mv(b), mv(e));
  }
};

/// Constructs a conditional parser, performing argument type deduction.
///
/// Predicate<A> Pred
/// Parser<A> PA
template<typename Pred, typename PA> constexpr
auto filter(Pred&& pred, PA&& pa) noexcept -> filter_t<Decay<PA>, Decay<Pred>> {
  return { fwd<PA>(pa), fwd<Pred>(pred) };
}

/// Predicate that compares the argument with a constant.
///
/// meaning(equal_to_constant_t<A>{a}(b)) = (a = b)
///
/// Note: This can be replaced by `ka::equal` partially applied to one
///   of its two parameters.
template<typename A>
struct equal_to_constant_t {
  A val;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_1(equal_to_constant_t, val)

// Predicate:
  template<typename B> constexpr
  auto operator()(B const& b) const noexcept -> bool {
    return val == b;
  }
};

template<typename A> constexpr
auto equal_to_constant(A&& a) noexcept -> equal_to_constant_t<Decay<A>> {
  return { fwd<A>(a) };
}

template<std::size_t N>
using size_constant_t = std::integral_constant<std::size_t, N>;

template<typename T, std::size_t N>
using CArray = T[N];

/// Constructs a parser that succeeds if the parsed value is equal to a constant value.
///
/// meaning(literal(val, pa)) = filter(pa, λa. a = val)
///
/// Parser<A> PA
template<typename A, typename PA = parse::symbol_t> constexpr
auto literal(A&& val, PA&& pa = {}) noexcept -> filter_t<Decay<PA>, equal_to_constant_t<Decay<A>>> {
  return { fwd<PA>(pa), { fwd<A>(val) } };
}

namespace detail {
  template<typename T, std::size_t N, std::size_t... I, typename PA> constexpr
  auto to_product_literal_impl(CArray<T, N> const& arr, index_sequence<I...>, PA pa) noexcept
    -> decltype(parse::flat_product(literal(arr[I], pa)...)) {
    return parse::flat_product(literal(arr[I], pa)...);
  }

  template<std::size_t N, typename T, typename PA> constexpr
  auto to_product_constant(CArray<T, N> const& arr, PA&& pa) noexcept
    -> decltype(to_product_literal_impl(arr, make_index_sequence<N>{}, fwd<PA>(pa))) {
    return to_product_literal_impl(arr, make_index_sequence<N>{}, fwd<PA>(pa));
  }

  template<std::size_t N, typename PA> constexpr
  auto to_product_constant(CArray<char const, N> const& arr, PA&& pa) noexcept
    // Remove the null character at the end.
    -> decltype(to_product_literal_impl(arr, make_index_sequence<N - 1>{}, fwd<PA>(pa))) {
    return to_product_literal_impl(arr, make_index_sequence<N - 1>{}, fwd<PA>(pa));
  }
} // namespace detail

/// Constructs a parser that succeeds if the parsed value is a product of a list of values.
///
/// meaning(literals(s, pa)) = meaning(literal(s[0], pa)) ⊗ ...
///
/// @note If the argument is an array of chars, the last character is expected to be a null
///       character and therefore will be discarded.
///
/// Parser<A> PA
template<std::size_t N, typename T, typename PA = parse::symbol_t> constexpr
auto literals(CArray<T, N> const& arr, PA&& pa = {}) noexcept
  -> decltype(detail::to_product_constant(arr, fwd<PA>(pa))) {
  return detail::to_product_constant(arr, fwd<PA>(pa));
}

// TODO: Move predicate types and related functions to functional.hpp.

/// Predicate that combines multiple predicates with a logical `and` operation.
/// Holds if all the predicates hold.
///
/// meaning(predicate_and_fn_t<Preds...>{p...}(a...)) = p(a...) ∧ ...
///
/// Predicate<T...>... Pred
template<typename... Pred>
struct predicate_and_t {
  ka::product_t<Pred...> preds;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_1(predicate_and_t, pred)

// Predicate<T...>:
  template<typename... T> constexpr
  auto operator()(T&&... t) const -> bool {
    return impl(preds, index_sequence_for<Pred...>{}, true, fwd<T>(t)...);
  }

  template<typename... T>
  auto operator()(T&&... t) -> bool {
    return impl(preds, index_sequence_for<Pred...>{}, true, fwd<T>(t)...);
  }

private:
  template<typename Preds, typename... T, std::size_t... I> constexpr
  static auto impl(Preds& preds, index_sequence<I...>, bool res, T&&... t) -> bool {
    return std::initializer_list<bool>{ (res = res && std::get<I>(preds)(t...), res)... }, res;
  }
};

/// Convenience function that constructs the predicate, deducting argument types.
struct predicate_and_fn_t {
// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_0(predicate_and_fn_t)
// Function<Predicate (Predicate...)>:
  /// Predicate<T...> Pred
  template<typename... Pred> constexpr
  auto operator()(Pred&&... pred) const noexcept -> predicate_and_t<Decay<Pred>...> {
    return { ka::product(fwd<Pred>(pred)...) };
  }
};
static constexpr auto& predicate_and = static_const_t<predicate_and_fn_t>::value;

/// Predicate that combines multiple predicates with a logical `or` operation.
/// Holds if any of the predicates holds.
///
/// meaning(predicate_or_fn_t<Preds...>{p...}(a...)) = p(a...) ∨ ...
///
/// Predicate<T...> Pred
template<typename... Pred>
struct predicate_or_t {
  ka::product_t<Pred...> preds;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_1(predicate_or_t, pred)

// Predicate<T...>:
  template<typename... T> constexpr
  auto operator()(T&&... t) const -> bool {
    return impl(preds, index_sequence_for<Pred...>{}, false, fwd<T>(t)...);
  }

  template<typename... T>
  auto operator()(T&&... t) -> bool {
    return impl(preds, index_sequence_for<Pred...>{}, false, fwd<T>(t)...);
  }

private:
  template<typename Preds, typename... T, std::size_t... Idx> constexpr
  static auto impl(Preds& preds, index_sequence<Idx...>, bool res, T&&... t) -> bool {
    return (void) std::initializer_list<bool>{ (res = res || std::get<Idx>(preds)(t...), res)... }
           , res;
  }
};

/// Convenience function that constructs the predicate, deducting argument types.
struct predicate_or_fn_t {
// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_0(predicate_or_fn_t)
// Function<Predicate (Predicate...)>:
  /// Predicate<T...> Pred
  template<typename... Pred> constexpr
  auto operator()(Pred&&... pred) const noexcept -> predicate_or_t<Decay<Pred>...> {
    return { ka::product(fwd<Pred>(pred)...) };
  }
};
static constexpr auto& predicate_or = static_const_t<predicate_or_fn_t>::value;

/// Constructs a parser that succeeds if the parsed value is equal to any of the arguments.
///
/// meaning(any_of(c...)) = filter((λx. x = c ∨ ...), symbol)
template<typename... T> constexpr
auto any_of(T&&... c) noexcept
  -> filter_t<parse::symbol_t, predicate_or_t<equal_to_constant_t<Decay<T>>...>> {
  return { {}, predicate_or(equal_to_constant(fwd<T>(c))...) };
}

namespace detail {
  struct container_size_t {
    template<typename C> constexpr
    auto operator()(C const& c) const -> std::size_t {
      return c.size();
    }
  };

  template<typename T>
  struct less_or_eq_t {
    T ref;
    constexpr
    auto operator()(T const& t) const -> bool {
      return t <= ref;
    }
  };
  template<typename T> constexpr
  auto less_or_eq(T&& t) noexcept -> less_or_eq_t<Decay<T>> { return { fwd<T>(t) }; }

  template<typename T>
  struct greater_or_eq_t {
    T ref;
    constexpr
    auto operator()(T const& t) const -> bool {
      return t >= ref;
    }
  };
  template<typename T> constexpr
  auto greater_or_eq(T&& t) noexcept -> greater_or_eq_t<Decay<T>> { return { fwd<T>(t) }; }
} // namespace detail

/// Constructs a parser that parses at least n elements. There is no upper
/// bound.
///
/// meaning(at_least(n, pa)) = quantify(n, ∞, pa)
///
/// Parser<A> PA
template<typename PA = parse::symbol_t> constexpr
auto at_least(std::size_t n, PA&& pa = {}) noexcept
  -> decltype(parse::quantify(fwd<PA>(pa), n)) {
  return parse::quantify(fwd<PA>(pa), n);
}

/// Constructs a parser that parses from 0 to n elements included. Always
/// succeeds.
///
/// meaning(at_most(n, pa)) = quantify(0, n + 1, pa)
///
/// Parser<A> PA
template<typename PA = parse::symbol_t> constexpr
auto at_most(std::size_t n, PA&& pa = {}) noexcept
  -> decltype(parse::quantify(fwd<PA>(pa), 0, n)) {
  return parse::quantify(fwd<PA>(pa), 0, n);
}

/// Constructs a parser that parses from min to max included elements.
///
/// meaning(between(min, max, pa)) = quantify(min, max + 1, pa)
///
/// Parser<A> PA
template<typename PA = parse::symbol_t> constexpr
auto between(std::size_t min, std::size_t max, PA&& pa = {}) noexcept
  -> decltype(parse::quantify(fwd<PA>(pa), min, max)) {
  return parse::quantify(fwd<PA>(pa), min, max);
}

namespace detail {
  template<typename PA, std::size_t... I> constexpr
  auto power_impl(PA&& pa, index_sequence<I...>) noexcept
    -> decltype(parse::flat_product(((void)I, pa)...)) {
    return parse::flat_product(((void)I, pa)...);
  }
} // namespace detail

/// Constructs a parser that succeeds if the parser argument succeeds at
/// parsing exactly `N` times.
///
/// meaning(power(p, n)) = p ⊗ p ⊗ ... (n occurrences of p)
///
/// TODO: Add to `ka/parse.hpp` with appropriate tests.
///
/// Parser<A> PA
template<typename PA, std::size_t N>
auto power(PA&& pa, size_constant_t<N>) noexcept
  -> decltype(detail::power_impl(fwd<PA>(pa), make_index_sequence<N>{})) {
  return detail::power_impl(fwd<PA>(pa), make_index_sequence<N>{});
}


namespace pred_ops {
  /// Constructs a predicate that is the `and` combination of two predicates.
  ///
  /// Predicate<T> Pred0, Pred1
  template<typename Pred0, typename Pred1> constexpr
  auto operator&&(Pred0&& pred0, Pred1&& pred1) noexcept
    -> decltype(predicate_and(fwd<Pred0>(pred0), fwd<Pred1>(pred1))) {
    return predicate_and(fwd<Pred0>(pred0), fwd<Pred1>(pred1));
  }

  /// Constructs a predicate that is the `or` combination of two predicates.
  ///
  /// Predicate<T> Pred0, Pred1
  template<typename Pred0, typename Pred1> constexpr
  auto operator||(Pred0&& pred0, Pred1&& pred1) noexcept
    -> decltype(predicate_or(fwd<Pred0>(pred0), fwd<Pred1>(pred1))) {
    return predicate_or(fwd<Pred0>(pred0), fwd<Pred1>(pred1));
  }
}

/// Predicate that holds when a character is within a specific character class.
struct is_char_class_t {
  std::ctype_base::mask mask;

// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_1(is_char_class_t, mask)

// Predicate:
  auto operator()(char c) const noexcept -> bool {
    std::locale const loc;
    auto& facet = std::use_facet<std::ctype<char>>(loc);
    return facet.is(mask, c);
  }
};

namespace detail {
  template<typename F, typename... T, std::size_t... I> constexpr
  auto transform_impl(F&& f, ka::product_t<T...> const& t, index_sequence<I...>) noexcept
    -> decltype(ka::product(f(std::get<I>(t))...)) {
    return ka::product(f(std::get<I>(t))...);
  }

  template<typename R, typename F, typename... T> constexpr
  auto fold_impl(R val, F&&, ka::product_t<T...> const&, index_sequence<>) -> R {
    return val;
  }

  template<typename R, typename F, typename... T, std::size_t I0, std::size_t... I> constexpr
  auto fold_impl(R val, F&& op, ka::product_t<T...> const& t, index_sequence<I0, I...>) -> R {
    return fold_impl(fwd<F>(op)(val, std::get<I0>(t)), op, t, index_sequence<I...>{});
  }
} // namespace detail

/// Applies a function on each element of the product and returns a product of the results.
///
/// Note: This is equivalent to modeling Functor for homogeneous products.
///
/// Procedure<U (T)> F
template<typename F, typename... T> constexpr
auto transform(F&& f, ka::product_t<T...> const& t) noexcept
  -> decltype(detail::transform_impl(fwd<F>(f), t, index_sequence_for<T...>{})) {
  return detail::transform_impl(fwd<F>(f), t, index_sequence_for<T...>{});
}

/// Folds elements of the product.
///
/// Procedure<R (R, T)> F
template<typename R, typename F, typename... T> constexpr
auto fold(R val, F&& op, ka::product_t<T...> const& t)
  -> decltype(detail::fold_impl(val, fwd<F>(op), t, index_sequence_for<T...>{})) {
  return detail::fold_impl(val, fwd<F>(op), t, index_sequence_for<T...>{});
}

/// Polymorphic `+` operation.
struct plus_t {
  template<typename T0, typename T1> constexpr
  auto operator()(T0 const& t0, T1 const& t1) const KA_NOEXCEPT_EXPR(t0 + t1)
    -> decltype(t0 + t1) {
    return t0 + t1;
  }
};

/// Flattens or converts a value into a char.
struct to_char_t {
  template<typename T>
  auto operator()(T&& t) const -> char;
};

inline
auto to_char(char c) -> char {
  return c;
}

namespace detail {
  // Convenience base type for SumValue visitors.
  template<typename D, typename R>
  struct sum_value_visitor_t : boost::static_visitor<R> {
    template<std::size_t I, typename A>
    auto operator()(indexed_t<I, A> const& i) const -> R {
      return static_cast<D const&>(*this)(src(i));
    }
  };

  struct to_char_visitor_t
    : sum_value_visitor_t<to_char_visitor_t, char>
    , to_char_t {
      using sum_value_visitor_t<to_char_visitor_t, char>::operator();
      using to_char_t::operator();
    };
} // namespace detail

/// Gets the value of the variant and converts it to char.
template<typename... T>
auto to_char(boost::variant<T...> const& v) -> char {
  return boost::apply_visitor(detail::to_char_visitor_t{}, v);
}

template<typename T>
auto to_char_t::operator()(T&& t) const -> char {
  return to_char(fwd<T>(t));
}

/// Flattens or converts a value into a string.
struct to_string_t {
  template<typename T>
  auto operator()(T&& t) const -> std::string;
};

static constexpr auto const& to_string = static_const_t<to_string_t>::value;

inline
auto str(char c) -> std::string {
  return std::string(1u, c);
}

template<typename T>
auto str(std::vector<T> const& c) -> std::string {
  auto const c_str = ka::fmap(to_string, c);
  return std::accumulate(c_str.begin(), c_str.end(), std::string{});
}

inline
auto str(std::string s) -> std::string {
  return s;
}

template<typename... T>
auto str(ka::product_t<T...> const& t) -> std::string {
  return fold(std::string{}, plus_t{}, transform(to_string, t));
}

template<std::size_t I, typename T>
auto str(indexed_t<I, T> const& v) -> std::string {
  using ka::detail_uri::parsing::str;
  return str(*v);
}

namespace detail {
  struct to_string_visitor_t
    : sum_value_visitor_t<to_string_visitor_t, std::string>
    , to_string_t {
      using sum_value_visitor_t<to_string_visitor_t, std::string>::operator();
      using to_string_t::operator();
    };
} // namespace detail

template<typename... T>
auto str(boost::variant<T...> const& v) -> std::string {
  return boost::apply_visitor(detail::to_string_visitor_t{}, v);
}

template<typename T>
auto str(opt_t<T> const& v) -> std::string {
  return v.empty() ? std::string{} : str(*v);
}

template<typename T>
auto to_string_t::operator()(T&& t) const -> std::string {
  using ka::detail_uri::parsing::str;
  return str(fwd<T>(t));
}

/// Converts a string into a value.
template<typename T>
struct from_string_t {
  auto operator()(std::string const& s) const -> opt_t<T> {
    std::istringstream iss{ s };
    T v;
    iss >> v;
    if (iss.fail()) {
      return {};
    }
    return ka::opt(v);
  }
};
template<typename T> constexpr
auto from_string(type_t<T>) noexcept -> from_string_t<T> { return {}; }

inline
auto to_lower(std::string s) -> std::string {
  boost::algorithm::to_lower(s);
  return s;
}

inline
auto to_upper(std::string s) -> std::string {
  boost::algorithm::to_upper(s);
  return s;
}

inline
auto decode_percent_unreserved(std::string const& s) -> std::string;

namespace detail_path {
  template<typename I>
  using bounded_range_t = ka::bounded_range_t<I, incr_t>;

  template<typename Component>
  using Iter = decltype(begin(std::declval<Component>()));

  template<typename I>
  using Value = typename std::iterator_traits<I>::value_type;

  template<typename I, typename J>
  auto equ(bounded_range_t<I> const& x, bounded_range_t<J> const& y) -> bool {
    return boost::algorithm::equal(begin(x), end(x), begin(y), end(y));
  }

  template<typename I>
  auto equ(bounded_range_t<I> const& x, I b, I e) -> bool {
    return boost::algorithm::equal(begin(x), end(x), b, e);
  }

  // TODO: Use a template variable when C++14 is available.
  template<typename C>
  auto sep(type_t<C> = {}) -> C {
    return C('/');
  }

  static constexpr char const parent_literal[] = "..";
  static constexpr char const id_literal[] = ".";

  template<typename C>
  using path_t = ka::product_t<std::vector<C>, bool, bool>;

  template<typename C> constexpr
  auto components(path_t<C> const& p) -> std::vector<C> const& {
    return std::get<0>(p);
  }

  template<typename C> constexpr
  auto is_absolute(path_t<C> const& p) -> bool {
    return std::get<1>(p);
  }

  template<typename C> constexpr
  auto is_slash_ended(path_t<C> const& p) -> bool {
    return std::get<2>(p);
  }

  /// OutputIterator O
  template<typename C, typename O>
  auto copy(path_t<C> const& p, O out) -> O {
    auto const s = sep(type_t<Value<Iter<C>>>{});
    if (is_absolute(p)) *out++ = s;
    auto first = true;
    for (auto const& c: components(p)) {
      if (!first) *out++ = s;
      out = std::copy(begin(c), end(c), out);
      first = false;
    }
    return out;
  }

  template<typename C, typename I>
  auto next_component(C sep, I b, I e) -> I {
    return std::find_if_not(b, e, [sep](C const& c) {return c == sep;});
  }

  /// ForwardIterator<C> I
  template <typename C, typename I>
  auto normalize_path(C sep,
                      bounded_range_t<I> const& id,
                      bounded_range_t<I> const& parent,
                      I b, I e) -> path_t<bounded_range_t<I>> {
    if (b == e) return path_t<bounded_range_t<I>>{ {}, false, false };
    auto v = std::vector<bounded_range_t<I>>{};
    auto const abs = *b == sep;
    bool slash_ended = false;
    while (b != e) {
      b = next_component(sep, b, e);
      auto compo = bounded_range_t<I>{ b, std::find(b, e, sep) };
      if (equ(compo, parent)) {
        if (v.empty()) {
          if (!abs) {
            //  /.. = /
            v.emplace_back(begin(compo), end(compo));
          }
        }
        else if (!equ(v.back(), parent)) {
          // (∀x != ..) a / (x / ..) = a / . = a
          v.pop_back();
        }
        else {
          // .. / ..
          v.emplace_back(begin(compo), end(compo));
        }
      }
      else if (!equ(compo, id) && !compo.empty()) {
        // x / . = x
        v.emplace_back(begin(compo), end(compo));
      }
      if (compo.empty() && begin(compo) == e) {
        slash_ended = true;
      }
      b = end(compo);
    }
    if (v.empty() && !abs) v.push_back(id);
    return path_t<bounded_range_t<I>>{ mv(v), abs, slash_ended };
  }
} // namespace detail_path

/// Returns the path in its normal form.
///
/// From https://isocpp.org/files/papers/p0219r0.html#normal-form:
/// > A path is in normal form if it has no redundant current directory (dot) or parent directory
/// > (dot-dot) elements. The normal form for an empty path is an empty path.
///
/// This conversion is purely lexical. It does not check that the path exists, does not follow
/// symlinks, and does not access the filesystem at all.
///
/// @see `std::filesystem::path::lexically_normal`
inline
auto lexically_normal_path(std::string const& path) -> std::string {
  using namespace detail_path;
  using I = std::string::const_iterator;
  static auto const id_str = std::string{ id_literal };
  static auto const parent_str = std::string{ parent_literal };
  auto const sep = detail_path::sep(type_t<char>{});
  bounded_range_t<I> const id { id_str.begin(), id_str.end() };
  bounded_range_t<I> const parent { parent_str.begin(), parent_str.end() };
  std::string res;
  res.reserve(path.size());
  auto const npath = normalize_path(sep, id, parent, path.begin(), path.end());
  copy(npath, std::back_inserter(res));

  // TODO: Improve this part.
  if (is_slash_ended(npath)
      && !components(npath).empty()
      && !equ(components(npath).back(), id)
      && !equ(components(npath).back(), parent)) {
    res += sep;
  }
  return res;
}

inline
auto userinfo(std::string const& s) -> uri_userinfo_t {
  std::string user;
  opt_t<std::string> password;

  auto pos = s.find(':');
  if (pos != std::string::npos) {
    password.emplace_front(s.substr(pos + 1));
    user = s.substr(0, pos);
  }
  else {
    user = s;
  }
  return ka::detail_uri::unchecked_uri_userinfo(user, password);
}

inline
auto authority(ka::product_t<opt_t<ka::product_t<uri_userinfo_t, char>>,
                             std::string,
                             opt_t<ka::product_t<char, opt_t<std::uint16_t>>>> const& v)
  -> uri_authority_t {
  return ka::detail_uri::unchecked_uri_authority(
    std::get<0>(v).fmap([](ka::product_t<uri_userinfo_t, char> const& userinfo) {
      return std::get<0>(userinfo); // ignore the '@' character.
    }),
    std::get<1>(v),
    flatten(std::get<2>(v).fmap([](ka::product_t<char, opt_t<std::uint16_t>> const& port) {
      return std::get<1>(port); // ignore the ':' character.
    })));
}

using hier_part_value_t = ka::product_t<
  opt_t<uri_authority_t>, // authority
  std::string // path
>;

inline
auto hier_part(ApplyIndexed<boost::variant,
                 ka::product_t<char, char, uri_authority_t, std::string>,
                 std::string, std::string, ka::unit_t> const& v) -> hier_part_value_t {
  using Value = hier_part_value_t;
  struct visitor_t : detail::sum_value_visitor_t<visitor_t, Value> {
    using sum_value_visitor_t<visitor_t, Value>::operator();

    // authority path-abempty
    auto operator()(ka::product_t<char, char, uri_authority_t, std::string> const& t) const
      -> Value {
      // ignore 2 chars, which are the "//".
      return Value{ ka::opt(std::get<2>(t)), lexically_normal_path(std::get<3>(t)) };
    }
    // path-absolute / path-rootless
    auto operator()(std::string const& s) const -> Value {
      return Value{ ka::opt_t<uri_authority_t>(), lexically_normal_path(s) };
    }
    // path-empty
    auto operator()(ka::unit_t) const -> Value {
      return Value{ ka::opt_t<uri_authority_t>(), {} };
    }
  };
  return boost::apply_visitor(visitor_t{}, v);
}

inline
auto uri(ka::product_t<std::string,                                       // scheme
                       char,                                              // ':'
                       hier_part_value_t,                                 // authority + path
                       opt_t<ka::product_t<char, std::string>>,           // '?' query
                       opt_t<ka::product_t<char, std::string>>> const& v) // '#' fragment
  -> uri_t {
  auto const hier_part = std::get<2>(v);
  // TODO: Add a polymorphic function type `proj_t<n> = std::get<n>` to simplify
  // `fmap`.
  return ka::detail_uri::unchecked_uri(
    std::get<0>(v),
    std::get<0>(hier_part),
    std::get<1>(hier_part),
    std::get<3>(v)
      .fmap([](ka::product_t<char, std::string> const& q) {
              return std::get<1>(q); // ignore the "?"
            }),
    std::get<4>(v)
      .fmap([](ka::product_t<char, std::string> const& q) {
              return std::get<1>(q); // ignore the "#"
            }));
}

/// Implementation of grammar of URI according to RFC 3986.
///
/// See https://tools.ietf.org/html/rfc3986.
namespace grammar {
using namespace parse::ops;
using namespace pred_ops;

/// Generates the implementation of a parser according to an expression.
#define KA_PARSER_GENERATE(name, expr, vt)                               \
  struct name ## _t {                                                    \
    using Value = vt;                                                    \
  /* Regular: */                                                         \
    KA_GENERATE_FRIEND_REGULAR_OPS_0(name ## _t)                         \
                                                                         \
  /* Functor: (up to isomorphism) */                                     \
    KA_PARSER_DERIVE_FUNCTOR()                                           \
                                                                         \
  /* Parser: */                                                          \
    /** @pre readable_bounded_range(b, e) */                             \
    template<typename I> constexpr                                       \
    auto operator()(I b, I e) const -> parse::res_t<vt, I> {             \
      return ( expr )(b, e);                                             \
    }                                                                    \
  };                                                                     \
  static constexpr auto const& name = static_const_t<name ## _t>::value;

KA_PARSER_GENERATE(alpha,    filter(is_char_class_t{ std::ctype_base::alpha }, parse::symbol), char)
KA_PARSER_GENERATE(alphanum, filter(is_char_class_t{ std::ctype_base::alnum }, parse::symbol), char)
KA_PARSER_GENERATE(digit,    filter(is_char_class_t{ std::ctype_base::digit }, parse::symbol), char)
KA_PARSER_GENERATE(hexdig,   filter(is_char_class_t{ std::ctype_base::xdigit }, parse::symbol), char)

KA_PARSER_GENERATE(colon,    literal(':'), char)
KA_PARSER_GENERATE(dblcolon, literals("::"), ka::product_t<char BOOST_PP_COMMA() char>)
KA_PARSER_GENERATE(period,   literal('.'), char)
KA_PARSER_GENERATE(slash,    literal('/'), char)

// unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
KA_PARSER_GENERATE(unreserved,
  ( alphanum + any_of('-', '.', '_', '~') )
    .fmap(to_char_t{}),
  char )

// pct-encoded = "%" HEXDIG HEXDIG
KA_PARSER_GENERATE(pct_encoded,
  ( literal('%') * hexdig * hexdig )
    .fmap(to_string)
    // RFC 3986 - 6.2.2.1.  Case Normalization
    //   For all URIs, the hexadecimal digits within a percent-encoding
    //   triplet (e.g., "%3a" versus "%3A") are case-insensitive and therefore
    //   should be normalized to use uppercase letters for the digits A-F.
    .fmap(to_upper)
    // RFC 3986 - 6.2.2.2.  Percent-Encoding Normalization
    //   In addition to the case normalization issue noted above, some URI producers percent-encode
    //   octets that do not require percent-encoding, resulting in URIs that are equivalent to their
    //   non-encoded counterparts.  These URIs should be normalized by decoding any percent-encoded
    //   octet that corresponds to an unreserved character, as described in Section 2.3.
    .fmap(decode_percent_unreserved),
  std::string )

// gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
KA_PARSER_GENERATE(gen_delims,
  any_of(':', '/', '?', '#', '[', ']', '@'),
  char )

// sub-delims = "!" / "$" / "&" / "'" / "(" / ")"
//                  / "*" / "+" / "," / ";" / "="
KA_PARSER_GENERATE(sub_delims,
  any_of('!', '$', '&', '\'', '(', ')',
                    '*', '+', ',',  ';', '='),
  char )

// reserved = gen-delims / sub-delims
KA_PARSER_GENERATE(reserved, (gen_delims + sub_delims).fmap(to_char_t{}), char)

// pchar = unreserved / pct-encoded / sub-delims / ":" / "@"
KA_PARSER_GENERATE(pchar,
  // First alternative that succeeds stops, so we put the greediest alternatives first.
  ( pct_encoded + unreserved + sub_delims + any_of(':', '@') )
  .fmap(to_string),
  std::string)

// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
KA_PARSER_GENERATE(scheme,
  ( alpha * *(alphanum + any_of('+', '-', '.')) )
    .fmap(to_string)
    // RFC 3986 - 6.2.2.1.  Case Normalization
    //  When a URI uses components of the generic syntax, the component
    //  syntax equivalence rules always apply; namely, that the scheme and
    //  host are case-insensitive and therefore should be normalized to
    //  lowercase.
    .fmap(to_lower),
  std::string)

// userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
KA_PARSER_GENERATE(userinfo,
  // First alternative that succeeds stops, so we put the greediest alternatives first.
  ( *(pct_encoded + unreserved + sub_delims + colon) )
    .fmap(to_string).fmap(parsing::userinfo), uri_userinfo_t)

KA_PARSER_GENERATE(r1_to_9, any_of('1', '2', '3', '4', '5', '6', '7', '8', '9'), char )
KA_PARSER_GENERATE(r0_to_4, any_of('0', '1', '2', '3', '4'), char )
KA_PARSER_GENERATE(r0_to_5, any_of('0', '1', '2', '3', '4', '5'), char )

// dec-octet = DIGIT                 ; 0-9
//           / %x31-39 DIGIT         ; 10-99
//           / "1" 2DIGIT            ; 100-199
//           / "2" %x30-34 DIGIT     ; 200-249
//           / "25" %x30-35          ; 250-255
KA_PARSER_GENERATE(dec_octet,
  // First alternative that succeeds stops, so we put the greediest alternatives first.
  ( (literals("25") * r0_to_5)
    + (literal('2') * r0_to_4 * digit)
    + (literal('1') * power(digit, size_constant_t<2>{}))
    + (r1_to_9 * digit)
    +  digit )
    .fmap(to_string), std::string)

// IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
KA_PARSER_GENERATE(ipv4address,
  (dec_octet * power(period * dec_octet, size_constant_t<3>{}))
    .fmap(to_string), std::string)

// h16 = 1*4HEXDIG
//     ; 16 bits of address represented in hexadecimal
KA_PARSER_GENERATE(h16, between(1, 4, hexdig).fmap(to_string), std::string);
KA_PARSER_GENERATE(h16colon, (h16 * colon).fmap(to_string), std::string)

// ls32 = ( h16 ":" h16 ) / IPv4address
//      ; least-significant 32 bits of address
KA_PARSER_GENERATE(ls32, ((h16colon * h16) + ipv4address).fmap(to_string), std::string)

// IPv6address =                            6( h16 ":" ) ls32
//             /                       "::" 5( h16 ":" ) ls32
//             / [               h16 ] "::" 4( h16 ":" ) ls32
//             / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
//             / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
//             / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
//             / [ *4( h16 ":" ) h16 ] "::"              ls32
//             / [ *5( h16 ":" ) h16 ] "::"              h16
//             / [ *6( h16 ":" ) h16 ] "::"
KA_PARSER_GENERATE(ipv6address,
 (   (                                                 power(h16colon, size_constant_t<6>{}) * ls32)
   + (                                     dblcolon  * power(h16colon, size_constant_t<5>{}) * ls32)
   + (                   parse::opt(h16) * dblcolon  * power(h16colon, size_constant_t<4>{}) * ls32)
   + (((between(1, 2, h16colon) * colon) + dblcolon) * power(h16colon, size_constant_t<3>{}) * ls32)
   + (((between(1, 3, h16colon) * colon) + dblcolon) * power(h16colon, size_constant_t<2>{}) * ls32)
   + (((between(1, 4, h16colon) * colon) + dblcolon) *                             h16colon  * ls32)
   + (((between(1, 5, h16colon) * colon) + dblcolon)                                         * ls32)
   + (((between(1, 6, h16colon) * colon) + dblcolon)                                         * h16 )
   + (((between(1, 7, h16colon) * colon) + dblcolon)                                               ) )
     .fmap(to_string),
   std::string)

// IPvFuture = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
KA_PARSER_GENERATE(ipvfuture,
  ( literal('v') * at_least(1, hexdig) * period * at_least(1, unreserved + sub_delims + colon) )
    .fmap(to_string),
  std::string)

// IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
KA_PARSER_GENERATE(ip_literal,
  ( literal('[') * (ipv6address + ipvfuture) * literal(']') )
    .fmap(to_string),
  std::string)

// reg-name = *( unreserved / pct-encoded / sub-delims )
KA_PARSER_GENERATE(regname,
  ( *(unreserved + pct_encoded + sub_delims) )
    .fmap(to_string),
  std::string)

// host = IP-literal / IPv4address / reg-name
KA_PARSER_GENERATE(host, (ip_literal + ipv4address + regname)
    .fmap(to_string)
    // RFC 3986 - 6.2.2.1.  Case Normalization
    //  When a URI uses components of the generic syntax, the component
    //  syntax equivalence rules always apply; namely, that the scheme and
    //  host are case-insensitive and therefore should be normalized to
    //  lowercase.
    .fmap(to_lower),
  std::string)

// port = *DIGIT
KA_PARSER_GENERATE(port,
  parse::list(digit).fmap(to_string).fmap(from_string(type_t<std::uint16_t>{})),
  opt_t<std::uint16_t>)

// authority = [ userinfo "@" ] host [ ":" port ]
KA_PARSER_GENERATE(authority,
  ( parse::opt(userinfo * literal('@')) * host * parse::opt(colon * port) )
    .fmap(parsing::authority),
  uri_authority_t)

// segment = *pchar
KA_PARSER_GENERATE(segment, parse::list(pchar).fmap(to_string), std::string)
KA_PARSER_GENERATE(segment_list, parse::list(slash * segment).fmap(to_string), std::string)

// segment-nz = 1*pchar
KA_PARSER_GENERATE(segment_nz, at_least(1, pchar).fmap(to_string), std::string)

// path-abempty = *( "/" segment )
KA_PARSER_GENERATE(path_abempty, segment_list.fmap(to_string), std::string)

// path-absolute = "/" [ segment-nz *( "/" segment ) ]
KA_PARSER_GENERATE(path_absolute,
  (slash * parse::opt(segment_nz * segment_list))
    .fmap(to_string),
  std::string)

// path-rootless = segment-nz *( "/" segment )
KA_PARSER_GENERATE(path_rootless, (segment_nz * segment_list).fmap(to_string), std::string)

// path-empty = 0<pchar>
// meaning(path-empty) = 1
// hier-part = "//" authority path-abempty
//             / path-absolute
//             / path-rootless
//             / path-empty
KA_PARSER_GENERATE(hier_part,
  ( (literals("//") * authority * path_abempty)
    + path_absolute
    + path_rootless
    + parse::unit )
    .fmap(parsing::hier_part),
  hier_part_value_t)

// query = *( pchar / "/" / "?" )
KA_PARSER_GENERATE(query,
  parse::list(pchar + slash + literal('?'))
    .fmap(to_string),
  std::string)

// fragment = *( pchar / "/" / "?" )
KA_PARSER_GENERATE(fragment, query, std::string)

// URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
KA_PARSER_GENERATE(uri,
  ( scheme * literal(':') * hier_part
           * parse::opt(literal('?') * query)
           * parse::opt(literal('#') * fragment) )
    .fmap(parsing::uri),
  ka::uri_t)

#undef KA_PARSER_GENERATE
} // namespace grammar

inline
auto decode_percent_unreserved(std::string const& s) -> std::string {
  if (s.size() != 3)
    return s;

  unsigned int encoded_char = 0;
  std::istringstream iss(s);
  char percent = '\0';
  iss >> percent >> std::hex >> encoded_char;
  assert(percent == '%');
  assert(encoded_char <= static_cast<unsigned int>(std::numeric_limits<char>::max()));
  auto const decoded_char = static_cast<char>(encoded_char);
  // If it is a reserved character, return the percent encoded version.
  if (!grammar::reserved(&decoded_char, &decoded_char + 1).empty()) {
    return s;
  }
  return std::string(1u, decoded_char);
}

} // namespace parsing
} // namespace detail_uri
} // namespace ka

#endif // KA_URI_PARSING_HPP
