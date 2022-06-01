#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <ka/functional.hpp>
#include <ka/productiter.hpp>
#include <ka/testutils.hpp>
#include <ka/typetraits.hpp>
#include <ka/unit.hpp>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
  template<typename I>
  using Category = typename std::iterator_traits<I>::iterator_category;

#define KA_EXPECT_TRUE_OR_RETURN(EXPR, ERRMSG) \
  if (!(EXPR)) return testing::AssertionFailure() << (ERRMSG)

#define KA_EXPECT_EQ_OR_RETURN(A, B, ERRMSG) \
  KA_EXPECT_TRUE_OR_RETURN((A) == (B), (ERRMSG))

  // Tests a type isomorphic to iterator unit type (unit type, iterator of unit
  // type and unit type, etc.).
  // `_1i` is isomorphic iterator unit. `_1` is isomorphic to unit.
  // InputIterator I, Regular T
  template<typename I, typename T>
  auto test_iter_unit(I _1i, T _1) -> testing::AssertionResult {
    testing::StaticAssertTypeEq<std::random_access_iterator_tag, Category<I>>();
    KA_EXPECT_EQ_OR_RETURN(_1i, _1i, "not equal");
    KA_EXPECT_EQ_OR_RETURN(_1, *_1i, "wrong dereferencing");
    KA_EXPECT_EQ_OR_RETURN(_1i, ++_1i, "wrong increment");
    KA_EXPECT_EQ_OR_RETURN(_1i, --_1i, "wrong decrement");
    KA_EXPECT_EQ_OR_RETURN(_1i, _1i + 67, "wrong forward jump");   // Arbitrary
    KA_EXPECT_EQ_OR_RETURN(_1i, _1i - 876, "wrong backward jump"); //  distances.
    KA_EXPECT_EQ_OR_RETURN(0, _1i - _1i, "wrong difference");
    KA_EXPECT_TRUE_OR_RETURN(ka::is_regular({_1i, _1i, _1i}), "not regular");
    return testing::AssertionSuccess();
  }

#undef KA_EXPECT_EQ_OR_RETURN
#undef KA_EXPECT_TRUE_OR_RETURN

  using ka::test::A;
  using ka::test::B;
  using ka::test::C;
}

// Tests of unit.
TEST(ProductIter, Zero) {
  using namespace ka;
  using iterator_cat_ops::operator*; // Product of iterators.
  using fn_cat_ops::operator*; // Product of values (tuple).
  auto _1i = unit_iter; // unit_iter == product_iter()
  auto _1 = unit; // unit == product()

  EXPECT_TRUE(test_iter_unit(_1i, _1));
  EXPECT_TRUE(test_iter_unit(_1i * _1i, _1 * _1));
  EXPECT_TRUE(test_iter_unit(_1i * _1i * _1i, _1 * _1 * _1));
  // Etc.
}

namespace ka { namespace test {
  using container_fn_types = testing::Types<
    array_fn_t,
    vector_fn_t, list_fn_t, deque_fn_t, forward_list_fn_t,
    set_fn_t, multiset_fn_t,
    unordered_set_fn_t, unordered_multiset_fn_t,
    map_fn_t, multimap_fn_t,
    unordered_map_fn_t, unordered_multimap_fn_t
  >;
}} // namespace ka::test

template<typename T>
struct ProductIterOne : testing::Test {
};

TYPED_TEST_SUITE(ProductIterOne, ka::test::container_fn_types);

// Tests of singleton iterator.
TYPED_TEST(ProductIterOne, Laws) {
  using namespace ka;
  auto ctor_fn = TypeParam{};
  auto c = test::fill_with<A>(ctor_fn);
  auto i = std::next(std::begin(c), 2);
  using I = Decay<decltype(i)>;
  static_assert(Equal<Category<I>, Category<product_iter_t<I>>>::value, "");
  EXPECT_EQ(product_iter(i), product_iter(i));
  EXPECT_EQ(product(*i), *product_iter(i));
  auto i0 = i;
  EXPECT_EQ(product_iter(++i0), ++product_iter(i));
}

namespace ka { namespace test {
  using bidirectional_container_fn_types = testing::Types<
    array_fn_t,
    vector_fn_t, list_fn_t, deque_fn_t,
    set_fn_t, multiset_fn_t,
    map_fn_t, multimap_fn_t
  >;
}} // namespace ka::test

template<typename T>
struct ProductIterOneBidirectional : testing::Test {
};

