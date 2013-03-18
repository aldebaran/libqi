/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#include <qic/future.h>
#include <qic/value.h>
#include <gtest/gtest.h>

bool isCallbackCalled = false;

void future_callback_simple(qi_future_t* fut, void *data)
{
  qi_value_t *val = 0;
  if (qi_future_is_ready(fut) && !qi_future_has_error(fut))
    val = qi_future_get_value(fut);
  if (!val) {
    printf("Value is callback is empty\n");
    return;
  }

  int result = 0;
  if (val)
    result = qi_value_get_int64_default(val, -1);
  EXPECT_EQ(42, result);
  isCallbackCalled = true;
  qi_value_destroy(val);
}

TEST(TestFuture, SimpleType)
{
  qi_value_t *val = qi_value_create("i");
  int answer = 42;
  qi_value_set_int64(val, answer);

  int res = 0;
  res = qi_value_get_int64_default(val, 0);
  EXPECT_EQ(42, res);

  qi_promise_t* promise = qi_promise_create();
  qi_future_t*  future = qi_promise_get_future(promise);

  qi_future_add_callback(future, future_callback_simple, 0);

  ASSERT_FALSE(qi_future_is_ready(future));
  qi_promise_set_value(promise, val);
  EXPECT_EQ(1, qi_future_is_ready(future));
  qi_value_destroy(val);
  qi_future_wait(future);
  ASSERT_EQ(1, qi_future_is_ready(future));

  qi_value_t *rest = qi_future_get_value(future);
  EXPECT_EQ(42, qi_value_get_int64_default(rest, 0));

  qi_value_destroy(rest);
  qi_future_destroy(future);
  qi_promise_destroy(promise);
}

TEST(TestFuture, Error)
{
  qi_promise_t* promise = qi_promise_create();
  qi_future_t*  future = qi_promise_get_future(promise);

  qi_promise_set_error(promise, "it's friday");
  qi_future_wait(future);

  ASSERT_EQ(1, qi_future_has_error(future));
  ASSERT_EQ(1, qi_future_is_ready(future));

  std::string error(qi_future_get_error(future));
  ASSERT_TRUE(error.compare("it's friday") == 0);

  qi_future_destroy(future);
  qi_promise_destroy(promise);
}

TEST(TestFuture, IsCallbackCalled)
{
  EXPECT_TRUE(isCallbackCalled);
}
