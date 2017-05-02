/*
**
** Copyright (C) 2010-2016, Aldebaran Robotics
*/

#include <qi/detail/conceptpredicate.hpp>
#include <qi/detail/relationpredicate.hpp>
#include "test_relations.hpp"
#include <qi/range.hpp>
#include <qi/type/traits.hpp>
#include <qi/macroregular.hpp>
#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <cstdint>

#include <iterator>
#include <vector>
#include <functional>
#include <iostream>
#include <cassert>

// This file contains tests on concept predicates, for example isRegular.

namespace test
{
  template<typename T>
  bool operator!=(const T& a, const T& b)
  {
    return !(a == b);
  }

  struct Regular
  {
    using Self = Regular;
    int x = 0;
    Regular(int x) : x(x) {}
  // Regular:
    // Copy, assignment and destruction by default.
    Regular() = default;
    QI_GENERATE_FRIEND_REGULAR_OPS_1(Self, x)
  };

  struct RegularStateless
  {
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_0(RegularStateless)
  };

  struct EqualityOfAddress
  {
    using Self = EqualityOfAddress;
    int x = 0;
    EqualityOfAddress(int x) : x(x) {}
  // Regular:
    // Copy, assignment and destruction by default.
    EqualityOfAddress() = default;
    friend bool operator==(const Self& a, const Self& b)
    {
      return &a == &b;
    }
    friend bool operator<(Self a, Self b)
    {
      return a.x < b.x;
    }
  };

  struct LessIsLessEqual
  {
    using Self = LessIsLessEqual;
    int x = 0;
    LessIsLessEqual(int x) : x(x) {}
  // Regular:
    // Copy, assignment and destruction by default.
    LessIsLessEqual() = default;
    friend bool operator==(Self a, Self b)
    {
      return a.x == b.x;
    }
    friend bool operator<(Self a, Self b)
    {
      return a.x <= b.x;
    }
  };

  struct BadAssignment
  {
    using Self = BadAssignment;
    int x = 0;
    BadAssignment(int x) : x(x) {}
  // Regular:
    // Copy and destruction by default.
    BadAssignment() = default;
    void operator=(const Self& a)
    {
      x = a.x + 1;
    }
    friend bool operator==(Self a, Self b)
    {
      return a.x == b.x;
    }
    friend bool operator<(Self a, Self b)
    {
      return a.x < b.x;
    }
  };

  struct BadCopy
  {
    using Self = BadCopy;
    int x = {};
  // Regular:
    // Default constructor, copy and destruction by default.
    BadCopy() = default;
    BadCopy(int x) : x{x}
    {
    }
    BadCopy(const Self& a) : x{a.x + 1}
    {
    }
    friend bool operator==(Self a, Self b)
    {
      return a.x == b.x;
    }
    friend bool operator<(Self a, Self b)
    {
      return a.x < b.x;
    }
  };

  // We don't want to add operator++ to the (pseudo) regular types above,
  // to ensure they are only Regular (minimal interface), but we want
  // for test purpose to easily create incremental ranges.
  struct IncrMemberX
  {
    template<typename T>
    void operator()(T& t) const
    {
      ++t.x;
    }
  } incrMemberX;

} // namespace test

// Spaces cause problems in the macros (e.g EXPECT_TRUE).
using longlong = long long;
using longdouble = long double;
using unsignedchar = unsigned char;
using unsignedshort = unsigned short;
using unsignedint = unsigned int;
using unsignedlong = unsigned long;
using unsignedlonglong = unsigned long long;

TEST(ConceptPredicate, isRegularBuiltinTypes)
{
  using namespace qi;
  using qi::detail::isRegular;
  EXPECT_TRUE(isRegular({true, false}));
  EXPECT_TRUE(isRegular(incrRange(char(10))));
  EXPECT_TRUE(isRegular(incrRange(short(10))));
  EXPECT_TRUE(isRegular(incrRange(int(10))));
  EXPECT_TRUE(isRegular(incrRange(long(10))));
  EXPECT_TRUE(isRegular(incrRange(longlong(10))));
  EXPECT_TRUE(isRegular(incrRange(float(10))));
  EXPECT_TRUE(isRegular(incrRange(double(10))));
  EXPECT_TRUE(isRegular(incrRange(longdouble(10))));

  EXPECT_TRUE(isRegular(incrRange(unsignedchar(10))));
  EXPECT_TRUE(isRegular(incrRange(unsignedshort(10))));
  EXPECT_TRUE(isRegular(incrRange(unsignedint(10))));
  EXPECT_TRUE(isRegular(incrRange(unsignedlong(10))));
  EXPECT_TRUE(isRegular(incrRange(unsignedlonglong(10))));

  EXPECT_TRUE(isRegular(incrRange((int*)128)));
  EXPECT_TRUE(isRegular(incrRange((float*)128)));
}

