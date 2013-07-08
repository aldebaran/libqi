/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#include <qic/future.h>
#include <qic/value.h>

#include <qi/os.hpp> // that's cheating!
#include <gtest/gtest.h>

bool isCallbackCalled = false;

void future_callback_simple(qi_future_t* fut, void *data)
{
  qi_value_t *val = 0;
  if (qi_future_has_value(fut, QI_FUTURETIMEOUT_INFINITE))
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
  qi_future_destroy(fut);
}

TEST(TestFuture, SimpleType)
{
  isCallbackCalled = false;
  qi_value_t *val = qi_value_create("i");
  int answer = 42;
  qi_value_set_int64(val, answer);

  int res = 0;
  res = qi_value_get_int64_default(val, 0);
  EXPECT_EQ(42, res);

  qi_promise_t* promise = qi_promise_create(1);
  qi_future_t*  future = qi_promise_get_future(promise);

  qi_future_add_callback(future, future_callback_simple, 0);

  ASSERT_FALSE(qi_future_is_finished(future));
  qi_promise_set_value(promise, val);
  EXPECT_EQ(1, qi_future_is_finished(future));
  qi_value_destroy(val);
  qi_future_wait(future, QI_FUTURETIMEOUT_INFINITE);
  ASSERT_EQ(1, qi_future_is_finished(future));

  qi_value_t *rest = qi_future_get_value(future);
  EXPECT_EQ(42, qi_value_get_int64_default(rest, 0));
  // WaIt for callback
  for (unsigned i=0; i<50 && !isCallbackCalled; ++i)
    qi::os::msleep(10);
  EXPECT_TRUE(isCallbackCalled);
  qi_value_destroy(rest);
  qi_future_destroy(future);
  qi_promise_destroy(promise);
}

TEST(TestFuture, Error)
{
  qi_promise_t* promise = qi_promise_create(true);
  qi_future_t*  future = qi_promise_get_future(promise);

  qi_promise_set_error(promise, "it's friday");
  qi_future_wait(future, QI_FUTURETIMEOUT_INFINITE);

  ASSERT_EQ(1, qi_future_has_error(future, QI_FUTURETIMEOUT_INFINITE));
  ASSERT_EQ(1, qi_future_is_finished(future));

  const char *s = qi_future_get_error(future);
  std::string error(s);
  free((void*)s);
  ASSERT_TRUE(error.compare("it's friday") == 0);

  qi_future_destroy(future);
  qi_promise_destroy(promise);
}

TEST(TestFuture, NotCancelable)
{
  qi_promise_t* promise = qi_promise_create(true);
  qi_future_t*  future = qi_promise_get_future(promise);
  ASSERT_FALSE(qi_future_is_cancelable(future));
  qi_future_destroy(future);
  qi_promise_destroy(promise);
}

void onError(qi_promise_t *prom, void *) {
  qi_promise_set_error(prom, "plouf");
}

void onValue(qi_promise_t *prom, void *) {
  qi_value_t *val = qi_value_create("s");
  qi_promise_set_value(prom, val);
  qi_value_destroy(val);
}

void onCancel(qi_promise_t *prom, void *) {
  qi_promise_set_canceled(prom);
}

TEST(TestFuture, CancelableError)
{
  qi_promise_t* promise = qi_promise_cancelable_create(0, onError, 0);
  qi_future_t*  future = qi_promise_get_future(promise);
  ASSERT_TRUE(qi_future_is_cancelable(future));
  qi_future_cancel(future);

  ASSERT_TRUE(qi_future_has_error(future, QI_FUTURETIMEOUT_INFINITE));
  ASSERT_TRUE(qi_future_is_finished(future));
  ASSERT_FALSE(qi_future_is_canceled(future));
  qi_future_destroy(future);
  qi_promise_destroy(promise);
}

TEST(TestFuture, CancelableValue)
{
  qi_promise_t* promise = qi_promise_cancelable_create(0, onValue, 0);
  qi_future_t*  future = qi_promise_get_future(promise);
  ASSERT_TRUE(qi_future_is_cancelable(future));
  qi_future_cancel(future);

  ASSERT_TRUE(qi_future_has_value(future, QI_FUTURETIMEOUT_INFINITE));
  ASSERT_TRUE(qi_future_is_finished(future));
  ASSERT_FALSE(qi_future_is_canceled(future));
  qi_future_destroy(future);
  qi_promise_destroy(promise);
}

TEST(TestFuture, CancelableCancel)
{
  qi_promise_t* promise = qi_promise_cancelable_create(0, onCancel, 0);
  qi_future_t*  future = qi_promise_get_future(promise);
  ASSERT_TRUE(qi_future_is_cancelable(future));
  qi_future_cancel(future);

  ASSERT_FALSE(qi_future_has_error(future, QI_FUTURETIMEOUT_INFINITE));
  ASSERT_FALSE(qi_future_has_value(future, QI_FUTURETIMEOUT_INFINITE));
  ASSERT_TRUE(qi_future_is_finished(future));
  ASSERT_TRUE(qi_future_is_canceled(future));
  qi_future_destroy(future);
  qi_promise_destroy(promise);
}

TEST(TestFuture, IsCallbackCalled)
{
  EXPECT_TRUE(isCallbackCalled);
}
