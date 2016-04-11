/**
** Copyright (C) 2016 Aldebaran Robotics
*/

#include <functional>
#include <gtest/gtest.h>
#include <qi/actor.hpp>
#include <qi/async.hpp>

namespace
{
const auto usualTimeout = qi::MilliSeconds(500);
} // anonymous

TEST(TestAsync, asyncLambda)
{
  bool executed = false;
  auto executing = qi::async([&executed]{executed = true;});
  ASSERT_EQ(qi::FutureState_FinishedWithValue, executing.wait(usualTimeout));
  ASSERT_TRUE(executed);
}

bool gExecuted = false;
void executeGlobal(){ gExecuted = true; }
void execute(bool& executed){ executed = true; }

TEST(TestAsync, asyncFreeFunction)
{
  auto executing = qi::async(executeGlobal);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, executing.wait(usualTimeout));
  ASSERT_TRUE(gExecuted);
}

TEST(TestAsync, asyncStdBoundFunction)
{
  bool executed = false;
  auto executing = qi::async(std::bind(execute, std::ref(executed)));
  ASSERT_EQ(qi::FutureState_FinishedWithValue, executing.wait(usualTimeout));
  ASSERT_TRUE(gExecuted);
}

TEST(TestAsync, asyncBoostBoundFunction)
{
  bool executed = false;
  auto executing = qi::async(boost::bind(execute, std::ref(executed)));
  ASSERT_EQ(qi::FutureState_FinishedWithValue, executing.wait(usualTimeout));
  ASSERT_TRUE(gExecuted);
}

TEST(TestAsync, asyncQiBoundFunction)
{
  bool executed = false;
  auto executing = qi::async(qi::bind(execute, std::ref(executed)));
  ASSERT_EQ(qi::FutureState_FinishedWithValue, executing.wait(usualTimeout));
  ASSERT_TRUE(gExecuted);
}

// Let's try with a famous actor
class JohnnyDepp: public qi::Actor
{
public:
  bool calledFromStrand() { return strand()->isInThisContext(); }
};

TEST(TestAsync, asyncActorMethod)
{
  JohnnyDepp actor;
  ASSERT_TRUE(qi::async(qi::bind(&JohnnyDepp::calledFromStrand, &actor)).value());
}
