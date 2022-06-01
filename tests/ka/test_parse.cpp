#include <gtest/gtest.h>
#include <cctype>
#include <algorithm>
#include <functional>
#include <iterator>
#include <boost/utility/string_ref.hpp>
#include <ka/parse.hpp>
#include <ka/testutils.hpp>
#include <ka/conceptpredicate.hpp>
#include <ka/src.hpp>

using namespace ka;
using namespace ka::parse;
using namespace ka::test;

// A result of parsing is Regular.
TEST(ParseResult, Regular) {
  A const c[4];
  auto const t = type_t<A>{};
  // `ok` and `err` returns parsing results. Iterators are arbitrary values here.
  EXPECT_TRUE(is_regular({ ok(A{9}, c+0), ok(A{0}, c+1), err(t, c+2), err(t, c+3) }));
}

// A result of parsing contains optionally a value and an iterator to what
// remains to parse.
TEST(ParseResult, Basic) {
  auto const a = A{4};
  auto const i = &a + 1;
  {
    auto const res = ok(a, i); // Ok: contains a value.
    EXPECT_EQ(a, src(res));
    EXPECT_EQ(i, iter(res));
  }
  {
    auto const res = err(type_t<A>{}, i); // Ko: does not contain a value.
    EXPECT_TRUE(ka::empty(res)); // Therefore, calling `src` is undefined behavior.
    EXPECT_EQ(i, iter(res)); // Always ok to get the iterator.
  }
}

// A result of parsing is convertible to an optional.
TEST(ParseResult, ToOpt) {
  using std::get;
  auto const t = type_t<A>{};
  auto test_data = array_fn(
    //           input             expected output
    //           ----------------  ---------------
    ka::product( ok(A{4}, &t),     ka::opt(A{4})  ), // Iterators are arbitrary.
    ka::product( err(t, &t + 3),   ka::opt_t<A>() ),
    ka::product( ok(A{9}, &t + 1), ka::opt(A{9})  )
  );
  for (auto const& x: test_data) {
    EXPECT_EQ(get<1>(x), to_opt(get<0>(x)));
  }
}

struct equ_fn_res_t {
  // Function<res_t<B, I> (res_t<A, I>)> F, G
  template<typename F, typename G>
  auto operator()(F f, G g) const -> bool {
    auto const a = A{45};
    auto const t = type_t<A>{};
    for (auto const& x: array_fn(ok(A{4}, &a), ok(A{5}, &a+1), ok(A{81}, &a+2),
        err(t, &a), err(t, &a+1), err(t, &a+2))) {
      if (f(x) != g(x)) return false;
    }
    return true;
  }
};

// `parse::res_t` is a functor on its value type.
TEST(ParseResult, Functor) {
  // Composable functions.
  auto const f = [](A a) -> B { return B{2 * a.value}; };
  auto const g = [](B b) -> C { return C{b.value + 1}; };

  EXPECT_TRUE(is_functor(ka::fmap, array_fn(ka::product(g, f)), equ_fn_res_t{}));
}

// A parser is a function from a range of elements to a parsing result.
// Range of elements is bound by a pair of iterators.
// A parser can succeed or fail. If it fails, input is not consumed (i.e. begin
// iterator is returned).
TEST(ParseElement, Basic) {
  using namespace ka::parse;
  auto p = symbol_t{}; // Parser of one symbol.
  A const v[] = {A{5}, A{-46}};

  // Succeeds.
  EXPECT_EQ(ok(v[0], v + 1), p(v + 0, v + 2));

  // Fails.
  auto const t = type_t<A>{};
  EXPECT_EQ(err(t, v + 1), p(v + 1, v + 1)); // empty range
}

// Linearizable<Linearizable> L
// Relation<ParseResult> R
template<typename L, typename R = equal_t>
struct equ_parser_t {
  L ranges;
  R equiv; // equivalence(equiv)

  template<typename PA, typename PB>
  auto operator()(PA pa, PB pb) const -> bool {
    for (auto const& r: ranges) {
      auto b = r.begin();
      auto e = r.end();
      if (! equiv(pa(b, e), pb(b, e))) return false;
    }
    return true;
  }
};

