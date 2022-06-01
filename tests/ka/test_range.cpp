#include <gtest/gtest.h>
#include <cstdint>
#include <ka/range.hpp>
#include <array>
#include <ka/conceptpredicate.hpp>
#include <ka/macroregular.hpp>
#include <ka/testutils.hpp>
#include <ka/ark/inputiter.hpp>

/// Archetype to ensure that only forward iterator's operations are used.
struct forward_iter_t {
  using self = forward_iter_t;
  int i;
  KA_GENERATE_FRIEND_REGULAR_OPS_1(self, i)
  self& operator++() {++i; return *this;}
};

TEST(Range, BoundedRangeBasic) {
  using namespace ka;
  using I = forward_iter_t;
  bounded_range_t<I> rng{I{3}, I{5}};
  EXPECT_FALSE(is_empty(rng)); // [3, 4]
  pop(rng);
  EXPECT_FALSE(is_empty(rng)); // [4]
  pop(rng);
  EXPECT_TRUE(is_empty(rng));  // []
}

struct readable_forward_iter_t {
  using self = readable_forward_iter_t;
  int i;
  KA_GENERATE_FRIEND_REGULAR_OPS_1(self, i)
  const int& operator*() const {return i;}
  self& operator++() {++i; return *this;}
};

TEST(Range, BoundedRangeReadable) {
  using namespace ka;
  using I = readable_forward_iter_t;
  bounded_range_t<I> rng{I{3}, I{5}};
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(3, front(rng));
  pop(rng);
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_TRUE(is_empty(rng));
}

TEST(Range, BoundedRangeHelperFunctionIterators) {
  using namespace ka;
  std::array<int, 2> a {{4, 1}};
  auto rng = bounded_range(begin(a), end(a));
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(1, front(rng));
  pop(rng);
  EXPECT_TRUE(is_empty(rng));
}

TEST(Range, BoundedRangeHelperFunctionSequence) {
  using namespace ka;
  std::array<int, 2> a {{4, 1}};
  auto rng = bounded_range(a);
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(1, front(rng));
  pop(rng);
  EXPECT_TRUE(is_empty(rng));
}

TEST(Range, BoundedRangeHelperFunctionSequenceIncr) {
  using namespace ka;
  using functional_ops_accu::operator*;
  using C = std::array<int, 2>;
  C a {{4, 1}};
  incr_t incr;
  auto rng = bounded_range(a, incr * incr); // skip 'odd' iterators
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_TRUE(is_empty(rng));
}

TEST(Range, BoundedRangeHelperFunctionSequenceArrayOfStd) {
  using namespace ka;
  using P = std::pair<int, int>;
  P a[2] = {{4, 3}, {1, 5}};
  auto rng = bounded_range(a);
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ((P{4, 3}), front(rng));
  pop(rng);
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ((P{1, 5}), front(rng));
  pop(rng);
  EXPECT_TRUE(is_empty(rng));
}

// Iterator I
template<typename I>
struct pair_iter_t {
  I b, e;
  friend I begin(const pair_iter_t& x) {
    return x.b;
  }
  friend I end(const pair_iter_t& x) {
    return x.e;
  }
};

TEST(Range, BoundedRangeHelperFunctionSequenceCustom) {
  using namespace ka;
  std::array<int, 2> a {{4, 1}};
  pair_iter_t<decltype(begin(a))> p{begin(a), end(a)};
  auto rng = bounded_range(p);
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(1, front(rng));
  pop(rng);
  EXPECT_TRUE(is_empty(rng));
}

// Iterator I
template<typename I>
struct pair_iter_member_t {
  I b, e;
  I begin() const {
    return b;
  }
  I end() const {
    return e;
  }
};

TEST(Range, BoundedRangeHelperFunctionSequenceCustomMember) {
  using namespace ka;
  std::array<int, 2> a {{4, 1}};
  pair_iter_member_t<decltype(begin(a))> p{begin(a), end(a)};
  auto rng = bounded_range(p);
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_FALSE(is_empty(rng));
  EXPECT_EQ(1, front(rng));
  pop(rng);
  EXPECT_TRUE(is_empty(rng));
}

TEST(Range, BoundedRangeRegular) {
  using namespace ka;
  using I = readable_forward_iter_t;
  using R = bounded_range_t<I>;
  EXPECT_TRUE(is_regular({R{I{0}, I{3}}, R{I{1}, I{4}}, R{I{2}, I{3}}}));
}

struct mutable_forward_iter_t {
  using self = mutable_forward_iter_t;
  int i;
  KA_GENERATE_FRIEND_REGULAR_OPS_1(self, i)
  int& operator*() {return i;}
  const int& operator*() const {return i;}
  self& operator++() {++i; return *this;}
};

