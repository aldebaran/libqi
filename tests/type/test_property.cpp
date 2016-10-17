/*
** Copyright (C) 2015 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/property.hpp>
#include <qi/signalspy.hpp>

qiLogCategory("qi.test.property");

namespace test
{
  template< template< class ... > class PropertyType >
  void setDuringCreation()
  {
    PropertyType<int> prop(4);
    qi::AnyValue val = prop.value();
    ASSERT_EQ(4, val.to<int>());
  }

  template< template< class ... > class PropertyType >
  void signalOfProperty()
  {
    PropertyType<int> prop(4);
    qi::SignalSpy spy(prop);
    prop.setValue(2);
    qi::os::sleep(1);
    qi::AnyValue val = prop.value();
    ASSERT_EQ(2, val.to<int>());
    ASSERT_EQ(1u, spy.recordCount());
  }

  template< template< class ... > class PropertyType >
  void keepValue()
  {
    PropertyType<int> k;
    k.set(42);
    EXPECT_EQ(42, k.get());
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
    EXPECT_EQ(42, k.get());
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
    EXPECT_EQ(42, k.get());
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

TEST(TestProperty, scopedLockReadOnly)
{
  qi::Property<int> k;
  k.set(42);
  int result = 0;
  {
    qi::Property<int>::ScopedLockReadOnly locked_k(k);
    result = *locked_k;
  }
  EXPECT_EQ(42, result);
}

TEST(TestProperty, scopedLockReadWrite)
{
  qi::Property<int> k;
  k.set(42);
  int result = 0;
  {
    qi::Property<int>::ScopedLockReadWrite locked_k(k);
    result = *locked_k;
    EXPECT_EQ(42, result);
  }
  EXPECT_EQ(42, result);

  qi::Future<int> handlerResult;
  qi::Promise<int> object;
  k.connect(test::Setter<int>(object));
  handlerResult = object.future();
  {
    qi::Property<int>::ScopedLockReadWrite locked_k(k);

    *locked_k = 1234;
  }
  EXPECT_EQ(1234, handlerResult.value());
}

class Dummy
{
  int value;
public:
  Dummy() : value(0) { }
  int read() const { return value; }
  void write(int newValue) { value = newValue; }
};

TEST(TestProperty, scopedReadOnlyClassMember)
{
  qi::Property<Dummy> k;
  {
    qi::Property<Dummy>::ScopedLockReadOnly locked_k(k);
    const int value = locked_k->read();
    EXPECT_EQ(0, value);
  }
}

TEST(TestProperty, scopedReadWriteClassMember)
{
  qi::Property<Dummy> k;
  {
    qi::Property<Dummy>::ScopedLockReadWrite locked_k(k);
    locked_k->write(42);
    const int value = locked_k->read();
    EXPECT_EQ(42, value);
  }
}

struct Incrementer
{
  qi::Property<int>* const counter;
  Incrementer(qi::Property<int>& counter)
    : counter(&counter)
  {}
  void operator()() const
  {
    qi::Property<int>::ScopedLockReadWrite locked_counter(*counter);
    ++(*locked_counter);
  }
};

struct Matcher
{
  qi::Promise<bool> running;
  int targetValue;
  Matcher(int targetValue, qi::Promise<bool> running)
    : running(running)
    , targetValue(targetValue)
  {}
  void operator()(int newValue)
  {
    if (newValue == targetValue)
    {
      running.setValue(true);
    }
  }
};

TEST(TestProperty, threadSafeGetSet)
{
  static const int TARGET_COUNT = 42;
  qi::Promise<bool> isRunning;
  qi::Future<bool> running = isRunning.future();
  qi::Property<int> counter;
  counter.set(0);

  counter.connect(Matcher(TARGET_COUNT, isRunning));

  for (int i = 0; i < TARGET_COUNT; ++i)
  {
    qi::async(Incrementer(counter));
  }

  EXPECT_TRUE(running.value());
  EXPECT_EQ(TARGET_COUNT, counter.get());
}

TEST(TestProperty, customSetter)
{
  qi::Property<int> property{12, qi::Property<int>::Getter{}, [this](int& storage, const int& value)
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
  qi::Strand strand;
  qi::Property<int> property{12, qi::Property<int>::Getter{}, strand.schedulerFor([this](int& storage, const int& value)
  {
    storage = value;
    return true;
  })};
  const int expected = 42;
  property.set(expected);
  EXPECT_EQ(expected, property.get().value());
}