template<typename L, typename R = equal_t>
auto equ_parser(L ranges, R equiv = {}) -> equ_parser_t<L, R> {
  return {mv(ranges), mv(equiv)};
}

struct equ_fn_symbol_t {
  // Function<Parser<A> (Parser)> F, G
  template<typename F, typename G>
  auto operator()(F f, G g) const -> bool {
    using V = std::vector<A>;
    auto equ = equ_parser(array_fn(V{}, V{A{4}}, V{A{0}, A{9}, A{-8}, A{836}}));
    return equ(f(symbol), g(symbol));
  }
};

// A parametrized parser is a functor on its value type.
TEST(ParseFunctor, Basic) {
  // Composable functions.
  auto const f = [](A a) -> B {return B{2 * a.value};};
  auto const g = [](B b) -> C {return C{b.value + 1};};

  // Functor test algorithm:
  // 1) Lift functions f,g to functions of parsers.
  //    (f: A -> B)   -> (f': symbol_t -> symbol_t)  // Done via
  //    (g: B -> C)   -> (g': symbol_t -> symbol_t)  // ka::fmap.
  //    (g∘f: A -> C) -> ((g∘f)': symbol_t -> symbol_t)
  //
  // 2) Compare lifted functions through the given equivalence:
  //    g' ∘ f' ~ (g'∘f')
  EXPECT_TRUE(is_functor(ka::fmap, array_fn(ka::product(g, f)), equ_fn_symbol_t{}));
}

namespace test_parse {
  // Characters associated to test types, for parsing purpose.
  auto char_(type_t<A>) -> char {return 'A';}
  auto char_(type_t<B>) -> char {return 'B';}
  auto char_(type_t<C>) -> char {return 'C';}

  // Simple parser for test purpose.
  // Expected format: char digit
  //  (where `char` depends on type `T`)
  // Example of valid input for type `A`: "A4"
  template<typename T>
  struct sym_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(sym_t)
  // Parser:
    template<typename I> constexpr
    auto operator()(I b, I e) const -> parse::res_t<T, I> {
      return (b != e && *b == char_(type_t<T>{}) && ++b != e && std::isdigit(*b))
        ? ok(T{*b - '0'}, std::next(b))
        : err(type_t<T>{}, b);
    }
  // Functor:
    template<typename F>
    auto fmap(F f) const -> fmapped_t<F, sym_t> {
      return {mv(f), *this};
    }
  };

  auto input_sample() -> std::array<boost::string_ref, 14> const& {
    static auto const a = std::array<boost::string_ref, 14>{
      "", "A3", "A3A6", "A3A6A7", "B3", "B3A6", "3A", "$jH9w_?", "A3B5", "A3C7",
      "B8A7A", "C0A1", "B9C7A8A8", "C0B0"
    };
    return a;
  }
} // namespace test_parse

// Product of parsers succeeds if the sequence of all of them succeeds.
TEST(ParseProduct, Basic) {
  auto const s = std::string("A5B2C5");
  auto const b = s.begin();
  auto const e = s.end();
  auto const pa = test_parse::sym_t<A>{};
  auto const pb = test_parse::sym_t<B>{};

  // Ok.
  EXPECT_EQ(ok(ka::product(A{5}, B{2}), b + 4), parse::product(pa, pb)(b, e));

  // Ko: first parser failed (could not parse a `B`).
  auto const t0 = type_t<ka::product_t<B, A>>{};
  EXPECT_EQ(err(t0, b), parse::product(pb, pa)(b, e));

  // Ko: second parser failed (could not parse an `A`).
  auto const t1 = type_t<ka::product_t<A, A>>{};
  EXPECT_EQ(err(t1, b), parse::product(pa, pa)(b, e));
}

