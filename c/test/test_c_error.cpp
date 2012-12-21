/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/c/api_c.h>
#include <qimessaging/c/error_c.h>
#include <qimessaging/c/qi_c.h>
#include <qi/os.hpp>
#include <gtest/gtest.h>
#include <qimessaging/servicedirectory.hpp>
#include <cstring>
#include <iostream>

TEST(TestCError, IsErrorNullBeforeSetError)
{
  ASSERT_TRUE(qi_c_error() == 0);
}

TEST(TestCError, TestNoSD)
{
  qi_session_t* session = qi_session_create();
  ASSERT_FALSE(qi_session_connect(session, "tcp://127.0.0.1:55"));
  /* CXX backend just return false and does not throw, so no error
  const char*   error = qi_c_error();
  ASSERT_NE(error, (char *) 0);
  ASSERT_EQ(::strcmp(error, "Connection error: Connection refused"), 0);
  */
}

TEST(TestCError, TestNoService)
{
  qi::ServiceDirectory sd;
  qi_session_t*        session;

  sd.listen("tcp://127.0.0.1:0");

  session = qi_session_create();
  ASSERT_TRUE(qi_session_connect(session, sd.listenUrl().str().c_str()));

  qi_object_t* obj = qi_session_get_service(session, "tiptop");
  ASSERT_TRUE(obj == (qi_object_t*) 0);

  const char*  error = qi_c_error();
  ASSERT_NE(error, (char *) 0);
  ASSERT_EQ(::strcmp(error, "Cannot find service 'tiptop' in index"), 0);
}
