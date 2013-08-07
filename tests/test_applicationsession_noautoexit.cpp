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
static qi::ServiceDirectory _sd;
static char **_argv = 0;
static int _argc = 3;

void onStop()
{
  _stopped = true;
}

TEST(QiApplicationSessionNoAutoExit, defaultConnect)
{
  ASSERT_TRUE(qi::ApplicationSession::session().isConnected());

  ASSERT_EQ(_sd.endpoints()[0].str(), qi::ApplicationSession::session().url());

  ASSERT_FALSE(_stopped);
  _sd.close();
  qi::os::msleep(100);
  ASSERT_FALSE(_stopped);
}

TEST(QiApplicationSessionNoAutoConnect, checkArgs)
{
  ASSERT_EQ(3, qi::ApplicationSession::argc());
  EXPECT_EQ(std::string("no"), qi::ApplicationSession::argv()[0]);
  EXPECT_EQ(std::string("options"), qi::ApplicationSession::argv()[1]);
  EXPECT_EQ(std::string("given"), qi::ApplicationSession::argv()[2]);

  ASSERT_EQ(3, _argc);
  EXPECT_EQ(std::string("no"), _argv[0]);
  EXPECT_EQ(std::string("options"), _argv[1]);
  EXPECT_EQ(std::string("given"), _argv[2]);
  EXPECT_EQ(0, _argv[3]);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  _sd.listen("tcp://127.0.0.1:0");
  _argv = new char*[4];
  strcpy((_argv[0] = new char[4]), "no");
  strcpy((_argv[1] = new char[10]), "options");
  strcpy((_argv[2] = new char[10]), "given");
  _argv[3] = 0;

  qi::ApplicationSession app(_argc, _argv, qi::ApplicationSession_NoAutoExit, _sd.endpoints()[0]);
  app.atStop(&onStop);
  return RUN_ALL_TESTS();
}