// Product operator is associative.
// Product operator flattens products, instead of nesting them.
// E.g. `pa * pb * pc` has type `parse::product<PA, PB, PC>` instead of
// `parse::product<parse::product<PA, PB>, PC>`.
TEST(ParseProductMonoid, Operator) {
  using parse::ops::operator*;
  auto pa = test_parse::sym_t<A>{};
  auto pb = test_parse::sym_t<B>{};
  auto pc = test_parse::sym_t<C>{};

  EXPECT_EQ((pa * pb) * pc, pa * (pb * pc));

  EXPECT_EQ(parse::product(pa, pb), pa * pb);
  EXPECT_EQ(parse::product(pa, pb, pc), pa * pb * pc);
  EXPECT_EQ(parse::product(pa, pb, pc, pb, pa), pa * pb * pc * pb * pa);
}

// Unit-parser always succeeds without consuming input and returns ka::unit.
TEST(ParseUnit, Basic) {
  {
    auto const v = std::array<A, 3>{A{5}, A{1}, A{9}};
    EXPECT_EQ(ok(ka::unit, v.begin()), parse::unit(v.begin(), v.end()));
  } {
    auto const v = std::array<B, 2>{B{7}, B{1}};
    EXPECT_EQ(ok(ka::unit, v.begin()), parse::unit(v.begin(), v.end()));
  }
}

namespace test_parse {
  // Makes res_t<T, I> comparable with res_t<product_t<T>, I> to implement logic
  // "up to isomorphism" (T is isomorphic to product_t<T>).
  template<typename T, typename I>
  auto extract(parse::res_t<T, I> const& a) -> parse::res_t<T, I> const& {
    return a;
  }

  template<typename T, typename I>
  auto extract(parse::res_t<ka::product_t<T>, I> const& a) -> parse::res_t<T, I> {
    return a.fmap([](ka::product_t<T> const& p) -> T {
      return std::get<0>(p);
    });
  }

  // A ≅ (A)
  // (A) ≅ A
  struct equiv_res_product_t {
    template<typename T, typename I, typename U>
    auto operator()(parse::res_t<T, I> const& a, parse::res_t<U, I> const& b) const
      -> bool {
      return extract(a) == extract(b);
    }
  } equiv_res_product;
} // namespace test_parse

// Unit-parser is the unit of the parser product (up to isomorphism).
TEST(ParseProductMonoid, Unit) {
  using parse::ops::operator*;
  using namespace test_parse;
  auto p = sym_t<A>{};
  auto equ = equ_parser(input_sample(), test_parse::equiv_res_product);
  auto _1 = parse::unit;
  EXPECT_EQ(_1, parse::product());
  EXPECT_TRUE(equ(_1 * _1, _1));
  EXPECT_TRUE(equ(p * _1, p));
  EXPECT_TRUE(equ(_1 * p, p));
}

// Sum of parsers succeeds if any succeeds. Parsers are tried in left-to-right
// order.
TEST(ParseSum, Basic) {
  auto const s = std::string("A5B2C5");
  auto const b = s.begin();
  auto const e = s.end();
  auto const pa = test_parse::sym_t<A>{};
  auto const pb = test_parse::sym_t<B>{};
  auto const pc = test_parse::sym_t<C>{};

  { // Ok: First alternative.
    using sum = SumValue<A, B>;
    EXPECT_EQ(ok(sum{indexed<0>(A{5})}, b + 2), parse::sum(pa, pb)(b, e));
  }
  { // Ok: second alternative.
    using sum = SumValue<B, A>;
    EXPECT_EQ(ok(sum{indexed<1>(A{5})}, b + 2), parse::sum(pb, pa)(b, e));
  }
  { // Ko.
    using sum = SumValue<B, C>;
    auto t = type_t<sum>{};
    EXPECT_EQ(err(t, b), parse::sum(pb, pc)(b, e));
  }
}

// Sum operator is associative.
// Sum operator flattens sums, instead of nesting them.
// E.g. `pa + pb + pc` has type `parse::sum<PA, PB, PC>` instead of
// `parse::sum<parse::sum<PA, PB>, PC>`.
TEST(ParseSumMonoid, Operator) {
  using parse::ops::operator+;
  auto pa = test_parse::sym_t<A>{};
  auto pb = test_parse::sym_t<B>{};
  auto pc = test_parse::sym_t<C>{};

  EXPECT_EQ((pa + pb) + pc, pa + (pb + pc));
  EXPECT_EQ(parse::sum(pa, pb), pa + pb);
  EXPECT_EQ(parse::sum(pa, pb, pc), pa + pb + pc);
  EXPECT_EQ(parse::sum(pa, pb, pc, pb, pa), pa + pb + pc + pb + pa);
}

