#include <utility>
#include <gtest/gtest.h>
#include <qi/functional.hpp>
#include <qi/type/integersequence.hpp>
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

TEST(MoveOnCopy, OperatorStar)
{
  using namespace qi;
  static_assert(std::is_same<decltype(*makeMoveOnCopy()), std::tuple<>&>::value, "");
  static_assert(std::is_same<decltype(*makeMoveOnCopy(5)), int&>::value, "");
  static_assert(std::is_same<decltype(*makeMoveOnCopy(5, 'a')), std::tuple<int, char>&>::value, "");
  static_assert(std::is_same<decltype(*makeMoveOnCopy(5, 'a', 3.14f)), std::tuple<int, char, float>&>::value, "");
}

TEST(MoveOnCopy, asTuple)
{
  using namespace qi;
  static_assert(std::is_same<decltype(asTuple(makeMoveOnCopy())), std::tuple<>&>::value, "");
  static_assert(std::is_same<decltype(asTuple(makeMoveOnCopy(5))), std::tuple<int>&>::value, "");
  static_assert(std::is_same<decltype(asTuple(makeMoveOnCopy(5, 'a'))), std::tuple<int, char>&>::value, "");
  static_assert(std::is_same<decltype(asTuple(makeMoveOnCopy(5, 'a', 3.14f))), std::tuple<int, char, float>&>::value, "");
}

namespace test
{
  std::tuple<int, char, float> h(MoveOnly<int>&& i, MoveOnly<char>&& c, MoveOnly<float>&& f)
  {
    return std::make_tuple(*i, *c, *f);
  }

  std::tuple<> k()
  {
    return std::make_tuple();
  }

  std::tuple<int> l(int i)
  {
    return std::make_tuple(i);
  }
} // namespace test

TEST(MoveOnCopy, MultipleArgumentsUnpack)
{
  using namespace qi;
  using namespace test;
  const int i = 5;
  const char c = 'a';
  const float f = 3.14f;
  auto m = makeMoveOnCopy(MoveOnly<int>(i), MoveOnly<char>(c), MoveOnly<float>(f));
  auto g = [=]() mutable {
    auto h2 = apply(h);
    return h2(std::move(*m));
  };
  ASSERT_EQ(g(), std::make_tuple(i, c, f));
}

TEST(MoveOnCopy, NoArgumentUnpack)
{
  using namespace qi;
  using namespace test;
  auto m = makeMoveOnCopy();
  auto g = [=]() mutable {
    auto k2 = apply(k);
    return k2(std::move(*m));
  };
  ASSERT_EQ(g(), std::make_tuple());
}

TEST(MoveOnCopy, OneArgumentUnpack)
{
  using namespace qi;
  using namespace test;
  const int i = 5;
  auto m = makeMoveOnCopy(i);
  auto g = [=]() mutable {
    auto l2 = apply(l);
    return l2(std::move(asTuple(m)));
  };
  ASSERT_EQ(g(), std::make_tuple(i));
}