TYPED_TEST_SUITE(ProductIterOneBidirectional, ka::test::bidirectional_container_fn_types);

TYPED_TEST(ProductIterOneBidirectional, Laws) {
  using namespace ka;
  auto ctor_fn = TypeParam{};
  auto c = test::fill_with<A>(ctor_fn);
  auto i = std::next(std::begin(c), 2);
  auto i0 = i;
  EXPECT_EQ(product_iter(++i0), ++product_iter(i));
  i0 = i;
  EXPECT_EQ(product_iter(--i0), --product_iter(i));
}

namespace ka { namespace test {
  using random_access_container_fn_types = testing::Types<
    array_fn_t, vector_fn_t, deque_fn_t
  >;
}} // namespace ka::test

template<typename T>
struct ProductIterOneRandomAccess : testing::Test {
};

TYPED_TEST_SUITE(ProductIterOneRandomAccess, ka::test::random_access_container_fn_types);

TYPED_TEST(ProductIterOneRandomAccess, Laws) {
  using namespace ka;
  auto ctor_fn = TypeParam{};
  auto c = test::fill_with<A>(ctor_fn);
  auto n = 2;
  auto i = std::next(std::begin(c), n);
  EXPECT_TRUE(is_regular({product_iter(i), product_iter(i), product_iter(i)}));
  EXPECT_EQ(product_iter(i + n), product_iter(i) + n);
  EXPECT_EQ(product_iter(i - n), product_iter(i) - n);
  EXPECT_EQ((i + n) - (i - n), product_iter(i + n) - product_iter(i - n));
}

template<typename T>
struct ProductIterNRandomAccess : testing::Test {
};

namespace ka { namespace test {
  using random_access_container_triple_types = testing::Types<
    std::tuple<array_fn_t, vector_fn_t, deque_fn_t>,
    std::tuple<vector_fn_t, deque_fn_t, array_fn_t>,
    std::tuple<deque_fn_t, array_fn_t, vector_fn_t>
  >;
}} // namespace ka::test

TYPED_TEST_SUITE(ProductIterNRandomAccess, ka::test::random_access_container_triple_types);

TYPED_TEST(ProductIterNRandomAccess, Laws) {
  using namespace ka;
  using std::begin;
  // `c`, `d`, `e` are different containers of `A`.
  auto c = test::fill_with<A>(std::get<0>(TypeParam{}));
  auto d = test::fill_with<A>(std::get<1>(TypeParam{}));
  auto e = test::fill_with<A>(std::get<2>(TypeParam{}));
  auto i = std::next(begin(c), 2);
  auto j = std::next(begin(d), 2);
  auto k = std::next(begin(e), 2);
  EXPECT_EQ(product(*i, *j, *k), *product_iter(i, j, k));
  auto i0 = i; auto j0 = j; auto k0 = k;
  EXPECT_EQ(product_iter(++i0, ++j0, ++k0), ++product_iter(i, j, k));
  i0 = i; j0 = j; k0 = k;
  EXPECT_EQ(product_iter(--i0, --j0, --k0), --product_iter(i, j, k));
  EXPECT_EQ(product_iter(i+2, j+2, k+2), product_iter(i, j, k) + 2);
  EXPECT_EQ(product_iter(i-2, j-2, k-2), product_iter(i, j, k) - 2);
  auto const p = product_iter(i-2, j-2, k-2);
  auto const q = product_iter(i+2, j+2, k+2);
  EXPECT_EQ(std::distance(p, q), q - p);
}

namespace ka { namespace test {
  using container_triple_fn_types = testing::Types<
    std::tuple<vector_fn_t, map_fn_t, set_fn_t>,
    std::tuple<multiset_fn_t, list_fn_t, unordered_multimap_fn_t>,
    std::tuple<list_fn_t, multiset_fn_t, vector_fn_t>
  >;
}} // namespace ka::test

template<typename T>
struct ProductIterEnd : testing::Test {
};

TYPED_TEST_SUITE(ProductIterEnd, ka::test::container_triple_fn_types);

TYPED_TEST(ProductIterEnd, SameSize) {
  using namespace ka;
  using std::begin;
  using std::next;
  auto c = test::fill_with<A>(std::get<0>(TypeParam{}));
  auto d = test::fill_with<A>(std::get<1>(TypeParam{}));
  auto e = test::fill_with<A>(std::get<2>(TypeParam{}));
  EXPECT_EQ(product_iter(begin(c), begin(d), begin(e)), product_begin(c, d, e));
  EXPECT_EQ(product_iter(end(c), end(d), end(e)), product_end_same_size(c, d, e));
}

