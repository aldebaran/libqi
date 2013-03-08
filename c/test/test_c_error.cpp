/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qic/error.h>
#include <qic/session.h>
#include <qic/value.h>
#include <qic/future.h>
#include <qic/servicedirectory.h>
#include <qic/object.h>
#include <gtest/gtest.h>


TEST(TestCError, IsErrorNullBeforeSetError)
{
  ASSERT_TRUE(qi_c_error() == 0);
}

TEST(TestCError, TestNoSD)
{
  qi_session_t* session = qi_session_create();
  ASSERT_FALSE(qi_future_get_int64(qi_session_connect(session, "tcp://127.0.0.1:55"), 0));
  /* CXX backend just return false and does not throw, so no error
  const char*   error = qi_c_error();
  ASSERT_NE(error, (char *) 0);
  ASSERT_EQ(::strcmp(error, "Connection error: Connection refused"), 0);
  */
  qi_session_destroy(session);
}

TEST(TestCError, TestNoService)
{
  qi_servicedirectory_t* sd;
  qi_session_t*          session;

  sd = qi_servicedirectory_create();
  qi_servicedirectory_listen(sd, "tcp://0.0.0.0:0");

  qi_value_t *val = qi_servicedirectory_endpoints(sd);
  qi_value_t *ele = qi_value_list_get(val, 0);
  const char *url = qi_value_get_string(ele);

  printf("connecting to: %s\n", url);

  session = qi_session_create();
  qi_future_t* fuc = qi_session_connect(session, url);
  ASSERT_FALSE(qi_future_has_error(fuc));
  qi_future_destroy(fuc);
  free((void*)url);
  qi_value_destroy(ele);
  qi_value_destroy(val);

  qi_future_t *fu = qi_session_get_service(session, "tiptop");
  qi_value_t* fuv = qi_future_get_value(fu);
  qi_object_t* obj = qi_value_get_object(fuv);
  ASSERT_TRUE(obj == (qi_object_t*) 0);
  qi_object_destroy(obj);
  qi_value_destroy(fuv);

  const char*  error = qi_future_get_error(fu);
  ASSERT_NE(error, (char *) 0);
  ASSERT_EQ(::strcmp(error, "Cannot find service 'tiptop' in index"), 0);
  qi_future_destroy(fu);
  qi_servicedirectory_destroy(sd);
  qi_session_destroy(session);
}