// Zero-parser always failed without consuming input. Its value type is
// uninstantiable (sum of zero type).
TEST(ParseZero, Basic) {
  auto const t = type_t<ka::zero_t>{};
  {
    auto const v = std::array<A, 3>{A{5}, A{1}, A{9}};
    EXPECT_EQ(err(t, v.begin()), parse::zero(v.begin(), v.end()));
  } {
    auto const v = std::array<B, 2>{B{7}, B{1}};
    EXPECT_EQ(err(t, v.begin()), parse::zero(v.begin(), v.end()));
  }
}

namespace test_parse {
  // The following functions implement parsing result equality up-to-isomorphism.
  // Their precondition is that results are not empty.
  struct equiv_res_zero_t {
    // 0 + 0 = 0
    // Should not be defined, but linker complains...
    auto equ_val(SumValue<ka::zero_t, ka::zero_t>, ka::zero_t) const -> bool {
      return true;
    }
    // A + 0 ≅ A
    template<typename A>
    auto equ_val(SumValue<A, ka::zero_t> x, A y) const -> bool {
      return *boost::get<indexed_t<0, A>>(x) == y;
    }
    // 0 + A ≅ A
    template<typename A, int = 0> // TODO: remove the int when MSVC > 2015
    auto equ_val(SumValue<ka::zero_t, A> x, A y) const -> bool {
      return *boost::get<indexed_t<1, A>>(x) == y;
    }
    // 0 ≅ 0 * A
    // Should not be defined, but linker complains...
    template<typename A>
    auto equ_val(ka::zero_t, ka::product_t<ka::zero_t, A>) const -> bool {
      return true;
    }
    // 0 ≅ A * 0
    // Should not be defined, but linker complains...
    template<typename A>
    auto equ_val(ka::zero_t, ka::product_t<A, ka::zero_t>) const -> bool {
      return true;
    }
    template<typename A, typename I, typename B>
    auto operator()(res_t<A, I> const& a, res_t<B, I> const& b) const -> bool {
      auto ea = a.empty();
      auto eb = b.empty();
      return ea == eb && iter(a) == iter(b) && (ea || equ_val(src(a), src(b)));
    }
  } equiv_res_zero;

} // namespace test_parse

// Zero-parser is the unit of the parser sum (up to isomorphism).
TEST(ParseSumMonoid, Zero) {
  using parse::ops::operator+;
  using namespace test_parse;
  auto equ = equ_parser(input_sample(), equiv_res_zero);
  auto _0 = parse::zero;
  auto p = sym_t<A>{};
  EXPECT_EQ(_0, parse::sum());
  EXPECT_TRUE(equ(_0 + _0, _0));
  EXPECT_TRUE(equ(p + _0, p));
  EXPECT_TRUE(equ(_0 + p, p));
}

// A semiring relates sum and product in the usual manner:
//  - 0 ≠ 1 (ensured by type system)
//  - annihilation property (this test):
///     0 = 0 * a = a * 0
//  - distributivity (next test):
//      a * (b + c) = (a * b) + (a * c)
//      (b + c) * a = (b * a) + (c * a)
TEST(ParseQuasiSemiring, AnnihilationProperty) {
  using parse::ops::operator*;
  using namespace test_parse;
  auto equ = equ_parser(input_sample(), equiv_res_zero);
  auto a = sym_t<A>{};
  auto _0 = parse::zero;
  EXPECT_TRUE(equ(_0, _0 * a));
  EXPECT_TRUE(equ(_0, a * _0));
}

