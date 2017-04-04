#include <vector>
#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <qi/functional.hpp>
#include <qi/detail/conceptpredicate.hpp>
#include <qi/range.hpp>

TEST(FunctionalPolymorphicConstantFunction, RegularNonVoid)
{
  using namespace qi;
  using namespace qi::detail;
  using F = PolymorphicConstantFunction<int>;
  const auto incr = [](F& f) {
    ++f.ret;
  };
  // F is regular because int is.
  ASSERT_TRUE(isRegular(incrRange(F{0}, F{100}, incr)));
}

TEST(FunctionalPolymorphicConstantFunction, RegularVoid)
{
  using namespace qi;
  using namespace qi::detail;
  using F = PolymorphicConstantFunction<void>;
  ASSERT_TRUE(isRegular({F{}}));
}

struct NonRegular
{
  int i;
  friend bool operator==(NonRegular a, NonRegular b)
  {
    return &a == &b;
  }
  friend bool operator<(NonRegular a, NonRegular b)
  {
    return &a < &b;
  }
};

TEST(FunctionalPolymorphicConstantFunction, NonRegularNonVoid)
{
  using namespace qi;
  using namespace qi::detail;
  using F = PolymorphicConstantFunction<NonRegular>;
  auto incr = [](F& f) {
    ++f.ret.i;
  };
  // F is not regular because NonRegular isn't.
  ASSERT_FALSE(isRegular(incrRange(F{{0}}, F{{100}}, incr)));
}

TEST(FunctionalPolymorphicConstantFunction, BasicNonVoid)
{
  using namespace qi;
  const char c = 'z';
  PolymorphicConstantFunction<unsigned char> f{c};
  ASSERT_EQ(c, f());
  ASSERT_EQ(c, f(1));
  ASSERT_EQ(c, f(2.345));
  ASSERT_EQ(c, f("abcd"));
  ASSERT_EQ(c, f(true));
  ASSERT_EQ(c, f(std::vector<int>{5, 7, 2, 1}));
  ASSERT_EQ(c, f(1, 2.345, "abcd", true));
}

TEST(FunctionalPolymorphicConstantFunction, BasicVoid)
{
  using namespace qi;
  PolymorphicConstantFunction<void> f;
  ASSERT_NO_THROW(f());
  ASSERT_NO_THROW(f(1));
  ASSERT_NO_THROW(f(2.345));
  ASSERT_NO_THROW(f("abcd"));
  ASSERT_NO_THROW(f(true));
  ASSERT_NO_THROW(f(std::vector<int>{5, 7, 2, 1}));
  ASSERT_NO_THROW(f(1, 2.345, "abcd", true));
}
