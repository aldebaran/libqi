/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/c/api_c.h>
#include <qimessaging/c/qi_c.h>
#include <gtest/gtest.h>
#include <string>

bool isCallbackCalled = false;

void future_callback_simple(const void* value, char success, void *data)
{
  int *result = (int *) value;

  EXPECT_EQ(42, *result);
  EXPECT_TRUE((bool) success);

  isCallbackCalled = true;
}

TEST(TestFuture, SimpleType)
{
  int answer = 42;
  qi_promise_t* promise = qi_promise_create();
  qi_future_t*  future = qi_promise_get_future(promise);

  qi_future_set_callback(future, future_callback_simple, 0);

  ASSERT_FALSE(qi_future_is_ready(future));
  qi_promise_set_value(promise, &answer);
  EXPECT_TRUE((bool) qi_future_is_ready(future));

  qi_future_wait(future);
  ASSERT_TRUE((bool) qi_future_is_ready(future));

  int *result = (int *) qi_future_get_value(future);
  EXPECT_EQ(42, *result);

  qi_future_destroy(future);
  qi_promise_destroy(promise);
}

TEST(TestFuture, Error)
{
  qi_promise_t* promise = qi_promise_create();
  qi_future_t*  future = qi_promise_get_future(promise);

  qi_promise_set_error(promise, "it's friday");
  qi_future_wait(future);

  ASSERT_TRUE((bool) qi_future_is_error(future));
  ASSERT_TRUE((bool) qi_future_is_ready(future));

  std::string error(qi_future_get_error(future));
  ASSERT_TRUE(error.compare("it's friday") == 0);

  qi_future_destroy(future);
  qi_promise_destroy(promise);
}

TEST(TestFuture, IsCallbackCalled)
{
  EXPECT_TRUE(isCallbackCalled);
}