namespace test_parse {
  struct equiv_res_distrib_t {
    template<typename IndexedPtr>
    auto src_idx(IndexedPtr p) const -> decltype(&**p) {
      return p != nullptr ? &**p : nullptr;
    }
    // Used by test for distributivity.
    template<typename A, typename B, typename C>
    auto distrib_equ(A& x_a, B* x_b, C* x_c, A& y_a, B* y_b, C* y_c) const -> bool {
      if ( (x_b == nullptr) != (y_b == nullptr)
        || (x_c == nullptr) != (y_c == nullptr)) return false;
      return x_a == y_a
        && (x_b != nullptr
            ? *x_b == *y_b
            : *x_c == *y_c);
    }
    // A * (B + C) ≅ (A * B) + (A * C)
    template<typename A, typename B, typename C>
    auto equ_val(
      ka::product_t<                                     // *
        A,                                               // A
        boost::variant<indexed_t<0, B>, indexed_t<1, C>> // B + C
      > const& x,
      boost::variant<                      // +
        indexed_t<0, ka::product_t<A, B>>, // A * B
        indexed_t<1, ka::product_t<A, C>>  // A * C
      > const& y) const -> bool {

      auto* y_ab = src_idx(boost::get<indexed_t<0, ka::product_t<A, B>>>(&y));
      auto* y_ac = src_idx(boost::get<indexed_t<1, ka::product_t<A, C>>>(&y));
      auto& y_a = y_ab != nullptr ? std::get<0>(*y_ab) : std::get<0>(*y_ac);
      auto* y_b = y_ab != nullptr ? &std::get<1>(*y_ab) : nullptr;
      auto* y_c = y_ac != nullptr ? &std::get<1>(*y_ac) : nullptr;
      auto& x_a = std::get<0>(x);
      auto* x_b = src_idx(boost::get<indexed_t<0, B>>(&std::get<1>(x)));
      auto* x_c = src_idx(boost::get<indexed_t<1, C>>(&std::get<1>(x)));
      return distrib_equ(x_a, x_b, x_c, y_a, y_b, y_c);
    }

    // (B + C) * A ≅ (B * A) + (C * A)
    // Factorization seems more trouble than having two versions.
    template<typename A, typename B, typename C>
    auto equ_val(
      ka::product_t<                                      // *
        boost::variant<indexed_t<0, B>, indexed_t<1, C>>, // B + C
        A                                                 // A
      > const& x,
      boost::variant<                      // +
        indexed_t<0, ka::product_t<B, A>>, // B * A
        indexed_t<1, ka::product_t<C, A>>  // C * A
      > const& y) const -> bool {

      auto* y_ba = src_idx(boost::get<indexed_t<0, ka::product_t<B, A>>>(&y));
      auto* y_ca = src_idx(boost::get<indexed_t<1, ka::product_t<C, A>>>(&y));
      auto& y_a = y_ba != nullptr ? std::get<1>(*y_ba) : std::get<1>(*y_ca);
      auto* y_b = y_ba != nullptr ? &std::get<0>(*y_ba) : nullptr;
      auto* y_c = y_ca != nullptr ? &std::get<0>(*y_ca) : nullptr;
      auto& x_a = std::get<1>(x);
      auto* x_b = src_idx(boost::get<indexed_t<0, B>>(&std::get<0>(x)));
      auto* x_c = src_idx(boost::get<indexed_t<1, C>>(&std::get<0>(x)));
      return distrib_equ(x_a, x_b, x_c, y_a, y_b, y_c);
    }
    template<typename A, typename I, typename B>
    auto operator()(res_t<A, I> const& a, res_t<B, I> const& b) const -> bool {
      auto ea = a.empty();
      auto eb = b.empty();
      return ea == eb && iter(a) == iter(b) && (ea || equ_val(src(a), src(b)));
    }
  } equiv_res_distrib;
} // namespace test_parse

TEST(ParseQuasiSemiring, Distributivity) {
  using parse::ops::operator*;
  using parse::ops::operator+;
  using namespace test_parse;
  auto equ = equ_parser(input_sample(), equiv_res_distrib);
  auto a = sym_t<A>{};
  auto b = sym_t<B>{};
  auto c = sym_t<C>{};
  EXPECT_TRUE(equ(a * (b + c), (a * b) + (a * c)));
  EXPECT_TRUE(equ((b + c) * a, (b * a) + (c * a)));
}

