/*
**  Copyright (C) 2017 SoftBank Robotics Europe
**  See COPYING for the license
*/

#include <memory>
#include <numeric>
#include <gtest/gtest.h>
#include <ka/ark/mutable.hpp>
#include <ka/conceptpredicate.hpp>
#include <ka/range.hpp>
#include <ka/src.hpp>

TEST(ArkMutable, Basic) {
  using namespace ka;
  int const i = 5;
  int j = i;
  ark_mutable_t<int*> p{&j};
  EXPECT_EQ(i, src(p));
  auto q = p;
  ++src(p);
  EXPECT_EQ(i + 1, src(p));
  EXPECT_EQ(i + 1, src(q));
}

TEST(ArkMutable, SharedPtr) {
  using namespace ka;
  int const i = 5;
  ark_mutable_t<std::shared_ptr<int>> p{std::make_shared<int>(i)};
  EXPECT_EQ(i, src(p));
  ++src(p);
  EXPECT_EQ(i + 1, src(p));
}

TEST(ArkMutable, Integral) {
  using namespace ka;
  int const i = 5;
  ark_mutable_t<int> x{i};
  EXPECT_EQ(i, src(x));
  auto y = x;
  ++src(x);
  EXPECT_EQ(i + 1, src(x));
  EXPECT_EQ(i, src(y));
}

TEST(ArkMutable, RegularPointer) {
  using namespace ka;
  using M = ark_mutable_t<int*>;

  std::size_t const N = 100;
  int a[N];
  std::iota(std::begin(a), std::end(a), 0);
  ASSERT_TRUE(is_regular(bounded_range(M{a + 0}, M{a + N},
    [&](M& x) {
      x = M{a + src(x) + 1};
    })
  ));
}

TEST(ArkMutable, RegularIntegral) {
  using namespace ka;
  using M = ark_mutable_t<int>;
  ASSERT_TRUE(is_regular(bounded_range(M{0}, M{100},
    [&](M& x) {
      x = M{src(x) + 1};
    })
  ));
}
