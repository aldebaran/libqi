/*
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/os.hpp>

#include <qic/application.h>
#include <qic/object.h>
#include <qic/value.h>
#include <qic/future.h>

int res0 = 0;
int res1 = 0;
bool pass0 = false;
bool pass1 = false;

void signal_callback_0(qi_value_t* cont, void *)
{
  qi_value_t* val = qi_value_tuple_get(cont, 0);
  res0 = qi_value_get_int64_default(val, -1);
  printf("callback0: value: %d\n", res0);
  qi_value_destroy(val);
  pass0 = true;
}

void signal_callback_1(qi_value_t* cont, void *)
{
  qi_value_t* val = qi_value_tuple_get(cont, 0);
  res1 = qi_value_get_int64_default(val, -1);
  printf("callback1: value: %d\n", res1);
  qi_value_destroy(val);
  pass1 = true;
}

TEST(TestSignal, CreateAndDestroy)
{
  qi_object_builder_t* qiob =  qi_object_builder_create();
  qi_object_builder_register_event(qiob, "plouf::(s)");
  qi_object_builder_destroy(qiob);
}

qi_value_t *create_tup(int v) {
  qi_value_t *cont = qi_value_create("(i)");
  qi_value_t *val = qi_value_create("i");
  qi_value_set_int64(val, v);
  qi_value_tuple_set(cont, 0, val);
  qi_value_destroy(val);
  return cont;
}

TEST(TestSignal, SimpleSignalConnect)
{
  qi_object_builder_t* qiob =  qi_object_builder_create();
  qi_object_builder_register_event(qiob, "plouf::(i)");
  qi_object_t *object = qi_object_builder_get_object(qiob);
  qi_object_builder_destroy(qiob);

  unsigned long long link = qi_future_get_int64_default(qi_object_event_connect(object, "plouf::(i)", signal_callback_0, 0), 0);
  ASSERT_GE(link, 0u);
  int n = 42;
  qi_value_t *val = create_tup(n);

  pass0 = false;
  pass1 = false;
  ASSERT_NE(-1, qi_object_event_emit(object, "plouf::(i)", val));
  qi_value_destroy(val);
  int timeout = 1000;
  while (timeout > 0)
  {
    qi::os::msleep(1);
    if (pass0)
      break;
    timeout -= 1;
  }

  EXPECT_EQ(res0, n);
  qi_object_destroy(object);
}

TEST(TestSignal, MultipleSignalConnect)
{
  qi_object_builder_t* qiob =  qi_object_builder_create();
  qi_object_builder_register_event(qiob, "plouf::(i)");
  qi_object_t *object = qi_object_builder_get_object(qiob);
  qi_object_builder_destroy(qiob);

  qi_object_event_connect(object, "plouf::(i)", signal_callback_0, 0);
  qi_object_event_connect(object, "plouf::(i)", signal_callback_1, 0);

  int n = 21;
  qi_value_t* val = create_tup(n);
  pass0 = false;
  pass1 = false;

  ASSERT_NE(-1, qi_object_event_emit(object, "plouf::(i)", val));
  qi_value_destroy(val);
  int timeout = 1000;
  while (timeout > 0)
  {
    qi::os::msleep(1);
    if (pass0 && pass1)
      break;
    timeout -= 1;
  }

  EXPECT_EQ(res0, n);
  EXPECT_EQ(res1, n);
  qi_object_destroy(object);
}

TEST(TestSignal, SignalDisconnect)
{
  qi_object_builder_t* qiob =  qi_object_builder_create();
  qi_object_builder_register_event(qiob, "plouf::(i)");
  qi_object_t *object = qi_object_builder_get_object(qiob);
  qi_object_builder_destroy(qiob);

  qi_object_event_connect(object, "plouf::(i)", signal_callback_0, 0);
  unsigned long long l = qi_future_get_uint64_default(qi_object_event_connect(object, "plouf::(i)", signal_callback_1, 0), 0);
  qi_future_t* fut = qi_object_event_disconnect(object, l);
  if (qi_future_has_error(fut, QI_FUTURETIMEOUT_INFINITE)) {
    const char *errr = qi_future_get_error(fut);
    printf("future error: %s\n", errr);
    free((void*)errr);
  }
  EXPECT_TRUE(!qi_future_has_error(fut, QI_FUTURETIMEOUT_INFINITE));
  qi_future_destroy(fut);

  int n = 11;
  qi_value_t* val = create_tup(n);
  pass0 = false;
  pass1 = false;
  ASSERT_NE(-1, qi_object_event_emit(object, "plouf::(i)", val));

  int timeout = 1000;
  while (timeout > 0)
  {
    qi::os::msleep(1);
    if (pass0)
      break;
    timeout -= 1;
  }

  EXPECT_EQ(res0, n);
  EXPECT_EQ(pass1, false);
  qi_value_destroy(val);
  qi_object_destroy(object);
}


int main(int argc, char **argv) {
  qi_application_t*     app;
  app = qi_application_create(&argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  int ret = RUN_ALL_TESTS();
  qi_application_destroy(app);
  return ret;
}
