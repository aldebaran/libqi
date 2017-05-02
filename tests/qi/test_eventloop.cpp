#include <condition_variable>
#include <mutex>
#include <gtest/gtest.h>
#include <qi/eventloop.hpp>
#include "test_future.hpp"

int ping(int v)
{
  if (v>= 0)
    return v;
  else
    throw std::runtime_error("Invalid argument ");
}

TEST(EventLoop, EventLoopCanPostWithDuration)
{
  std::mutex m;
  std::condition_variable cv;

  auto cb = [&]
  {
    std::unique_lock<std::mutex> l{m};
    cv.notify_one();
  };

  qi::EventLoop loop;
  loop.start(1);
  {
    std::unique_lock<std::mutex> l{m};
    loop.post(cb, qi::MilliSeconds{1});
    ASSERT_EQ(std::cv_status::no_timeout, cv.wait_for(l, std::chrono::milliseconds{100}));
  }
  loop.stop();
  loop.join();
}

TEST(EventLoop, EventLoopCanAsyncDelay)
{
  qi::EventLoop loop;
  loop.start(1);
  loop.asyncDelay([]{}, qi::MilliSeconds{1}).value(100);
  loop.stop();
  loop.join();
}

TEST(EventLoop, asyncNoop)
{
  qi::async([]{}).value(100);
}

TEST(EventLoop, async)
{
  static const qi::MilliSeconds SMALL_CALL_DELAY{ 20 };
  static const qi::MilliSeconds BIG_CALL_DELAY{ 200 };
  static const int VALID_VALUE = 42;
  static const auto makeValidValue = [] { return 42; };
  static const auto makeError = [] { throw std::runtime_error("Voluntary Fail"); };

  qi::EventLoop& el = *qi::getEventLoop();
  {
    auto f = el.asyncDelay(makeValidValue, BIG_CALL_DELAY);
    EXPECT_FALSE(f.isFinished());
    f.wait();
    EXPECT_FALSE(f.hasError());
    EXPECT_EQ(f.value(), VALID_VALUE);
  }

  {
    auto f = el.asyncDelay(makeValidValue, BIG_CALL_DELAY);
    EXPECT_FALSE(f.isFinished());
    EXPECT_NO_THROW(f.cancel());
    EXPECT_EQ(f.wait(), qi::FutureState_Canceled);
  }

  {
    auto f = el.asyncDelay(makeError, BIG_CALL_DELAY);
    EXPECT_FALSE(f.isFinished());
    f.wait();
    EXPECT_TRUE(f.hasError());
  }

  // We cannot guarantee the minimum delay that an async call will take, but we can guarantee that it will
  // be systematically after the specified delay.

  {
    const auto beginTime = qi::SteadyClock::now();
    auto callTime = beginTime;
    auto f = el.asyncDelay([&]{ callTime = qi::SteadyClock::now(); }, SMALL_CALL_DELAY);
    f.wait(); // This test will timeout if it's not called in a reasonable time
    EXPECT_TRUE(f.isFinished());
    const auto timeUntilCall = callTime - beginTime;
    EXPECT_TRUE(timeUntilCall >= SMALL_CALL_DELAY);
  }

  {
    const auto beginTime = qi::SteadyClock::now();
    auto callTime = beginTime;
    auto f = el.asyncAt([&] { callTime = qi::SteadyClock::now(); }, qi::SteadyClock::now() + SMALL_CALL_DELAY);
    f.wait(); // This test will timeout if it's not called in a reasonable time
    EXPECT_TRUE(f.isFinished());
    const auto timeUntilCall = callTime - beginTime;
    EXPECT_TRUE(timeUntilCall >= SMALL_CALL_DELAY);
  }

  qi::async<void>([]{}).value();
}

TEST(EventLoop, asyncFast)
{
  qi::EventLoop* el = qi::getEventLoop();
  for (int i = 0; i < 10; ++i)
  {
    qi::Future<int> f = el->async(get42);
    f.wait();
  }
}
