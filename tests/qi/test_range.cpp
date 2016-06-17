#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <cstdint>
#include <qi/range.hpp>
#include <array>
#include <qi/detail/conceptpredicate.hpp>
#include <qi/macroregular.hpp>

/// Archetype to ensure that only forward iterator's operations are used.
struct ForwardIter
{
  using Self = ForwardIter;
  int i;
  QI_GENERATE_FRIEND_REGULAR_OPS_1(Self, i)
  Self& operator++() {++i; return *this;}
};

TEST(Range, BoundedRangeBasic)
{
  using namespace qi;
  using I = ForwardIter;
  BoundedRange<I> rng{I{3}, I{5}};
  EXPECT_FALSE(isEmpty(rng)); // [3, 4]
  pop(rng);
  EXPECT_FALSE(isEmpty(rng)); // [4]
  pop(rng);
  EXPECT_TRUE(isEmpty(rng));  // []
}

struct ReadableForwardIter
{
  using Self = ReadableForwardIter;
  int i;
  QI_GENERATE_FRIEND_REGULAR_OPS_1(Self, i)
  const int& operator*() const {return i;}
  Self& operator++() {++i; return *this;}
};

TEST(Range, BoundedRangeReadable)
{
  using namespace qi;
  using I = ReadableForwardIter;
  BoundedRange<I> rng{I{3}, I{5}};
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ(3, front(rng));
  pop(rng);
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_TRUE(isEmpty(rng));
}

TEST(Range, BoundedRangeHelperFunctionIterators)
{
  using namespace qi;
  std::array<int, 2> a{4, 1};
  auto rng = boundedRange(begin(a), end(a));
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ(1, front(rng));
  pop(rng);
  EXPECT_TRUE(isEmpty(rng));
}

TEST(Range, BoundedRangeHelperFunctionSequence)
{
  using namespace qi;
  std::array<int, 2> a{4, 1};
  auto rng = boundedRange(a);
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ(1, front(rng));
  pop(rng);
  EXPECT_TRUE(isEmpty(rng));
}

TEST(Range, BoundedRangeHelperFunctionSequenceArrayOfStd)
{
  using namespace qi;
  using P = std::pair<int, int>;
  P a[2] = {{4, 3}, {1, 5}};
  auto rng = boundedRange(a);
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ((P{4, 3}), front(rng));
  pop(rng);
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ((P{1, 5}), front(rng));
  pop(rng);
  EXPECT_TRUE(isEmpty(rng));
}

// Iterator I
template<typename I>
struct PairIter {
  I b, e;
  friend I begin(const PairIter& x)
  {
    return x.b;
  }
  friend I end(const PairIter& x)
  {
    return x.e;
  }
};

TEST(Range, BoundedRangeHelperFunctionSequenceCustom)
{
  using namespace qi;
  std::array<int, 2> a{4, 1};
  PairIter<decltype(begin(a))> p{begin(a), end(a)};
  auto rng = boundedRange(p);
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ(1, front(rng));
  pop(rng);
  EXPECT_TRUE(isEmpty(rng));
}

// Iterator I
template<typename I>
struct PairIterMember {
  I b, e;
  I begin() const
  {
    return b;
  }
  I end() const
  {
    return e;
  }
};

TEST(Range, BoundedRangeHelperFunctionSequenceCustomMember)
{
  using namespace qi;
  std::array<int, 2> a{4, 1};
  PairIterMember<decltype(begin(a))> p{begin(a), end(a)};
  auto rng = boundedRange(p);
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ(4, front(rng));
  pop(rng);
  EXPECT_FALSE(isEmpty(rng));
  EXPECT_EQ(1, front(rng));
  pop(rng);
  EXPECT_TRUE(isEmpty(rng));
}

