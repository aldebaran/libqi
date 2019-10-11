/*
**
** Copyright (C) 2014, Aldebaran Robotics
*/

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <qi/log.hpp>
#include <qi/futuregroup.hpp>
#include <qi/application.hpp>
#include <qi/eventloop.hpp>

#include <gtest/gtest.h>

#include <thread>
#include <chrono>

#include <ka/macro.hpp>
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4355, )

qiLogCategory("test_futuregroup");

namespace {

  void infiniteTask(qi::Promise<void> promise)
  {
    while (!promise.isCancelRequested())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds{ 2 });
    }
    promise.setCanceled();
  }

  static boost::mutex random_mutex;
  static boost::random::mt19937 rng;

  template<int minValue, int maxValue>
  int random_number()
  {
    boost::mutex::scoped_lock lock(random_mutex);
    static boost::random::uniform_int_distribution<int> random(minValue, maxValue);
    return random(rng);
  }

  void variableTask(qi::Promise<void> promise)
  {
    int iterations = random_number<1, 10>();
    while (iterations)
    {
      --iterations;
      std::this_thread::sleep_for(std::chrono::milliseconds{ 300 });
      if (promise.isCancelRequested())
      {
        promise.setCanceled();
        return;
      }
    }
    promise.setValue(0);
  }

  template<class TaskFunc>
  qi::Future<void> launchTask(TaskFunc task)
  {
    qi::Promise<void> promise;
    qi::Future<void> future = promise.future();
    qi::getEventLoop()->post(boost::bind(task, promise));
    return future;
  }

}


TEST(TestScopedFutureGroup, cancelAddedFutures)
{
  using FutureList = std::vector<qi::Future<void>>;
  FutureList futures;
  qi::ScopedFutureGroup group;
  for (int i = 0; i < 10; ++i)
  {
    futures.push_back(launchTask(infiniteTask));
    group.add(futures.back());
  }

  EXPECT_FALSE(group.empty());
  EXPECT_EQ(futures.size(), group.size());

  group.cancelAll();
  qi::waitForAll(futures);
  EXPECT_TRUE(group.empty());
}

TEST(TestScopedFutureGroup, cancelOnScopeExit)
{
  using FutureList = std::vector<qi::Future<void>>;
  FutureList futures;

  {
    qi::ScopedFutureGroup group;
    for (int i = 0; i < 10; ++i)
    {
      futures.push_back(launchTask(infiniteTask));
      group.add(futures.back());
    }

    EXPECT_FALSE(group.empty());
    EXPECT_EQ(futures.size(), group.size());
  }

  qi::waitForAll(futures);
}

TEST(TestScopedFutureGroup, cancelWhileProcessing)
{
  using FutureList = std::vector<qi::Future<void>>;
  FutureList futures;

  {
    qi::ScopedFutureGroup group;
    for (int i = 0; i < 10; ++i)
    {
      futures.push_back(launchTask(variableTask));
      group.add(futures.back());
    }

    EXPECT_FALSE(group.empty());
    EXPECT_EQ(futures.size(), group.size());
    std::this_thread::sleep_for(std::chrono::seconds{ 1 });
  }

  qi::waitForAll(futures);
}

KA_WARNING_POP()