TYPED_TEST(ProductIterEnd, DifferentSize0) {
  using namespace ka;
  using std::begin;
  using std::next;
  auto c = test::fill_with<A>(std::get<0>(TypeParam{}));
  auto d = test::fill_with<A>(std::get<1>(TypeParam{}));
  auto e = test::fill_with<A>(std::get<2>(TypeParam{}));
  c.erase(begin(c), next(begin(c), 2));
  auto n = std::distance(begin(c), end(c));
  EXPECT_EQ(product_iter(begin(c), begin(d), begin(e)), product_begin(c, d, e));
  EXPECT_EQ(
    product_iter(next(begin(c), n), next(begin(d), n), next(begin(e), n)),
    product_end(c, d, e));
}

TYPED_TEST(ProductIterEnd, DifferentSize1) {
  using namespace ka;
  using std::begin;
  using std::next;
  auto c = test::fill_with<A>(std::get<0>(TypeParam{}));
  auto d = test::fill_with<A>(std::get<1>(TypeParam{}));
  auto e = test::fill_with<A>(std::get<2>(TypeParam{}));
  d.erase(begin(d), next(begin(d), 2));
  auto n = std::distance(begin(d), end(d));
  EXPECT_EQ(product_iter(begin(c), begin(d), begin(e)), product_begin(c, d, e));
  EXPECT_EQ(
    product_iter(next(begin(c), n), next(begin(d), n), next(begin(e), n)),
    product_end(c, d, e));
}

TYPED_TEST(ProductIterEnd, DifferentSize2) {
  using namespace ka;
  using std::begin;
  using std::next;
  auto c = test::fill_with<A>(std::get<0>(TypeParam{}));
  auto d = test::fill_with<A>(std::get<1>(TypeParam{}));
  auto e = test::fill_with<A>(std::get<2>(TypeParam{}));
  e.erase(begin(e), next(begin(e), 2));
  auto n = std::distance(begin(e), end(e));
  EXPECT_EQ(product_iter(begin(c), begin(d), begin(e)), product_begin(c, d, e));
  EXPECT_EQ(
    product_iter(next(begin(c), n), next(begin(d), n), next(begin(e), n)),
    product_end(c, d, e));
}

TEST(ProductIterEnd, DifferentSizeConcrete) {
  using namespace ka;
  A a[] = {A{12}, A{34}, A{647}};
  B b[] = {B{98}, B{733}};
  C c[] = {C{21}, C{43}, C{9163}};
  auto v = std::vector<A>{a[0], a[1], a[2]};
  auto l = std::list<B>{b[0], b[1]};
  auto m = std::multiset<C>{c[0], c[1], c[2]};
  auto i = product_begin(v, l, m);
  EXPECT_EQ(product(a[0], b[0], c[0]), *i);
  ++i;
  EXPECT_EQ(product(a[1], b[1], c[1]), *i);
  ++i;
  EXPECT_EQ(product_end(v, l, m), i);
}

TEST(ProductIterEnd, Mutate) {
  using namespace ka;
  A a[] = {A{12}, A{34}, A{647}};
  B b[] = {B{98}, B{733}};
  C c[] = {C{21}, C{43}, C{9163}};
  auto v = std::vector<A>{a[0], a[1], a[2]};
  auto l = std::list<B>{b[0], b[1]};
  auto f = std::forward_list<C>{c[0], c[1], c[2]};
  auto i = product_begin(v, l, f);
  EXPECT_EQ(product(a[0], b[0], c[0]), *i);
  a[0] = A{++a[0].value};
  b[0] = B{++b[0].value};
  c[0] = C{++c[0].value};
  *i = product(a[0], b[0], c[0]);
  EXPECT_EQ(product(a[0], b[0], c[0]), *i);
}

namespace {

#define KA_TEST_ITER_MOD_BASE(n)       \
    boost::iterator_facade<            \
      iter_mod_t<n>,                   \
      int,                             \
      std::random_access_iterator_tag, \
      int,                             \
      std::ptrdiff_t                   \
    >

