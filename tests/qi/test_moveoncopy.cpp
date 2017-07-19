#include <gtest/gtest.h>
#include <qi/moveoncopy.hpp>
#include "tools.hpp"

TEST(MoveOnCopy, Basic)
{
  using namespace qi;
  using M = test::MoveOnly<int>;
  const int i = 7635;
  MoveOnCopy<M> m0(M{i});
  MoveOnCopy<M> m1(m0);
  ASSERT_EQ(i, **m1);
}

TEST(MoveOnCopy, Lamdba)
{
  using namespace qi;
  using M = test::MoveOnly<int>;
  const int i = 7635;
  MoveOnCopy<M> m{M{i}};
  auto f = [=]() -> int {
    return **m;
  };
  ASSERT_EQ(i, f());
}

// Must also with copyable object.
TEST(MoveOnCopy, Copyable)
{
  using namespace qi;
  const int i = 7635;
  MoveOnCopy<int> m{i};
  auto f = [=]() -> int {
    return *m;
  };
  ASSERT_EQ(i, f());
}

TEST(MoveOnCopy, Assignment)
{
  using namespace qi;
  using M = test::MoveOnly<int>;
  const int i = 7635;
  MoveOnCopy<M> m0(M{i});
  MoveOnCopy<M> m1;
  m1 = m0;
  ASSERT_EQ(i, **m1);
}

TEST(MoveOnCopy, MakeMoveOnCopy)
{
  using namespace qi;
  using M = test::MoveOnly<int>;
  const int i = 7635;
  auto m = makeMoveOnCopy(M{i});
  auto f = [=]() -> int {
    return **m;
  };
  ASSERT_EQ(i, f());
}

TEST(MoveOnCopy, RelationalOperators)
{
  using namespace qi;
  const int i = 7635;
  MoveOnCopy<int> m0(i);
  MoveOnCopy<int> m1(i + 1);
  ASSERT_EQ(m0, m0);
  ASSERT_NE(m0, m1);
  ASSERT_LT(m0, m1);
  ASSERT_LE(m0, m1);
  ASSERT_LE(m0, m0);
  ASSERT_GT(m1, m0);
  ASSERT_GE(m1, m0);
  ASSERT_GE(m0, m0);
}

TEST(MoveOnCopy, ConstInstance)
{
  using namespace qi;
  const int i = 7635;
  const MoveOnCopy<int> m(i);
  *m = *m + 1;
  ASSERT_EQ(i + 1, *m);
}