TEST(Range, BoundedRangeRegular)
{
  using namespace qi;
  using namespace qi::detail;
  using I = ReadableForwardIter;
  using R = BoundedRange<I>;
  EXPECT_TRUE(isRegular({R{I{0}, I{3}}, R{I{1}, I{4}}, R{I{2}, I{3}}}));
}

struct MutableForwardIter
{
  using Self = MutableForwardIter;
  int i;
  QI_GENERATE_FRIEND_REGULAR_OPS_1(Self, i)
  int& operator*() {return i;}
  const int& operator*() const {return i;}
  Self& operator++() {++i; return *this;}
};

TEST(Range, BoundedRangeMutable)
{
  using namespace qi;
  using I = MutableForwardIter;
  BoundedRange<I> rng{I{3}, I{5}};
  EXPECT_FALSE(isEmpty(rng)); // [3, 4]
  EXPECT_EQ(3, front(rng));
  front(rng) = 18;
  EXPECT_EQ(18, front(rng));  // [18, 4]
}

struct Incrementable
{
  using Self = Incrementable;
  int i = 0;
  Incrementable(int i) : i(i)
  {
  }
  Incrementable() = default;
  QI_GENERATE_FRIEND_REGULAR_OPS_1(Self, i)
  void operator++()
  {
    ++i;
  }
};

namespace std
{
  template<>
  struct numeric_limits<Incrementable>
  {
    static int min() {return std::numeric_limits<int>::min();}
    static int max() {return std::numeric_limits<int>::max();}
  };
}

TEST(Range, IncrBoundedRangeBasic)
{
  using namespace qi;
  using N = Incrementable;
  using R = IncrBoundedRange<N>;
  R rng{N{3}, N{5}};
  EXPECT_FALSE(isEmpty(rng)); // [3, 4]
  EXPECT_EQ(N{3}, front(rng));
  front(rng) = N{1};
  EXPECT_EQ(N{1}, front(rng));  // [1, 2, 3, 4]
  pop(rng);
  EXPECT_EQ(N{2}, front(rng));  // [2, 3, 4]
}

TEST(Range, IncrBoundedRangRegular)
{
  using namespace qi;
  using namespace qi::detail;
  using N = Incrementable;
  using R = IncrBoundedRange<N>;
  EXPECT_TRUE(isRegular({R{N{0}, N{3}}, R{N{1}, N{4}}, R{N{2}, N{3}}}));
}

TEST(Range, IncrBoundedRangeHelper)
{
  using namespace qi;
  using N = Incrementable;
  auto rng = incrRange(N{3}, N{7});
  EXPECT_FALSE(isEmpty(rng)); // [3, 4, 5, 6]
  EXPECT_EQ(N{3}, front(rng));
  front(rng) = N{1};
  EXPECT_EQ(N{1}, front(rng));  // [1, 2, 3, 4, 5, 6]
  pop(rng);
  EXPECT_EQ(N{2}, front(rng));  // [2, 3, 4, 5, 6]
}

TEST(Range, IncrBoundedRangeHelperTerse)
{
  using namespace qi;
  using N = Incrementable;
  auto rng = incrRange(N{4});
  EXPECT_FALSE(isEmpty(rng)); // [0, 1, 2, 3]
  EXPECT_EQ(N{0}, front(rng));
  front(rng) = N{2};
  EXPECT_EQ(N{2}, front(rng));  // [2, 3]
  pop(rng);
  EXPECT_EQ(N{3}, front(rng));  // [3]
}

TEST(Range, IncrBoundedRangeHelperSuperTerse)
{
  using namespace qi;
  using N = Incrementable;
  auto rng = incrRange<N>();
  EXPECT_FALSE(isEmpty(rng)); // [std::numeric_limits<int>::min(), ..., std::numeric_limits<int>::max()]
  EXPECT_EQ(std::numeric_limits<int>::min(), front(rng));
  front(rng) = N{2};
  EXPECT_EQ(N{2}, front(rng));  // [2, ..., std::numeric_limits<int>::max()]
  pop(rng);
  EXPECT_EQ(N{3}, front(rng));  // [3, ..., std::numeric_limits<int>::max()]
}

