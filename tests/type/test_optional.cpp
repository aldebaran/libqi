/*
**  Copyright (C) 2018 SoftBank Robotics Europe
**  See COPYING for the license
*/

#include <boost/optional.hpp>
#include <gtest/gtest.h>
#include <qi/type/typeinterface.hpp>

using namespace qi;

// If T doesn't have a less operator, the qi type interface for `boost::optional<T>` must fallback
// to comparing the addresses of the objects when using `qi::TypeInterface::less`.
TEST(Optional, TypeOfOptionalOfTypeUsesPtrLessOpIfNoneExists)
{
  struct S { int v; };
  using OptS = boost::optional<S>;

  auto* const typeOfOptStruct = static_cast<OptionalTypeInterface*>(qi::typeOf<OptS>());

  const OptS s1;
  auto* const vss1 = AnyReference::from(s1).rawValue();
  const OptS s2;
  auto* const vss2 = AnyReference::from(s2).rawValue();

  EXPECT_EQ(&s1 < &s2, typeOfOptStruct->less(vss1, vss2));
  EXPECT_EQ(&s2 < &s1, typeOfOptStruct->less(vss2, vss1));
}

// If T has a less operator, the qi type interface for `boost::optional<T>` must use that operator
// when using `qi::TypeInterface::less`.
TEST(Optional, TypeOfOptionalOfTypeUsesTypeLessOpIfItExists)
{
  struct S
  {
    S(int v) : v(v) {}
    int v;
    bool operator<(const S& o) const { return v < o.v; }
  };
  using OptS = boost::optional<S>;

  auto* const typeOfOptStruct = static_cast<OptionalTypeInterface*>(qi::typeOf<OptS>());

  const OptS s1{ 42 };
  auto* const vss1 = AnyReference::from(s1).rawValue();
  const OptS s2{ 15 };
  auto* const vss2 = AnyReference::from(s2).rawValue();

  EXPECT_EQ(s1 < s2, typeOfOptStruct->less(vss1, vss2));
  EXPECT_EQ(s2 < s1, typeOfOptStruct->less(vss2, vss1));
}
