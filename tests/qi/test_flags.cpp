/*
**  Copyright (C) 2017 SoftBank Robotics
**  See COPYING for the license
*/

#include <gtest/gtest.h>
#include <qi/flags.hpp>
#include <ka/conceptpredicate.hpp>
#include <set>
#include <type_traits>

using std::set;

////////////////////////////////////////////////////////////////////////////////
// Types and functions declarations
////////////////////////////////////////////////////////////////////////////////
template <typename T> const set<T>& setOf();

////////////////////////////////////////////////////////////////////////////////
// Flags from an enum
////////////////////////////////////////////////////////////////////////////////
enum class Opt
{
  First  = 1 << 0,
  Second = 1 << 1,
  Third  = 1 << 2,
};
using ExampleFlags = qi::Flags<Opt>;

template <>
const set<Opt>& setOf<Opt>()
{
  static const set<Opt>& optionSet{Opt::First, Opt::Second, Opt::Third};
  return optionSet;
}

template <>
const set<ExampleFlags>& setOf<ExampleFlags>()
{
  static const set<ExampleFlags>& exampleFlagsSet{ExampleFlags{Opt::First},
                                                  ExampleFlags{Opt::Second},
                                                  ExampleFlags{Opt::First, Opt::Second},
                                                  ExampleFlags{Opt::Third},
                                                  ExampleFlags{Opt::Third, Opt::First},
                                                  ExampleFlags{Opt::Third, Opt::Second},
                                                  ExampleFlags{Opt::Third, Opt::Second, Opt::First},
                                                  ExampleFlags{}};
  return exampleFlagsSet;
}

////////////////////////////////////////////////////////////////////////////////
// Flags from a class
////////////////////////////////////////////////////////////////////////////////
class OptionClass
{
public:
  OptionClass()
    : _value{}
  {}

  explicit OptionClass(unsigned int value)
    : _value(value)
  {}

  friend unsigned int underlying(const OptionClass& a)
  {
    return a._value;
  }

  KA_GENERATE_FRIEND_REGULAR_OPS_1(OptionClass, _value)

private:
  unsigned int _value;
};
using ExampleClassFlags = qi::Flags<OptionClass>;


template <>
const set<OptionClass>& setOf<OptionClass>()
{
  static const set<OptionClass>& optionClassSet{OptionClass{16}, OptionClass{32}};
  return optionClassSet;
}

template <>
const set<ExampleClassFlags>& setOf<ExampleClassFlags>()
{
  static const set<ExampleClassFlags>& exampleClassFlagsSet{ExampleClassFlags{OptionClass{16}},
                                                            ExampleClassFlags{OptionClass{32}},
                                                            ExampleClassFlags{OptionClass{48}},
                                                            ExampleClassFlags{OptionClass{0}}};
  return exampleClassFlagsSet;
}

////////////////////////////////////////////////////////////////////////////////
// Test case
////////////////////////////////////////////////////////////////////////////////
template <typename T>
class TestFlags : public ::testing::Test
{
public:
  using Flags = T;
  using Flag = typename Flags::type;

  static auto setOfFlag() -> decltype(setOf<Flag>())
  {
    return setOf<Flag>();
  }

  void firstValue(Flag& val) const
  {
    // assert that the flag type has at least one value
    ASSERT_GT(setOfFlag().size(), 0u);

    val = *setOfFlag().begin();
  }

  Flag firstValue() const
  {
    Flag v {};
    firstValue(v);
    return v;
  }

  void twoFirstValues(Flag& first, Flag& second) const
  {
    ASSERT_GE(setOfFlag().size(), 2u);

    first = *setOfFlag().begin();
    second = *(++setOfFlag().begin());
  }

  std::pair<Flag, Flag> twoFirstValues() const
  {
    Flag v1 {}, v2{};
    twoFirstValues(v1, v2);
    return {v1, v2};
  }

  template<typename Range>
  static void expectFlags(const Flags& f, const Range& r)
  {
    using namespace std;
    const auto rEnd = end(r);
    for (const auto& val : setOfFlag())
    {
      if (find(begin(r), rEnd, val) == rEnd)
        EXPECT_FALSE(f.test(val));
      else
        EXPECT_TRUE(f.test(val));
    }
  }