namespace test_parse {
  struct equiv_res_opt_t {
    // ka::opt_t<A> ≅ A + 1
    template<typename A>
    auto equ_val(ka::opt_t<A> x, SumValue<A, ka::unit_t> y) const -> bool {
      using ka::src;
      auto* y_a = boost::get<indexed_t<0, A>>(&y);
      return x.empty()
        ? y_a == nullptr
        : src(x) == src(src(y_a));
    }
    template<typename A, typename I, typename B>
    auto operator()(res_t<A, I> const& a, res_t<B, I> const& b) const -> bool {
      auto ea = a.empty();
      auto eb = b.empty();
      return ea == eb && iter(a) == iter(b) && (ea || equ_val(src(a), src(b)));
    }
  } equiv_res_opt;

} // namespace test_parse

// An optional parser of `A` always succeeds: if `A` cannot be parsed, it
// returns unit.
// opt(p) ≅ p + 1
TEST(ParseOpt, Basic) {
  using parse::ops::operator+;
  using namespace test_parse;
  auto _1 = parse::unit;
  auto p = sym_t<A>{};
  auto equ = equ_parser(input_sample(), equiv_res_opt);
  EXPECT_TRUE(equ(parse::opt(p), p + _1));
}

TEST(ParseQuantify, Basic) {
  auto const pa = test_parse::sym_t<A>{};
  using V = std::vector<A>;
  {
    { // Ok: No element.
      auto const s = std::string("A5A2B5");
      auto const b = s.begin();
      auto const e = s.end();
      auto p = quantify(pa, 0, 0);
      EXPECT_EQ(ok(V{}, b), p(b, e));
      EXPECT_EQ(ok(V{}, b), p(b, b)); // empty range
    }
    { // Ok: No element.
      auto const s = std::string("C5A2B5");
      auto const b = s.begin();
      auto const e = s.end();
      EXPECT_EQ(ok(V{}, b), quantify(pa, 0, 1)(b, e));
      EXPECT_EQ(ok(V{}, b), quantify(pa, 0, 2)(b, e));
      EXPECT_EQ(ok(V{}, b), quantify(pa, 0, 3)(b, e));
      EXPECT_EQ(ok(V{}, b), quantify(pa, 0)(b, e));
    }
    { // Ok: One element.
      auto const s = std::string("A5C2B5");
      auto const b = s.begin();
      auto const e = s.end();
      EXPECT_EQ(ok(V{A{5}}, b + 2), quantify(pa, 0, 1)(b, e));
      EXPECT_EQ(ok(V{A{5}}, b + 2), quantify(pa, 0, 2)(b, e));
      EXPECT_EQ(ok(V{A{5}}, b + 2), quantify(pa, 0, 3)(b, e));
      EXPECT_EQ(ok(V{A{5}}, b + 2), quantify(pa, 0)(b, e));
      EXPECT_EQ(ok(V{A{5}}, b + 2), quantify(pa, 1, 1)(b, e));
      EXPECT_EQ(ok(V{A{5}}, b + 2), quantify(pa, 1, 2)(b, e));
      EXPECT_EQ(ok(V{A{5}}, b + 2), quantify(pa, 1, 3)(b, e));
      EXPECT_EQ(ok(V{A{5}}, b + 2), quantify(pa, 1)(b, e));
    }
    { // Ok: One or two elements.
      auto const s = std::string("A5A2B5");
      auto const b = s.begin();
      auto const e = s.end();
      EXPECT_EQ(ok(V{A{5}},       b + 2), quantify(pa, 0, 1)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 0, 2)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 0, 3)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 0)(b, e));
      EXPECT_EQ(ok(V{A{5}},       b + 2), quantify(pa, 1, 1)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 1, 2)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 1, 3)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 1)(b, e));
    }
    { // Ok: Two elements.
      auto const s = std::string("A5A2B5");
      auto const b = s.begin();
      auto const e = s.end();
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 0, 2)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 0)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 1, 2)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 1)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 2, 2)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 2, 3)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 2, 4)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), quantify(pa, 2)(b, e));
    }
    { // Ok: From two to three elements.
      auto const s = std::string("A5A2A6A9B5");
      auto const b = s.begin();
      auto const e = s.end();
      EXPECT_EQ(ok(V{A{5}, A{2}, A{6}}, b + 6), quantify(pa, 2, 3)(b, e));
      EXPECT_EQ(ok(V{A{5}, A{2}, A{6}}, b + 6), quantify(pa, 2, 3)(b, b + 6));
    }
    { // Ko: At least one element.
      auto const s = std::string("B5A2A6A9B5");
      auto const b = s.begin();
      auto const e = s.end();
      auto const t = type_t<V>{};
      EXPECT_EQ(err(t, b), quantify(pa, 1, 1)(b, e));
      EXPECT_EQ(err(t, b), quantify(pa, 1, 2)(b, e));
      EXPECT_EQ(err(t, b), quantify(pa, 1, 3)(b, e));
      EXPECT_EQ(err(t, b), quantify(pa, 1)(b, e));
    }
    { // Ko: From three to four elements.
      auto const s = std::string("A5A2B5");
      auto const b = s.begin();
      auto const e = s.end();
      auto const t = type_t<V>{};
      EXPECT_EQ(err(t, b), quantify(pa, 3, 4)(b, e));
    }
  }
}

