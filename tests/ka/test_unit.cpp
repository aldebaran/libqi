#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <ka/unit.hpp>
#include <ka/testutils.hpp>

TEST(Unit, Regular) {
  using namespace ka;
  ASSERT_TRUE(is_regular({unit, unit, unit}));
}

TEST(Unit, ConstantUnitT) {
  using namespace ka;
  using namespace ka::test;
  using testing::StaticAssertTypeEq;
  StaticAssertTypeEq<unit_t, constant_unit_t<>::type>();
  StaticAssertTypeEq<unit_t, constant_unit_t<A>::type>();
  StaticAssertTypeEq<unit_t, constant_unit_t<A, B>::type>();
  StaticAssertTypeEq<unit_t, constant_unit_t<A, B, C>::type>();
  StaticAssertTypeEq<unit_t, constant_unit_t<A, B, C, D>::type>();
  StaticAssertTypeEq<unit_t, constant_unit_t<A, B, C, D, E>::type>();
  StaticAssertTypeEq<unit_t, constant_unit_t<A, B, C, D, E, F>::type>();
}

TEST(Unit, ConstantUnitTRegular) {
  using namespace ka;
  using namespace ka::test;
  auto x = constant_unit_t<A, B, C>{};
  ASSERT_TRUE(is_regular({x, x, x}));
}

TEST(Unit, ConstantUnit) {
  using namespace ka;
  using namespace ka::test;
  using testing::StaticAssertTypeEq;
  StaticAssertTypeEq<unit_t, ConstantUnit<>>();
  StaticAssertTypeEq<unit_t, ConstantUnit<A>>();
  StaticAssertTypeEq<unit_t, ConstantUnit<A, B>>();
  StaticAssertTypeEq<unit_t, ConstantUnit<A, B, C>>();
  StaticAssertTypeEq<unit_t, ConstantUnit<A, B, C, D>>();
  StaticAssertTypeEq<unit_t, ConstantUnit<A, B, C, D, E>>();
  StaticAssertTypeEq<unit_t, ConstantUnit<A, B, C, D, E, F>>();
}