  // Test iterator with circular orbit (0 -> 1 -> ... -> n-1 -> 0 -> ...).
  template<int n>
  struct iter_mod_t : KA_TEST_ITER_MOD_BASE(n) {
    using base = KA_TEST_ITER_MOD_BASE(n);
#undef KA_TEST_ITER_MOD_BASE
    using typename base::reference;
    using typename base::difference_type;
    int i;
  // Regular:
    explicit
    iter_mod_t(int i = 0) : i(i) {
    }
    bool equal(iter_mod_t x) const {
      return i == x.i;
    }
  // InputIterator:
    reference dereference() const {
      return i;
    }
    void increment() {
      i = (i + 1) % n;
    }
  // BidirectionalIterator:
    void decrement() {
      // `%` with negative numbers doesn't have the right behavior.
      i = (i == 0) ?  n - 1 : i - 1;
    }
  // RandomAccessIterator:
    /// Integral M
    template<typename M>
    void advance(M m) {
      if (m >= M(0)) {
        i = (i + m) % n;
      } else {
        m = -m % n; // ex: -(-5) % 3 = 5 % 3 = 2
        i = (i + (n - m)) % n;
      }
    }
    difference_type distance_to(iter_mod_t x) const {
      auto offset = (x.i > i) ? n : 0;
      return x.i - offset - i;
    }
  };
} // namespace

// Ensure first that our test iterator is correct.
TEST(ProductIterDistance, IterMod) {
  using I3 = iter_mod_t<3>;

  // Increment, decrement.
  {
    auto i = I3{0};
    EXPECT_EQ(I3{0}, i);
    EXPECT_EQ(I3{1}, ++i);
    EXPECT_EQ(I3{2}, ++i);
    EXPECT_EQ(I3{0}, ++i);

    EXPECT_EQ(I3{2}, --i);
    EXPECT_EQ(I3{1}, --i);
    EXPECT_EQ(I3{0}, --i);
    EXPECT_EQ(I3{2}, --i);
  }

  // Move forward.
  EXPECT_EQ(I3{0}, I3{0} + 0);
  EXPECT_EQ(I3{1}, I3{0} + 1);
  EXPECT_EQ(I3{2}, I3{0} + 2);
  EXPECT_EQ(I3{0}, I3{0} + 3);

  EXPECT_EQ(I3{1}, I3{2} + 5);  // 0 1 2 0 1
  EXPECT_EQ(I3{0}, I3{2} + 7);  // 0 1 2 0 1 2 0
  EXPECT_EQ(I3{0}, I3{2} + 13); // 0 1 2 0 1 2 0 1 2 0 1 2 0

  // Move backward.
  EXPECT_EQ(I3{0}, I3{0} - 3);
  EXPECT_EQ(I3{1}, I3{0} - 2);
  EXPECT_EQ(I3{2}, I3{0} - 1);
  EXPECT_EQ(I3{0}, I3{0} - 0);
  EXPECT_EQ(I3{0}, I3{2} - 5);  // 1 0 2 1 0
  EXPECT_EQ(I3{1}, I3{2} - 7);  // 1 0 2 1 0 2 1
  EXPECT_EQ(I3{1}, I3{2} - 13); // 1 0 2 1 0 2 1 0 2 1 0 2 1

  // Distance.
  EXPECT_EQ(0, I3{2} - I3{2});
  EXPECT_EQ(1, I3{2} - I3{1});
  EXPECT_EQ(2, I3{2} - I3{0});
  EXPECT_EQ(2, I3{1} - I3{2});
  EXPECT_EQ(0, I3{1} - I3{1});
  EXPECT_EQ(1, I3{1} - I3{0});
  EXPECT_EQ(1, I3{0} - I3{2});
  EXPECT_EQ(2, I3{0} - I3{1});
  EXPECT_EQ(0, I3{0} - I3{0});
}