TEST(Range, BoundedRangeMutable) {
  using namespace ka;
  using I = mutable_forward_iter_t;
  bounded_range_t<I> rng{I{3}, I{5}};
  EXPECT_FALSE(is_empty(rng)); // [3, 4]
  EXPECT_EQ(3, front(rng));
  front(rng) = 18;
  EXPECT_EQ(18, front(rng));  // [18, 4]
}

struct incrementable_t {
  using self = incrementable_t;
  int i = 0;
  incrementable_t(int i) : i(i) {
  }
  incrementable_t() = default;
  KA_GENERATE_FRIEND_REGULAR_OPS_1(self, i)
  void operator++() {
    ++i;
  }
};

namespace std {
  template<>
  struct numeric_limits<incrementable_t> {
    static int min() {return std::numeric_limits<int>::min();}
    static int max() {return std::numeric_limits<int>::max();}
  };
}

TEST(Range, IncrBoundedRangeBasic) {
  using namespace ka;
  using N = incrementable_t;
  bounded_range_t<N> rng{N{3}, N{5}};
  EXPECT_FALSE(is_empty(rng)); // [3, 4]
  EXPECT_EQ(N{3}, front(rng));
  front(rng) = N{1};
  EXPECT_EQ(N{1}, front(rng));  // [1, 2, 3, 4]
  pop(rng);
  EXPECT_EQ(N{2}, front(rng));  // [2, 3, 4]
}

TEST(Range, IncrBoundedRangeRegular) {
  using namespace ka;
  using N = incrementable_t;
  using R = bounded_range_t<N>;
  EXPECT_TRUE(is_regular({R{N{0}, N{3}}, R{N{1}, N{4}}, R{N{2}, N{3}}}));
}

TEST(Range, IncrBoundedRangeHelper) {
  using namespace ka;
  using N = incrementable_t;
  auto rng = bounded_range(N{3}, N{7});
  EXPECT_FALSE(is_empty(rng)); // [3, 4, 5, 6]
  EXPECT_EQ(N{3}, front(rng));
  front(rng) = N{1};
  EXPECT_EQ(N{1}, front(rng));  // [1, 2, 3, 4, 5, 6]
  pop(rng);
  EXPECT_EQ(N{2}, front(rng));  // [2, 3, 4, 5, 6]
}

TEST(Range, IncrBoundedRangeHelperTerse) {
  using namespace ka;
  using N = incrementable_t;
  auto rng = bounded_range(N{0}, N{4});
  EXPECT_FALSE(is_empty(rng)); // [0, 1, 2, 3]
  EXPECT_EQ(N{0}, front(rng));
  front(rng) = N{2};
  EXPECT_EQ(N{2}, front(rng));  // [2, 3]
  pop(rng);
  EXPECT_EQ(N{3}, front(rng));  // [3]
}

TEST(Range, IncrBoundedRangeHelperSuperTerse) {
  using namespace ka;
  using N = incrementable_t;
  auto rng = bounded_range<N>();
  EXPECT_FALSE(is_empty(rng)); // [std::numeric_limits<int>::min(), ..., std::numeric_limits<int>::max()]
  EXPECT_EQ(std::numeric_limits<int>::min(), front(rng));
  front(rng) = N{2};
  EXPECT_EQ(N{2}, front(rng));  // [2, ..., std::numeric_limits<int>::max()]
  pop(rng);
  EXPECT_EQ(N{3}, front(rng));  // [3, ..., std::numeric_limits<int>::max()]
}

struct incr_twice_t {
  using self = incr_twice_t;
  KA_GENERATE_FRIEND_REGULAR_OPS_0(self)
  template<typename N> // Incrementable N
  void operator()(N& n) const {
    ++n;
    ++n;
  }
};

TEST(Range, IncrBoundedRangeActionBasic) {
  using namespace ka;
  using N = incrementable_t;
  using R = bounded_range_t<N, incr_twice_t>;
  incr_twice_t a;
  R rng{N{3}, N{9}, a};
  EXPECT_FALSE(is_empty(rng)); // [3, 5, 7]
  EXPECT_EQ(N{3}, front(rng));
  front(rng) = N{-1};
  EXPECT_EQ(N{-1}, front(rng));  // [-1, 1, 3, 5, 7]
  pop(rng);
  EXPECT_EQ(N{1}, front(rng));  // [1, 3, 5, 7]
}

TEST(Range, IncrBoundedRangeActionRegular) {
  using namespace ka;
  using N = incrementable_t;
  using R = bounded_range_t<N, incr_twice_t>;
  incr_twice_t a;
  EXPECT_TRUE(is_regular({R{N{0}, N{4}, a}, R{N{1}, N{7}, a}, R{N{2}, N{2}, a}}));
}

