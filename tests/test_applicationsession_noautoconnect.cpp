/*
 ** Author(s):
 **  - Ly William Chhim <lwchhim@aldebaran-robotics.com>
 **
 ** Copyright (C) 2013 Aldebaran Robotics
 */

#include <gtest/gtest.h>

#include <qimessaging/applicationsession.hpp>
#include <qimessaging/servicedirectory.hpp>

static bool _stopped = false;
static char **_argv = 0;
static int _argc = 3;

void onStop()
{
  _stopped = true;
}

TEST(QiApplicationSessionNoAutoConnect, defaultConnect)
{
  qi::ServiceDirectory   sd;
  sd.listen("tcp://127.0.0.1:0");

  ASSERT_FALSE(qi::ApplicationSession::session().isConnected());
  qi::ApplicationSession::session().connect(sd.endpoints()[0]);
  ASSERT_TRUE(qi::ApplicationSession::session().isConnected());

  ASSERT_EQ(sd.endpoints()[0].str(), qi::ApplicationSession::session().url());

  ASSERT_FALSE(_stopped);
  sd.close();
  qi::os::msleep(100);
  ASSERT_TRUE(_stopped);
}

TEST(QiApplicationSessionNoAutoConnect, checkArgs)
{
  ASSERT_EQ(3, qi::ApplicationSession::argc());
  EXPECT_EQ(std::string("foo"), qi::ApplicationSession::argv()[0]);
  EXPECT_EQ(std::string("--address"), qi::ApplicationSession::argv()[1]);
  EXPECT_EQ(std::string("url"), qi::ApplicationSession::argv()[2]);

  ASSERT_EQ(3, _argc);
  EXPECT_EQ(std::string("foo"), _argv[0]);
  EXPECT_EQ(std::string("--address"), _argv[1]);
  EXPECT_EQ(std::string("url"), _argv[2]);
  EXPECT_EQ(0, _argv[3]);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  _argv = new char*[4];
  strcpy((_argv[0] = new char[4]), "foo");
  strcpy((_argv[1] = new char[10]), "--address");
  strcpy((_argv[2] = new char[10]), "url");
  _argv[3] = 0;
  qi::ApplicationSession app(_argc, _argv, qi::ApplicationSession_NoAutoConnect);
  app.atStop(&onStop);
  return RUN_ALL_TESTS();
}