// Tests distance for product of circular orbit-iterators.
// This case is very different from terminal orbits (an example of a terminal
// orbit is iterating through a standard container).
// In a circular orbit of size `n`, distance is at most `n-1`.
// In a product of two circular orbit-iterators of size `n` and `m`, the
// distance is at most `LCM(n, m) - 1` ('LCM' stands for 'least common multiple').
// For instance:
//   - with circular orbits of size `3` and `5`, the maximum distance is
//     `3*5 - 1 = 14`.
//   - with circular orbits of size `2` and `4`, the maximum distance is
//     `4 - 1 = 3`.
TEST(ProductIterDistance, CircularOrbit) {
  // Product of two circular orbit-iterators, respectively of size `3` and `5`.
  // Precondition: (0 <= i3 < 3) && (0 <= i5 < 5)
  auto p = [](int i3, int i5) {
    using ka::iterator_cat_ops::operator*; // Product of iterators.
    return iter_mod_t<3>{i3} * iter_mod_t<5>{i5};
  };
  EXPECT_EQ(p(2, 3), p(2, 3) + 0);
  EXPECT_EQ(p(0, 4), p(2, 3) + 1);
  EXPECT_EQ(p(1, 0), p(2, 3) + 2);
  EXPECT_EQ(p(2, 1), p(2, 3) + 3);

  EXPECT_EQ(0, p(0, 1) - p(0, 1));
  EXPECT_EQ(1, p(0, 1) - p(2, 0));
  EXPECT_EQ(2, p(0, 1) - p(1, 4));
  EXPECT_EQ(3, p(0, 1) - p(0, 3));
  EXPECT_EQ(4, p(0, 1) - p(2, 2));
  EXPECT_EQ(5, p(0, 1) - p(1, 1));
  EXPECT_EQ(6, p(0, 1) - p(0, 0));

  EXPECT_EQ( 8, p(0, 1) - p(1, 3)); // 2,4 0,0 1,1 2,2 0,3 1,4 2,0 0,1
  EXPECT_EQ( 9, p(0, 1) - p(0, 2));
  EXPECT_EQ(10, p(0, 1) - p(2, 1));
  EXPECT_EQ(11, p(0, 1) - p(1, 0));
  EXPECT_EQ(12, p(0, 1) - p(0, 4));
  EXPECT_EQ(13, p(0, 1) - p(2, 3));
  EXPECT_EQ(14, p(0, 1) - p(1, 2));
  EXPECT_EQ( 0, p(0, 1) - p(0, 1));
}

template<typename T>
struct ProductIterProjection : testing::Test {
};

TYPED_TEST_SUITE(ProductIterProjection, ka::test::container_triple_fn_types);

TYPED_TEST(ProductIterProjection, Basic) {
  using namespace ka;
  using std::begin;
  auto c = test::fill_with<A>(std::get<0>(TypeParam{}));
  auto d = test::fill_with<A>(std::get<1>(TypeParam{}));
  auto e = test::fill_with<A>(std::get<2>(TypeParam{}));
  auto i = begin(c);
  auto j = begin(d);
  auto k = begin(e);
  EXPECT_EQ(i, proj<0>(product_iter(i, j, k)));
  EXPECT_EQ(j, proj<1>(product_iter(i, j, k)));
  EXPECT_EQ(k, proj<2>(product_iter(i, j, k)));
}

TEST(ProductIter, Category) {
  using namespace ka;
  using testing::StaticAssertTypeEq;
  using V = std::vector<B>::iterator;
  using A = std::array<B, 3>::const_iterator;
  using L = std::list<B>::iterator;
  using S = std::set<B>::const_iterator;
  using F = std::forward_list<B>::iterator;
  using I = std::istream_iterator<B>;

  StaticAssertTypeEq<std::random_access_iterator_tag,
                     Category<product_iter_t<V>>>();

  StaticAssertTypeEq<std::random_access_iterator_tag,
                     Category<product_iter_t<V, A>>>();

  StaticAssertTypeEq<std::bidirectional_iterator_tag,
                     Category<product_iter_t<V, A, S>>>();

  StaticAssertTypeEq<std::bidirectional_iterator_tag,
                     Category<product_iter_t<V, A, S, L>>>();

  StaticAssertTypeEq<std::bidirectional_iterator_tag,
                     Category<product_iter_t<V, A, L, S>>>();

  StaticAssertTypeEq<std::bidirectional_iterator_tag,
                     Category<product_iter_t<V, L, A, S>>>();

  StaticAssertTypeEq<std::bidirectional_iterator_tag,
                     Category<product_iter_t<L, V, A, S>>>();

  StaticAssertTypeEq<std::forward_iterator_tag,
                     Category<product_iter_t<L, V, A, S, F>>>();

  StaticAssertTypeEq<std::forward_iterator_tag,
                     Category<product_iter_t<L, V, F, A, S>>>();

  StaticAssertTypeEq<std::input_iterator_tag,
                     Category<product_iter_t<L, V, F, A, S, I>>>();

  StaticAssertTypeEq<std::input_iterator_tag,
                     Category<product_iter_t<L, V, I, A, S>>>();

  StaticAssertTypeEq<std::input_iterator_tag,
                     Category<product_iter_t<I>>>();
}