  static void expectFlags(const Flags& f, const std::initializer_list<Flag>& r)
  {
    expectFlags(f, std::vector<Flag>(r));
  }

};

typedef ::testing::Types<ExampleFlags, ExampleClassFlags> FlagsTypes;
TYPED_TEST_SUITE(TestFlags, FlagsTypes);

TYPED_TEST(TestFlags, Regular)
{
  EXPECT_TRUE(ka::is_regular(ka::bounded_range(setOf<typename TestFixture::Flags>())));
}

TYPED_TEST(TestFlags, DefaultConstructNoFlags)
{
  using Flags = TypeParam;
  using Flag = typename TypeParam::type;

  Flags f; // default constructed
  for (const auto& val : setOf<Flag>())
  {
    EXPECT_FALSE(f.test(val));
  }
}

TYPED_TEST(TestFlags, ConstructWithAFlag)
{
  using Flags = TypeParam;

  const auto firstVal = this->firstValue();
  Flags f {firstVal};
  this->expectFlags(f, {firstVal});
}

TYPED_TEST(TestFlags, Set)
{
  using Flags = TypeParam;

  const auto firstVal = this->firstValue();
  Flags f; // default constructed
  f.set(firstVal);
  this->expectFlags(f, {firstVal});
}

TYPED_TEST(TestFlags, Reset)
{
  using Flags = TypeParam;

  const auto firstVal = this->firstValue();
  Flags f {firstVal};
  f.reset(firstVal);
  this->expectFlags(f, {});
}

TYPED_TEST(TestFlags, ResetOtherFlag)
{
  using Flags = TypeParam;

  const auto values = this->twoFirstValues();

  Flags f(values.first);
  f.reset(values.second);
  this->expectFlags(f, {values.first});
}

TYPED_TEST(TestFlags, OrOperator)
{
  using Flags = TypeParam;

  const auto values = this->twoFirstValues();

  Flags f = Flags{values.first, values.second};
  this->expectFlags(f, {values.first, values.second});
}

TYPED_TEST(TestFlags, OrAffectOperator)
{
  using Flags = TypeParam;

  const auto values = this->twoFirstValues();

  Flags f(values.first);
  f |= Flags(values.second);
  this->expectFlags(f, {values.first, values.second});
}

TYPED_TEST(TestFlags, AndOperator)
{
  using Flags = TypeParam;

  const auto values = this->twoFirstValues();

  {
    Flags f = Flags{values.first, values.second} & Flags(values.second);
    this->expectFlags(f, {values.second});
  }

  {
    Flags f = Flags{values.first, values.second} & Flags(values.first);
    this->expectFlags(f, {values.first});
  }
}

TYPED_TEST(TestFlags, AndAffectOperator)
{
  using Flags = TypeParam;

  const auto values = this->twoFirstValues();

  {
    Flags f = Flags(values.first) | Flags(values.second);
    f &= Flags(values.second);
    this->expectFlags(f, {values.second});
  }

  {
    Flags f = Flags(values.first) | Flags(values.second);
    f &= Flags(values.first);
    this->expectFlags(f, {values.first});
  }
}

TYPED_TEST(TestFlags, XorOperator)
{
  using Flags = TypeParam;

  const auto values = this->twoFirstValues();

  {
    Flags f = Flags(values.first) ^ Flags(values.second);
    this->expectFlags(f, {values.first, values.second});
  }

  {
    Flags f = Flags(values.first) ^ Flags(values.first);
    this->expectFlags(f, {});
  }
}

TYPED_TEST(TestFlags, XorAffectOperator)
{
  using Flags = TypeParam;

  const auto values = this->twoFirstValues();

  {
    Flags f{values.first};
    f ^= Flags(values.second);
    this->expectFlags(f, {values.first, values.second});
  }

  {
    Flags f{values.first};
    f ^= Flags(values.first);
    this->expectFlags(f, {});
  }
}

TYPED_TEST(TestFlags, NotOperator)
{
  using Flags = TypeParam;

  const auto value = this->firstValue();
  Flags f = ~(Flags{value});

  auto allFlagsButValue = this->setOfFlag();
  allFlagsButValue.erase(value);
  this->expectFlags(f, allFlagsButValue);
}
