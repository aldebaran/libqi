#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <qi/future.hpp>
#include "test_future.hpp"

qiLogCategory("test");

int exchange(int& target, int value)
{
  int old = target;
  target = value;
  return old;
}

// wrap a call, waiting before, and notifying state
void wrap(boost::function<void()> op, int msDelay, std::atomic<int>& notify)
{
  notify = 1;
  if (msDelay)
    qi::os::msleep(msDelay);
  notify = 2;
  try {
    op();
  }
  catch (const qi::PointerLockException&)
  {}
  notify = 3;
}

TEST(TestBind, Simple)
{
  int v = 0;
  qi::bind<void(int)>(&exchange, std::ref(v), _1)(15);
  EXPECT_EQ(15, v);
  qi::bind<void(void)>(&exchange, std::ref(v), 16)();
  EXPECT_EQ(16, v);
  qi::bind(&exchange, std::ref(v), _1)(15);
  EXPECT_EQ(15, v);
  qi::bind(&exchange, std::ref(v), 16)();
  EXPECT_EQ(16, v);
}

TEST(TestBind, MemFun)
{
  std::atomic<int> v{0};
  SetValue s1(v);
  qi::bind<void(int)>(&SetValue::exchange, &s1, _1)(1);
  EXPECT_EQ(1, v);
  qi::bind<void(void)>(&SetValue::exchange, &s1, 2)();
  EXPECT_EQ(2, v);
  qi::bind<void(int)>(&SetValue::exchange, boost::ref(s1), _1)(3);
  EXPECT_EQ(3, v);
  qi::bind<void(void)>(&SetValue::exchange, boost::ref(s1), 4)();
  EXPECT_EQ(4, v);
}

TEST(TestBind, SharedPtr)
{
  std::atomic<int> v{0};
  boost::shared_ptr<SetValue> s(new SetValue(v));
  qi::bind<void(int)>(&SetValue::exchange, s, _1)(1);
  EXPECT_EQ(1, v);
  qi::bind<void(void)>(&SetValue::exchange, s, 2)();
  EXPECT_EQ(2, v);

  boost::function<void(void)> f =  qi::bind<void(void)>(&SetValue::exchange, s, 3);
  s.reset();
  f();
  EXPECT_EQ(3, v);
}

TEST(TestBind, WeakPtr)
{
  std::atomic<int> v{0};
  boost::shared_ptr<SetValue> s(new SetValue(v));
  boost::weak_ptr<SetValue> w(s);
  qi::bind<void(int)>(&SetValue::exchange, w, _1)(1);
  EXPECT_EQ(1, v);
  qi::bind<void(void)>(&SetValue::exchange, w, 2)();
  EXPECT_EQ(2, v);

  boost::function<void(void)> f =  qi::bind<void(void)>(&SetValue::exchange, w, 3);
  s.reset();
  EXPECT_ANY_THROW(f());
  EXPECT_EQ(2, v);
}

TEST(TestBind, Trackable)
{
  std::atomic<int> v{0};
  {
    SetValue2 s1(v);
    qi::bind<void(int)>(&SetValue2::exchange, &s1, _1)(1);
    EXPECT_EQ(1, v);
    qi::bind<void(void)>(&SetValue2::exchange, &s1, 2)();
    EXPECT_EQ(2, v);
    qi::bind<void(int)>(&SetValue2::exchange, boost::ref(s1), _1)(3);
    EXPECT_EQ(3, v);
    qi::bind<void(void)>(&SetValue2::exchange, boost::ref(s1), 4)();
    EXPECT_EQ(4, v);
  }
  v = 0;
  {
    SetValue2 s1(v);
    qi::bind(&SetValue2::exchange, &s1, _1)(1);
    EXPECT_EQ(1, v);
    qi::bind(&SetValue2::exchange, &s1, 2)();
    EXPECT_EQ(2, v);
    qi::bind(&SetValue2::exchange, boost::ref(s1), _1)(3);
    EXPECT_EQ(3, v);
    qi::bind(&SetValue2::exchange, boost::ref(s1), 4)();
    EXPECT_EQ(4, v);
  }

  boost::function<void(void)> f;
  {
    SetValue2 s1(v);
    f = qi::bind<void(void)>(&SetValue2::exchange, &s1, 5);
  }
  EXPECT_ANY_THROW(f()); // s1 is trackable, bound and deleted: callback not executed
  EXPECT_EQ(4, v);

  // check waiting behavior of destroy
  std::atomic<int> notify{0};
  qi::int64_t time;
  {
    SetValue2 s1(v);
    boost::thread(wrap,
      qi::bind<void(void)>(&SetValue2::delayExchange, &s1, 100, 10),
      0,
      std::ref(notify));
    time = qi::os::ustime();
    // wait enough for operation to start
    while (!notify)
      qi::os::msleep(10);
    qi::os::msleep(20);
    time = qi::os::ustime();
    // exit scope, deleting s1, which should block until operation terminates
  }
  time = qi::os::ustime() - time;
  EXPECT_GT(time, 60000); // 100 - 20 - 10 - margin
  EXPECT_EQ(10, v);

  // check disable-call behavior again in our more complex threaded setup
  {
    notify = 0;
    SetValue2 s1(v);
    boost::thread(wrap,
      qi::bind<void(void)>(&SetValue2::delayExchange, &s1, 100, 11),
      50,
      std::ref(notify));
  }
  while (notify != 3)
    qi::os::msleep(10);
  EXPECT_EQ(10, v); //call not made
}

TEST(TestBind, BindLambda)
{
  auto f = qi::bind<int>([](int i){ return i; }, 18);
  ASSERT_EQ(18, f());
}
