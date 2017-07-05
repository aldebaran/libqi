/**
** Copyright (C) 2016 Aldebaran Robotics
*/

#include <functional>
#include <gtest/gtest.h>
#include <qi/actor.hpp>
#include <qi/async.hpp>
#include <qi/clock.hpp>
#include <iostream>

namespace
{
const auto usualTimeout = qi::MilliSeconds(500);

std::atomic<bool> gExecuted{false};
void executeGlobal(){ gExecuted = true; }
void execute(std::atomic<bool>& executed){ executed = true; }

class TestAsync : public ::testing::Test
{
public:
  void SetUp() override
  {
    gExecuted = false;
  }

  void TearDown() override
  {
    ASSERT_EQ(qi::FutureState_FinishedWithValue, executing.wait(usualTimeout));
    ASSERT_TRUE(gExecuted);
  }

  qi::Future<void> executing;
};
} // anonymous

TEST_F(TestAsync, asyncLambda)
{
  executing = qi::async([&]{gExecuted = true;});
}

TEST_F(TestAsync, asyncFreeFunction)
{
  executing = qi::async(executeGlobal);
}

TEST_F(TestAsync, asyncStdBoundFunction)
{
  executing = qi::async(std::bind(execute, std::ref(gExecuted)));
}

TEST_F(TestAsync, asyncBoostBoundFunction)
{
  executing = qi::async(boost::bind(execute, std::ref(gExecuted)));
}

TEST_F(TestAsync, asyncQiBoundFunction)
{
  executing = qi::async(qi::bind(execute, std::ref(gExecuted)));
}

TEST_F(TestAsync, asyncAtSimple)
{
  qi::SteadyClock::time_point tp(qi::SteadyClock::now()+qi::MilliSeconds(1));
  executing = qi::asyncAt(&executeGlobal, tp);
}

TEST_F(TestAsync, asyncDelaySimple)
{
  executing = qi::asyncDelay(&executeGlobal, qi::MilliSeconds(1));
}

// Let's try with a famous actor
class JohnnyDepp: public qi::Actor
{
public:
  bool calledFromStrand() { return strand()->isInThisContext(); }
};

TEST(TestAsyncActor, asyncActorMethod)
{
  JohnnyDepp actor;
  ASSERT_TRUE(qi::async(qi::bind(&JohnnyDepp::calledFromStrand, &actor)).value());
}
