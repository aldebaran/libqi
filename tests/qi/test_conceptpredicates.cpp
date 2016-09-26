/*
**
** Copyright (C) 2010-2016, Aldebaran Robotics
*/

#include <qi/detail/conceptpredicates.hpp>
#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <cstdint>

template<typename T>
bool operator!=(const T& a, const T& b)
{
  return !(a == b);
}

struct Regular
{
  using self = Regular;
  int i = 0;
// Regular:
  // Default constructor, copy, assignment and destruction by default.
  friend bool operator==(self x, self y)
  {
    return x.i == y.i;
  }
  friend bool operator<(self x, self y)
  {
    return x.i < y.i;
  }
};

struct RegularStateless
{
// Regular:
  RegularStateless() {}
  friend bool operator==(RegularStateless, RegularStateless) {return true;}
  friend bool operator< (RegularStateless, RegularStateless) {return false;}
};

struct EqualityOfAddress
{
  using self = EqualityOfAddress;
  int i = 0;
// Regular:
  // Default constructor, copy, assignment and destruction by default.
  friend bool operator==(self x, self y)
  {
    return &x == &y;
  }
  friend bool operator<(self x, self y)
  {
    return x.i < y.i;
  }
};

struct LessIsLessEqual
{
  using self = LessIsLessEqual;
  int i = 0;
// Regular:
  // Default constructor, copy, assignment and destruction by default.
  friend bool operator==(self x, self y)
  {
    return x.i == y.i;
  }
  friend bool operator<(self x, self y)
  {
    return x.i <= y.i;
  }
};

struct BadAssignment
{
  using self = BadAssignment;
  int i = 0;
// Regular:
  // Default constructor, copy and destruction by default.
  void operator=(const self& a)
  {
    i = a.i + 1;
  }
  friend bool operator==(self x, self y)
  {
    return x.i == y.i;
  }
  friend bool operator<(self x, self y)
  {
    return x.i < y.i;
  }
};

struct BadCopy
{
  using self = BadCopy;
  int i = 0;
// Regular:
  // Default constructor, copy and destruction by default.
  BadCopy() = default;
  BadCopy(const self& a) : i{a.i + 1}
  {
  }
  friend bool operator==(self x, self y)
  {
    return x.i == y.i;
  }
  friend bool operator<(self x, self y)
  {
    return x.i < y.i;
  }
};

// For some reason, spaces cause problems in the macros (e.g EXPECT_TRUE).
// There's no standard aliases without spaces on builtin types, so let's
// define them (keep cool, it's just a unit test :)).
using longlong = long long;
using longdouble = long double;
using unsignedchar = unsigned char;
using unsignedshort = unsigned short;
using unsignedint = unsigned int;
using unsignedlong = unsigned long;
using unsignedlonglong = unsigned long long;

TEST(ConceptPredicates, isRegularWithoutAction)
{
  using qi::detail::isRegular;
  EXPECT_TRUE(true);
  EXPECT_TRUE(isRegular(bool{}));
  EXPECT_TRUE(isRegular(char{}));
  EXPECT_TRUE(isRegular(short{}));
  EXPECT_TRUE(isRegular(int{}));
  EXPECT_TRUE(isRegular(long{}));
  EXPECT_TRUE(isRegular(longlong{}));
  EXPECT_TRUE(isRegular(float{}));
  EXPECT_TRUE(isRegular(double{}));
  EXPECT_TRUE(isRegular(longdouble{}));

  EXPECT_TRUE(isRegular(unsignedchar{}));
  EXPECT_TRUE(isRegular(unsignedshort{}));
  EXPECT_TRUE(isRegular(unsignedint{}));
  EXPECT_TRUE(isRegular(unsignedlong{}));
  EXPECT_TRUE(isRegular(unsignedlonglong{}));

  EXPECT_TRUE(isRegular<int*>(nullptr));
  EXPECT_TRUE(isRegular<float*>(nullptr));

  EXPECT_TRUE(isRegular(Regular{}));
  EXPECT_TRUE(isRegular(RegularStateless{}));
}

TEST(ConceptPredicates, isNotRegularWithoutAction)
{
  using qi::detail::isRegular;
  EXPECT_FALSE(isRegular(EqualityOfAddress{}));
  EXPECT_FALSE(isRegular(LessIsLessEqual{}));
  EXPECT_FALSE(isRegular(BadAssignment{}));
  EXPECT_FALSE(isRegular(BadCopy{}));
}

// TODO: Replace this by a polymorphic lambda when available.
struct Action
{
  template<typename A>
  void operator()(A& a) const
  {
    // We don't care about the safety of this, because the sole
    // purpose is to modify the value (see use below).
    ++*reinterpret_cast<std::uint8_t*>(&a);
  }
} action;

TEST(ConceptPredicates, isRegularWithAction)
{
  using qi::detail::isRegular;
  EXPECT_TRUE(isRegular(bool{}, action));
  EXPECT_TRUE(isRegular(char{}, action));
  EXPECT_TRUE(isRegular(short{}, action));
  EXPECT_TRUE(isRegular(int{}, action));
  EXPECT_TRUE(isRegular(long{}, action));
  EXPECT_TRUE(isRegular(longlong{}, action));
  EXPECT_TRUE(isRegular(float{}, action));
  EXPECT_TRUE(isRegular(double{}, action));
  EXPECT_TRUE(isRegular(longdouble{}, action));

  EXPECT_TRUE(isRegular(unsignedchar{}, action));
  EXPECT_TRUE(isRegular(unsignedshort{}, action));
  EXPECT_TRUE(isRegular(unsignedint{}, action));
  EXPECT_TRUE(isRegular(unsignedlong{}, action));
  EXPECT_TRUE(isRegular(unsignedlonglong{}, action));

  int i;
  EXPECT_TRUE(isRegular(&i, action));
  float f;
  EXPECT_TRUE(isRegular(&f, action));

  EXPECT_TRUE(isRegular(Regular{}, action));
}
