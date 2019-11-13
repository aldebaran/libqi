#include <utility>
#include <gtest/gtest.h>
#include <ka/functional.hpp>
#include <ka/integersequence.hpp>
#include <ka/moveoncopy.hpp>
#include <ka/testutils.hpp>

TEST(MoveOnCopy, Basic) {
  using namespace ka;
  using M = move_only_t<int>;
  const int i = 7635;
  move_on_copy_t<M> m0(M{i});
  move_on_copy_t<M> m1(m0);
  ASSERT_EQ(i, **m1);
}

TEST(MoveOnCopy, Lamdba) {
  using namespace ka;
  using M = move_only_t<int>;
  const int i = 7635;
  move_on_copy_t<M> m{M{i}};
  auto f = [=]() -> int {
    return **m;
  };
  ASSERT_EQ(i, f());
}

// Must also with copyable object.
TEST(MoveOnCopy, Copyable) {
  using namespace ka;
  const int i = 7635;
  move_on_copy_t<int> m{i};
  auto f = [=]() -> int {
    return *m;
  };
  ASSERT_EQ(i, f());
}

TEST(MoveOnCopy, Assignment) {
  using namespace ka;
  using M = move_only_t<int>;
  const int i = 7635;
  move_on_copy_t<M> m0(M{i});
  move_on_copy_t<M> m1;
  m1 = m0;
  ASSERT_EQ(i, **m1);
}

TEST(MoveOnCopy, MakeMoveOnCopy) {
  using namespace ka;
  using M = move_only_t<int>;
  const int i = 7635;
  auto m = move_on_copy(M{i});
  auto f = [=]() -> int {
    return **m;
  };
  ASSERT_EQ(i, f());
}

TEST(MoveOnCopy, RelationalOperators) {
  using namespace ka;
  const int i = 7635;
  move_on_copy_t<int> m0(i);
  move_on_copy_t<int> m1(i + 1);
  ASSERT_EQ(m0, m0);
  ASSERT_NE(m0, m1);
  ASSERT_LT(m0, m1);
  ASSERT_LE(m0, m1);
  ASSERT_LE(m0, m0);
  ASSERT_GT(m1, m0);
  ASSERT_GE(m1, m0);
  ASSERT_GE(m0, m0);
}

TEST(MoveOnCopy, ConstInstance) {
  using namespace ka;
  const int i = 7635;
  const move_on_copy_t<int> m(i);
  *m = *m + 1;
  ASSERT_EQ(i + 1, *m);
}

TEST(MoveOnCopy, OperatorStar) {
  using namespace ka;
  static_assert(std::is_same<decltype(*move_on_copy()), std::tuple<>&>::value, "");
  static_assert(std::is_same<decltype(*move_on_copy(5)), int&>::value, "");
  static_assert(std::is_same<decltype(*move_on_copy(5, 'a')), std::tuple<int, char>&>::value, "");
  static_assert(std::is_same<decltype(*move_on_copy(5, 'a', 3.14f)), std::tuple<int, char, float>&>::value, "");
}

TEST(MoveOnCopy, asTuple) {
  using namespace ka;
  static_assert(std::is_same<decltype(as_tuple(move_on_copy())), std::tuple<>&>::value, "");
  static_assert(std::is_same<decltype(as_tuple(move_on_copy(5))), std::tuple<int>&>::value, "");
  static_assert(std::is_same<decltype(as_tuple(move_on_copy(5, 'a'))), std::tuple<int, char>&>::value, "");
  static_assert(std::is_same<decltype(as_tuple(move_on_copy(5, 'a', 3.14f))), std::tuple<int, char, float>&>::value, "");
}

namespace test_moveoncopy {
  std::tuple<int, char, float> h(ka::move_only_t<int>&& i, ka::move_only_t<char>&& c, ka::move_only_t<float>&& f) {
    return std::make_tuple(*i, *c, *f);
  }

  std::tuple<> k() {
    return std::make_tuple();
  }

  std::tuple<int> l(int i) {
    return std::make_tuple(i);
  }
} // namespace test_moveoncopy

TEST(MoveOnCopy, MultipleArgumentsUnpack) {
  using namespace ka;
  using namespace test_moveoncopy;
  const int i = 5;
  const char c = 'a';
  const float f = 3.14f;
  auto m = move_on_copy(ka::move_only_t<int>(i), ka::move_only_t<char>(c), ka::move_only_t<float>(f));
  auto g = [=]() mutable {
    auto h2 = apply(h);
    return h2(std::move(*m));
  };
  ASSERT_EQ(g(), std::make_tuple(i, c, f));
}

TEST(MoveOnCopy, NoArgumentUnpack) {
  using namespace ka;
  using namespace test_moveoncopy;
  auto m = move_on_copy();
  auto g = [=]() mutable {
    auto k2 = apply(k);
    return k2(std::move(*m));
  };
  ASSERT_EQ(g(), std::make_tuple());
}

TEST(MoveOnCopy, OneArgumentUnpack) {
  using namespace ka;
  using namespace test_moveoncopy;
  const int i = 5;
  auto m = move_on_copy(i);
  auto g = [=]() mutable {
    auto l2 = apply(l);
    return l2(std::move(as_tuple(m)));
  };
  ASSERT_EQ(g(), std::make_tuple(i));
}
