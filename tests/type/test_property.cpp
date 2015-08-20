/*
** Copyright (C) 2015 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/property.hpp>
#include <qi/signalspy.hpp>

qiLogCategory("qi.test.property");

TEST(TestProperty, SetDuringCreation)
{
  qi::Property<int> prop(4);
  ASSERT_EQ(4, prop.value().value().to<int>());
}

TEST(TestProperty, SignalOfProperty)
{
  qi::Property<int> prop(4);
  qi::SignalSpy spy(prop);
  prop.setValue(2);
  qi::os::sleep(1);
  ASSERT_EQ(2, prop.value().value().to<int>());
  ASSERT_EQ(1u, spy.getCounter());
}

TEST(TestProperty, keepValue)
{
  qi::Property<int> k;
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

TEST(TestProperty, dispatchValue)
{
  qi::Property<int> k;
  qi::Promise<int> object;
  k.connect(Setter<int>(object));
  k.set(42);
  qi::Future<int> result = object.future();
  EXPECT_EQ(42, result.value());
  EXPECT_EQ(42, k.get());
}

TEST(TestProperty, dispatchAssignedValue)
{
  qi::Property<int> k;
  qi::Promise<int> object;
  k.connect(Setter<int>(object));
  k = 42;
  qi::Future<int> result = object.future();
  EXPECT_EQ(42, result.value());
  EXPECT_EQ(42, k.get());
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
  k.connect(Setter<int>(object));
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
    qi::async<void>(Incrementer(counter));
  }

  EXPECT_TRUE(running.value());
  EXPECT_EQ(TARGET_COUNT, counter.get());
}
