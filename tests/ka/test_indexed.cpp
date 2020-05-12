#include <ka/indexed.hpp>
#include <ka/testutils.hpp>
#include <ka/conceptpredicate.hpp>
#include <gtest/gtest.h>
#include <tuple>
#include <boost/variant.hpp>

using ka::test::A;

TEST(Indexed, IsRegular) {
  using namespace ka;
  using I = indexed_t<42, A>;
  EXPECT_TRUE(is_regular({ I{ A{0} }, I{ A{298} }, I{ A{348} }, I{ A{9423} }, I{
    A{4093283} }, I{ A{-2387} } }));
}

TEST(Indexed, Readable) {
  using namespace ka;
  auto const i = indexed<12>(A{42});
  EXPECT_EQ(A{42}, src(i));
}

TEST(Indexed, Mutable) {
  using namespace ka;

  // l-value
  auto i = indexed<12>(A{42});
  src(i) = A{489};
  EXPECT_EQ(A{489}, src(i));

  // r-value
  auto const m = src(indexed<12>(move_only_t<A>{A{77}}));
  EXPECT_EQ(A{77}, src(m));
}

TEST(Indexed, IndexStaticValue) {
  using namespace ka;
  static_assert(indexed_t<12, A>::index == 12, "");
}

TEST(Indexed, MoveOnlyType) {
  using namespace ka;
  auto i0 = indexed_t<0, move_only_t<A>>{ move_only_t<A>{ A{42} }};
  EXPECT_EQ(A{42}, src(src(i0)));

  auto const i1 = id_transfo_t{}(mv(i0));
  EXPECT_EQ(A{42}, src(src(i1)));
}

namespace {
  template<typename... T>
  struct variadic_type;

  struct ApplyIndexed : testing::Test {
    template<template<typename...> class T> constexpr
    static bool checkTypeEqNonZeroArg() {
      using namespace ka;
      using namespace ka::test;
      return
        testing::StaticAssertTypeEq<
          ka::ApplyIndexed<T, A>,
          T<indexed_t<0, A>>
        >() &&
        testing::StaticAssertTypeEq<
          ka::ApplyIndexed<T, A, B>,
          T<indexed_t<0, A>, indexed_t<1, B>>
        >() &&
        testing::StaticAssertTypeEq<
          ka::ApplyIndexed<T, A, B, C>,
          T<indexed_t<0, A>, indexed_t<1, B>, indexed_t<2, C>>
        >();
    }

    template<template<typename...> class T> constexpr
    static bool checkTypeEq() {
      return
        testing::StaticAssertTypeEq<
          ka::ApplyIndexed<T>,
          T<>
        >() &&
        checkTypeEqNonZeroArg<T>();
    }
  };
} // anonymous namespace

TEST_F(ApplyIndexed, Basic) {
  checkTypeEq<variadic_type>();
  checkTypeEq<std::tuple>();
  checkTypeEqNonZeroArg<boost::variant>();
}
