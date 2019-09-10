/*
** Copyright (C) 2015 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/property.hpp>
#include <qi/signalspy.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/container/stable_vector.hpp>
#include <boost/container/small_vector.hpp>
#include <thread>
#include <chrono>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>

qiLogCategory("qi.test.property");

using namespace qi;

namespace test
{
  template< template< class ... > class PropertyType >
  void setDuringCreation()
  {
    PropertyType<int> prop(4);
    qi::AnyValue val = prop.value().value();
    ASSERT_EQ(4, val.to<int>());
  }

  template< template< class ... > class PropertyType >
  void signalOfProperty()
  {
    PropertyType<int> prop(4);
    qi::SignalSpy spy(prop);
    prop.setValue(2);
    std::this_thread::sleep_for(std::chrono::seconds{ 1 });
    qi::AnyValue val = prop.value().value();
    ASSERT_EQ(2, val.to<int>());
    ASSERT_EQ(1u, spy.recordCount());
  }

  template< template< class ... > class PropertyType >
  void keepValue()
  {
    PropertyType<int> k;
    k.set(42);
    EXPECT_EQ(42, k.get().value());
  }


  template< class T >
  struct Setter
  {
    qi::Promise<T> result;
    Setter(qi::Promise<T> result)
      : result(result)
    {}

    void operator()(const T& value)
    {
      result.setValue(value);
    }
  };

  template< template< class ... > class PropertyType >
  void dispatchValue()
  {
    PropertyType<int> k;
    qi::Promise<int> object;
    k.connect(Setter<int>(object));
    k.set(42);
    qi::Future<int> result = object.future();
    EXPECT_EQ(42, result.value());
    EXPECT_EQ(42, k.get().value());
  }

  template< template< class ... > class PropertyType >
  void dispatchAssignedValue()
  {
    PropertyType<int> k;
    qi::Promise<int> object;
    k.connect(Setter<int>(object));
    k = 42;
    qi::Future<int> result = object.future();
    EXPECT_EQ(42, result.value());
    EXPECT_EQ(42, k.get().value());
  }

}

TEST(TestUnsafeProperty, SetDuringCreation)
{
  test::setDuringCreation<qi::UnsafeProperty>();
}

TEST(TestUnsafeProperty, SignalOfProperty)
{
  test::signalOfProperty<qi::UnsafeProperty>();
}

TEST(TestUnsafeProperty, keepValue)
{
  test::keepValue<qi::UnsafeProperty>();
}

TEST(TestUnsafeProperty, dispatchValue)
{
  test::dispatchValue<qi::UnsafeProperty>();
}

TEST(TestUnsafeProperty, dispatchAssignedValue)
{
  test::dispatchAssignedValue<qi::UnsafeProperty>();
}


TEST(TestProperty, SetDuringCreation)
{
  test::setDuringCreation<qi::Property>();
}

TEST(TestProperty, SignalOfProperty)
{
  test::signalOfProperty<qi::Property>();
}

TEST(TestProperty, keepValue)
{
  test::keepValue<qi::Property>();
}

TEST(TestProperty, dispatchValue)
{
  test::dispatchValue<qi::Property>();
}

TEST(TestProperty, dispatchAssignedValue)
{
  test::dispatchAssignedValue<qi::Property>();
}

TEST(TestProperty, customSetter)
{
  qi::Property<int> property{12, qi::Property<int>::Getter{}, [](int& storage, const int& value)
  {
    storage = value;
    return true;
  }};
  const int expected = 42;
  property.set(expected);
  EXPECT_EQ(expected, property.get().value());
}

TEST(TestProperty, customSetterStranded)
{
  const auto strand = std::make_shared<qi::Strand>();
  std::weak_ptr<qi::Strand> weakStrand = strand;
  qi::Property<int> property{ 12, qi::Property<int>::Getter{},
                              [=](int& storage, const int& value) {
                                if (auto strand = weakStrand.lock())
                                  return strand->async([&] {
                                    storage = value;
                                    return true;
                                  }).value();
                                return false;
                              } };
  const int expected = 42;
  property.set(expected);
  EXPECT_EQ(expected, property.get().value());
}

TEST(TestProperty, customSetterReturningFalseFailsButDoesNotThrow)
{
  const int initialValue = 12;
  qi::Property<int> property{initialValue, qi::Property<int>::Getter{}, [](int&, const int&)
  {
    return false;
  }};
  const int newValue = 42;
  property.set(newValue);
  EXPECT_EQ(initialValue, property.get().value());
}

using CustomException = std::exception;

TEST(TestProperty, customSetterThrowIsTransmitted)
{
  const int initialValue = 12;
  qi::Property<int> property{initialValue, qi::Property<int>::Getter{}, [](int&, const int&)->bool
  {
    throw CustomException{};
  }};
  const int newValue = 42;
  ASSERT_THROW(property.set(newValue), CustomException);
  EXPECT_EQ(initialValue, property.get().value());
}

TEST(TestProperty, WithStrandCallsGetterSetterFromIt)
{
  qi::Strand strand;
  qi::Property<int> property{ 42, strand,
                              [&](const int& v) {
                                EXPECT_TRUE(strand.isInThisContext());
                                return v;
                              },
                              [&](int& val, int newVal) {
                                EXPECT_TRUE(strand.isInThisContext());
                                val = newVal;
                                return true;
                              } };
  auto newVal = 13;
  EXPECT_NO_THROW(property.set(newVal).value());
  EXPECT_EQ(newVal, property.get().value());

  newVal = 8392;
  EXPECT_NO_THROW(property.set(newVal).value());
  EXPECT_EQ(newVal, property.get().value());
}

using Containers = testing::Types<
  std::array<int, 2>,
  std::vector<int>,
  std::deque<int>,
  std::forward_list<int>,
  std::list<int>,
  std::set<int>,
  std::map<int, int>,
  std::multiset<int>,
  std::multimap<int, int>,
  std::unordered_set<int>,
  std::unordered_map<int, int>,
  std::stack<int>,
  std::queue<int>,
  std::priority_queue<int>,
  boost::container::flat_map<int, int>,
  boost::container::flat_set<int>,
  boost::container::static_vector<int, 2>,
  boost::container::small_vector<int, 2>,
  boost::container::stable_vector<int>
>;

template<typename T>
struct TestPropertyWithContainerType : testing::Test {};
TYPED_TEST_CASE(TestPropertyWithContainerType, Containers);

TYPED_TEST(TestPropertyWithContainerType, CanBeInstanciatedWithContainer)
{
  qi::Property<TypeParam> p;
  SUCCEED();
}

TEST(TestReadOnlyProperty, Get)
{
  Property<int> source{ 42 };
  ReadOnlyProperty<int> prop{ source };
  EXPECT_EQ(42, prop.get().value());
  EXPECT_EQ(42, prop.value().value().to<int>());
}

TEST(TestReadOnlyProperty, SetThroughAdaptedProperty)
{
  Property<int> source{ 42 };
  ReadOnlyProperty<int> prop{ source };
  EXPECT_EQ(42, prop.get().value());
  source.set(51).value();
  EXPECT_EQ(51, prop.get().value());
}
