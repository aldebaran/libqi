#include <type_traits>
#include <gtest/gtest.h>
#include <qi/utility.hpp>
#include "tools.hpp"

TEST(UtilityFwd, Basic)
{
  using namespace qi;
  int i = 5;
  // In a template context, a "universal reference" `T&&` is deduced as:
  // - `U&` if a lvalue of type U if passed
  // - `U` otherwise (a rvalue of type U is passed)
  static_assert(std::is_lvalue_reference<decltype(fwd<int&>(i))>::value, "");
  static_assert(std::is_rvalue_reference<decltype(fwd<int>(5))>::value, "");
}

namespace test
{
  template<typename T>
  RefKind g(MoveOnly<T>&&)
  {
    return RefKind::RValue;
  }

  template<typename T>
  RefKind g(MoveOnly<T>&)
  {
    return RefKind::LValue;
  }

  template<typename T>
  RefKind f(T&& x)
  {
    using namespace qi;
    return g(fwd<T>(x));
  }
}

TEST(UtilityFwd, RealWord)
{
  using namespace qi;
  using namespace test;
  MoveOnly<int> m{4};
  ASSERT_EQ(RefKind::LValue, f(m));
  ASSERT_EQ(RefKind::RValue, f(MoveOnly<int>{4}));
}