TEST(ConceptPredicate, isRegularUserDefinedTypes)
{
  using namespace qi;
  using namespace test;
  using qi::detail::isRegular;

  EXPECT_TRUE(isRegular(incrRange(Regular{10}, incrMemberX)));
  {
    RegularStateless r;
    EXPECT_TRUE(isRegular(boundedRange(&r, &r + 1)));
  }
  EXPECT_FALSE(isRegular(incrRange(EqualityOfAddress{10}, incrMemberX)));
  EXPECT_FALSE(isRegular(incrRange(LessIsLessEqual{10}, incrMemberX)));
  EXPECT_FALSE(isRegular(incrRange(BadAssignment{10}, incrMemberX)));
  EXPECT_FALSE(isRegular(incrRange(BadCopy{10}, incrMemberX)));
}

namespace test
{
  // Allows specifying equality and total ordering relations.
  template<typename Eq, typename Lt> // Relation<Entity> Eq, Lt
  struct PseudoRegular
  {
    using Self = PseudoRegular;
    Entity x = entityBegin;
    PseudoRegular(Entity x) : x(x)
    {
    }
  // Regular:
    PseudoRegular() = default;
    // Default constructor, copy, assignment and destruction by default.
    friend bool operator==(Self a, Self b)
    {
      if (a.x == entityEnd || b.x == entityEnd) return a.x == b.x;
      return Eq{}(a.x, b.x);
    }
    friend bool operator<(Self a, Self b)
    {
      return Lt{}(a.x, b.x);
    }
    QI_GENERATE_FRIEND_REGULAR_DERIVED_OPS(Self);
  };

  // Relation<Entity> Eq, Relation<Entity> Lt
  template<typename Eq = std::equal_to<Entity>, typename Lt = std::less<Entity>>
  auto pseudoRegularEntityRange() -> decltype(qi::incrRange(PseudoRegular<Eq, Lt>{entityEnd}, incrMemberX))
  {
    return qi::incrRange(PseudoRegular<Eq, Lt>{entityEnd}, incrMemberX);
  }
} // namespace test

TEST(ConceptPredicate, isRegularWrongEquality)
{
  using namespace qi;
  using namespace qi::detail;
  using namespace test;
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<NotReflexiveNotSymmetricNotTransitive>()));
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<NotReflexiveSymmetricNotTransitive>()));
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<ReflexiveSymmetricNotTransitive>()));

  // This one must fail because even if the relation used for equality is an
  // equivalence, the trichotomy law is not respected
  // (i.e. we can have !(a < b) && !(b < a) && !(a == b)).
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<ReflexiveSymmetricTransitive>()));

  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<ReflexiveNotSymmetricNotTransitive>()));
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<ReflexiveNotSymmetricTransitive>()));
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<NotReflexiveSymmetricTransitive>()));
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<NotReflexiveNotSymmetricTransitive>()));
}

TEST(ConceptPredicate, isRegularWrongOrdering)
{
  using namespace qi;
  using namespace test;
  using namespace qi::detail;
  using Eq = std::equal_to<Entity>;
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<Eq, std::less_equal<Entity>>()));
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<Eq, NotTrichotomicNotTransitive>()));
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<Eq, TrichotomicNotTransitive>()));
  EXPECT_FALSE(isRegular(pseudoRegularEntityRange<Eq, NotTrichotomicTransitive>()));
}

TEST(ConceptPredicate, isRegularCorrectOrdering)
{
  using namespace qi;
  using namespace test;
  using namespace qi::detail;
  using Eq = std::equal_to<Entity>;
  EXPECT_TRUE(isRegular(pseudoRegularEntityRange<Eq, std::less<Entity>>()));
  EXPECT_TRUE(isRegular(pseudoRegularEntityRange<Eq, TrichotomicTransitive>()));
}