namespace test_parse {
  struct list_fn_t {
    template<typename PA> constexpr
    auto operator()(PA&& pa) const -> decltype(list(fwd<PA>(pa))) {
      return list(fwd<PA>(pa));
    }
  };

  struct quantify_0_fn_t {
    template<typename PA> constexpr
    auto operator()(PA&& pa) const -> decltype(quantify(fwd<PA>(pa), 0)) {
      return quantify(fwd<PA>(pa), 0);
    }
  };

  using list_types = testing::Types<
    list_fn_t,
    quantify_0_fn_t
  >;
} // namespace test_parse

template<typename T> struct ParseList : testing::Test {};
TYPED_TEST_SUITE(ParseList, test_parse::list_types);

// List of a parser applies it as much as possible.
TYPED_TEST(ParseList, Basic) {
  auto list = TypeParam{};
  auto const pa = test_parse::sym_t<A>{};
  using V = std::vector<A>;

  { // Ok: No element.
    auto const s = std::string("C5A2B5");
    auto const b = s.begin();
    auto const e = s.end();
    EXPECT_EQ(ok(V{}, b), list(pa)(b, e));
  }
  { // Ok: One element.
    auto const s = std::string("A5B2B5");
    auto const b = s.begin();
    auto const e = s.end();
    EXPECT_EQ(ok(V{A{5}}, b + 2), list(pa)(b, e));
  }
  { // Ok: Two elements, range's end not reached.
    auto const s = std::string("A5A2C5");
    auto const b = s.begin();
    auto const e = s.end();
    EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), list(pa)(b, e));
  }
  { // Ok: Two elements, range's end reached.
    auto const s = std::string("A5A2");
    auto const b = s.begin();
    auto const e = s.end();
    EXPECT_EQ(ok(V{A{5}, A{2}}, b + 4), list(pa)(b, e));
  }
  { // Ok: Five elements, range's end not reached.
    auto const s = std::string("A5A2A8A9A0B7");
    auto const b = s.begin();
    auto const e = s.end();
    EXPECT_EQ(ok(V{A{5}, A{2}, A{8}, A{9}, A{0}}, b + 10), list(pa)(b, e));
  }
}

// List operator flattens lists, instead of nesting them.
// E.g. `**pa` has type `parse::list<PA>` instead of
// `parse::list<parse::list<PA>>`.
TEST(ParseList, Operator) {
  using parse::ops::operator*;
  auto pa = test_parse::sym_t<A>{};
  EXPECT_EQ(list(pa), *pa);
  EXPECT_EQ(*pa, **pa);
  EXPECT_EQ(*pa, ***pa);
  EXPECT_EQ(*pa, ************pa);
}

// TODO: Test list concatenation when available.
// TODO: Test empty list when available.
// TODO: Test list monoid when available.