TEST(Range, IncrBoundedRangeActionHelper) {
  using namespace ka;
  using N = incrementable_t;
  auto rng = bounded_range(N{3}, N{9}, incr_twice_t{});
  EXPECT_FALSE(is_empty(rng)); // [3, 5, 7]
  EXPECT_EQ(N{3}, front(rng));
  front(rng) = N{-1};
  EXPECT_EQ(N{-1}, front(rng));  // [-1, 1, 3, 5, 7]
  pop(rng);
  EXPECT_EQ(N{1}, front(rng));  // [1, 3, 5, 7]
}

TEST(Range, IncrBoundedRangeActionHelperTerse) {
  using namespace ka;
  using N = incrementable_t;
  auto rng = bounded_range(N{0}, N{6}, incr_twice_t{});
  EXPECT_FALSE(is_empty(rng)); // [0, 2, 4]
  EXPECT_EQ(N{0}, front(rng));
  front(rng) = N{-2};
  EXPECT_EQ(N{-2}, front(rng));  // [-2, 0, 2, 4]
  pop(rng);
  EXPECT_EQ(N{0}, front(rng));  // [0, 2, 4, 6]
}

struct RangeBoundedRangeLinearizableBeginEnd
  : testing::TestWithParam<std::vector<ka::test::A>> {
};

namespace detail {
  using std::vector;
  using ka::test::A;

  auto boundedRangeLinearizableBeginEndTestingValues =
    testing::Values(
      vector<A>{},
      vector<A>{A{62}},
      vector<A>{A{62}, A{926}},
      vector<A>{A{62}, A{926}, A{-3}},
      vector<A>{A{62}, A{926}, A{-3}, A{0}, A{52720}}
    );
} // namespace detail

INSTANTIATE_TEST_SUITE_P(
  DefaultInstantiation,
  RangeBoundedRangeLinearizableBeginEnd,
  detail::boundedRangeLinearizableBeginEndTestingValues
);

TEST_P(RangeBoundedRangeLinearizableBeginEnd, Basic) {
  using namespace ka;
  using test::A;
  auto const values = GetParam();
  auto b = ark::input_iter(values.begin());
  auto e = ark::input_iter(values.end());
  auto const rng = bounded_range(b, e);
  EXPECT_EQ(b, begin(rng));
  EXPECT_EQ(e, end(rng));
  EXPECT_EQ(values.empty(), empty(rng));
  EXPECT_EQ(values.size(), static_cast<std::size_t>(size(rng)));
  for (auto i = 0; i != size(rng); ++i) {
    EXPECT_EQ(values[i], rng[i]);
  }
}

TEST_P(RangeBoundedRangeLinearizableBeginEnd, Mutable) {
  using namespace ka;
  using test::A;
  auto values = GetParam(); // Copy.
  auto b = ark::input_iter(values.begin());
  auto e = ark::input_iter(values.end());
  auto rng = bounded_range(b, e);
  auto succ = [](A a) -> A {
    return A{a.value + 1};
  };
  // Mutates values through the range.
  for (auto i = 0; i != size(rng); ++i) {
    rng[i] = succ(rng[i]);
  }
  // Checks the values against the original ones.
  for (auto i = 0; i != size(rng); ++i) {
    EXPECT_EQ(succ(GetParam()[i]), values[i]);
  }
}

TEST(Range, RepeatRangeBasic) {
  using namespace ka;
  using N = incrementable_t;
  using R = repeat_range_t<int, N>;
  R rng{9, N{0}, N{2}};
  EXPECT_FALSE(is_empty(rng)); // [9, 9]
  EXPECT_EQ(9, front(rng));
  pop(rng);
  EXPECT_EQ(9, front(rng));  // [9]
  pop(rng);
  EXPECT_TRUE(is_empty(rng));
}

TEST(Range, RepeatRangeRegular) {
  using namespace ka;
  using N = incrementable_t;
  using R = repeat_range_t<int, N>;
  EXPECT_TRUE(is_regular({R{7, N{0}, N{4}}, R{8, N{1}, N{7}}, R{9, N{2}, N{2}}}));
}

TEST(Range, RepeatRangeHelper) {
  using namespace ka;
  using N = incrementable_t;
  auto rng = repeat_range(9, N{2});
  EXPECT_FALSE(is_empty(rng)); // [9, 9]
  EXPECT_EQ(9, front(rng));
  pop(rng);
  EXPECT_EQ(9, front(rng));  // [9]
  pop(rng);
  EXPECT_TRUE(is_empty(rng));
}
