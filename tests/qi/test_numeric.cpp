/*
**  Copyright (C) 2018 SoftBank Robotics Europe
**  See COPYING for the license
*/

#include <cstdint>
#include <gtest/gtest.h>
#include <ka/typetraits.hpp>
#include <qi/numeric.hpp>

namespace
{

const auto int16max = std::numeric_limits<std::int16_t>::max();
const auto uint32max = std::numeric_limits<std::uint32_t>::max();

}

using NumericTypes = testing::Types<char, unsigned char,
                                    short, unsigned short,
                                    int, unsigned int,
                                    long, unsigned long,
                                    long long, unsigned long long,
                                    float, double, long double>;

template<typename T>
struct NumericConvertTyped : testing::Test {};
TYPED_TEST_SUITE(NumericConvertTyped, NumericTypes);

TYPED_TEST(NumericConvertTyped, ReturnTypeIsFirstTemplateParamater)
{
  static_assert(
    ka::Equal<decltype(qi::numericConvert<TypeParam, int>(std::declval<int>())),
              TypeParam>::value,
    "");
}

TEST(NumericConvert, SameTypeReturnsSameValue)
{
  int a = 42;
  EXPECT_EQ(static_cast<int>(42), qi::numericConvert<int>(a));
}

TEST(NumericConvert, BiggerTypeReturnsSameValue)
{
  std::int16_t a = 1278;
  EXPECT_EQ(static_cast<std::int32_t>(1278), qi::numericConvert<std::int32_t>(a));
}

TEST(NumericConvert, SmallerTypeButValueFitsReturnsSameValue)
{
  std::int32_t a = 12345;
  EXPECT_EQ(static_cast<std::int16_t>(12345), qi::numericConvert<std::int16_t>(a));
}

TEST(NumericConvert, OverflowThrows)
{
  std::int32_t a = static_cast<std::int32_t>(int16max) + 1;
  EXPECT_THROW(qi::numericConvert<std::int16_t>(a), qi::NumericPositiveOverflow);
  EXPECT_THROW(qi::numericConvert<std::int32_t>(uint32max), qi::NumericPositiveOverflow);
  EXPECT_THROW(qi::numericConvert<std::int32_t>(1e28f), qi::NumericPositiveOverflow);
  EXPECT_THROW(qi::numericConvert<std::int32_t>(1e78), qi::NumericPositiveOverflow);
}

TEST(NumericConvert, UnderflowThrows)
{
  std::int32_t a = -1;
  EXPECT_THROW(qi::numericConvert<std::uint32_t>(a), qi::NumericNegativeOverflow);
  EXPECT_THROW(qi::numericConvert<std::int32_t>(-1e24f), qi::NumericNegativeOverflow);
  EXPECT_THROW(qi::numericConvert<std::int32_t>(-1e54), qi::NumericNegativeOverflow);
}

template<typename T>
struct NumericConvertBoundTyped : testing::Test {};
TYPED_TEST_SUITE(NumericConvertBoundTyped, NumericTypes);

TYPED_TEST(NumericConvertBoundTyped, ReturnTypeIsFirstTemplateParamater)
{
  static_assert(
    ka::Equal<decltype(qi::numericConvertBound<TypeParam, int>(std::declval<int>())),
              TypeParam>::value,
    "");
}

TEST(NumericConvertBound, SameTypeReturnsSameValue)
{
  int a = 328091;
  EXPECT_EQ(static_cast<int>(328091), qi::numericConvertBound<int>(a));
}

TEST(NumericConvertBound, BiggerTypeReturnsSameValue)
{
  std::int16_t a = 3267;
  EXPECT_EQ(static_cast<std::int32_t>(3267), qi::numericConvertBound<std::int32_t>(a));
}

TEST(NumericConvertBound, SmallerTypeButValueFitsReturnsSameValue)
{
  std::int32_t a = 3479;
  EXPECT_EQ(static_cast<std::int16_t>(3479), qi::numericConvertBound<std::int16_t>(a));
}

TEST(NumericConvertBound, OverflowBoundsValueToMax)
{
  std::int32_t a = static_cast<std::int32_t>(int16max) + 1;
  EXPECT_EQ(int16max, qi::numericConvertBound<std::int16_t>(a));
  EXPECT_EQ(int16max, qi::numericConvertBound<std::int16_t>(1e45));
}

TEST(NumericConvertBound, UnderflowBoundsValueToMin)
{
  const auto uint32min = std::numeric_limits<std::uint32_t>::min();
  std::int32_t a = -1;
  EXPECT_EQ(uint32min, qi::numericConvertBound<std::uint32_t>(a));
  EXPECT_EQ(uint32min, qi::numericConvertBound<std::uint32_t>(1e-23));
}

TEST(NumericIsInRange, BasicReturnsTrue)
{
  EXPECT_TRUE(qi::numericIsInRange<std::uint32_t>(8329));
}

TEST(NumericIsInRange, UnderflowReturnsFalse)
{
  EXPECT_FALSE(qi::numericIsInRange<std::uint32_t>(-1));
  EXPECT_FALSE(qi::numericIsInRange<std::int32_t>(-1e42));
  EXPECT_FALSE(qi::numericIsInRange<std::int32_t>(-1e12f));
}

TEST(NumericIsInRange, OverflowReturnsFalse)
{
  EXPECT_FALSE(qi::numericIsInRange<std::int32_t>(uint32max));
  EXPECT_FALSE(qi::numericIsInRange<std::int32_t>(1e67));
  EXPECT_FALSE(qi::numericIsInRange<std::int32_t>(1e32f));
}