struct IncrTwice
{
  using Self = IncrTwice;
  QI_GENERATE_FRIEND_REGULAR_OPS_0(Self)
  template<typename N> // Incrementable N
  void operator()(N& n) const
  {
    ++n;
    ++n;
  }
};

TEST(Range, IncrBoundedRangeActionBasic)
{
  using namespace qi;
  using N = Incrementable;
  using R = IncrBoundedRangeAction<N, IncrTwice>;
  IncrTwice a;
  R rng{N{3}, N{9}, a};
  EXPECT_FALSE(isEmpty(rng)); // [3, 5, 7]
  EXPECT_EQ(N{3}, front(rng));
  front(rng) = N{-1};
  EXPECT_EQ(N{-1}, front(rng));  // [-1, 1, 3, 5, 7]
  pop(rng);
  EXPECT_EQ(N{1}, front(rng));  // [1, 3, 5, 7]
}

TEST(Range, IncrBoundedRangeActionRegular)
{
  using namespace qi;
  using namespace qi::detail;
  using N = Incrementable;
  using R = IncrBoundedRangeAction<N, IncrTwice>;
  IncrTwice a;
  EXPECT_TRUE(isRegular({R{N{0}, N{4}, a}, R{N{1}, N{7}, a}, R{N{2}, N{2}, a}}));
}

TEST(Range, IncrBoundedRangeActionHelper)
{
  using namespace qi;
  using N = Incrementable;
  auto rng = incrRange(N{3}, N{9}, IncrTwice{});
  EXPECT_FALSE(isEmpty(rng)); // [3, 5, 7]
  EXPECT_EQ(N{3}, front(rng));
  front(rng) = N{-1};
  EXPECT_EQ(N{-1}, front(rng));  // [-1, 1, 3, 5, 7]
  pop(rng);
  EXPECT_EQ(N{1}, front(rng));  // [1, 3, 5, 7]
}

TEST(Range, IncrBoundedRangeActionHelperTerse)
{
  using namespace qi;
  using N = Incrementable;
  auto rng = incrRange(N{6}, IncrTwice{});
  EXPECT_FALSE(isEmpty(rng)); // [0, 2, 4]
  EXPECT_EQ(N{0}, front(rng));
  front(rng) = N{-2};
  EXPECT_EQ(N{-2}, front(rng));  // [-2, 0, 2, 4]
  pop(rng);
  EXPECT_EQ(N{0}, front(rng));  // [0, 2, 4, 6]
}

TEST(Range, RepeatRangeBasic)
{
  using namespace qi;
  using N = Incrementable;
  using R = RepeatRange<int, N>;
  R rng{9, N{0}, N{2}};
  EXPECT_FALSE(isEmpty(rng)); // [9, 9]
  EXPECT_EQ(9, front(rng));
  pop(rng);
  EXPECT_EQ(9, front(rng));  // [9]
  pop(rng);
  EXPECT_TRUE(isEmpty(rng));
}

TEST(Range, RepeatRangeRegular)
{
  using namespace qi;
  using namespace qi::detail;
  using N = Incrementable;
  using R = RepeatRange<int, N>;
  EXPECT_TRUE(isRegular({R{7, N{0}, N{4}}, R{8, N{1}, N{7}}, R{9, N{2}, N{2}}}));
}

TEST(Range, RepeatRangeHelper)
{
  using namespace qi;
  using N = Incrementable;
  auto rng = repeatRange(9, N{2});
  EXPECT_FALSE(isEmpty(rng)); // [9, 9]
  EXPECT_EQ(9, front(rng));
  pop(rng);
  EXPECT_EQ(9, front(rng));  // [9]
  pop(rng);
  EXPECT_TRUE(isEmpty(rng));
}
