/*
 ** Author(s):
 **  - Ly William Chhim <lwchhim@aldebaran-robotics.com>
 **
 ** Copyright (C) 2013 Aldebaran Robotics
 */

#include <gtest/gtest.h>

#include <qi/applicationsession.hpp>
#include <thread>
#include <chrono>

static bool _stopped = false;
static qi::ApplicationSession* _app = nullptr;
static qi::Session* _sd = nullptr;

void onStop()
{
  _stopped = true;
}

TEST(QiApplicationSessionNoAutoExit, defaultConnect)
{
  ASSERT_FALSE(_app->session()->isConnected());
  _app->startSession();
  ASSERT_TRUE(_app->session()->isConnected());
  ASSERT_EQ(_sd->endpoints()[0].str(), _app->session()->url());

  ASSERT_FALSE(_stopped);
  _sd->close();
  std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
  ASSERT_FALSE(_stopped);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  auto sd = qi::makeSession();
  auto scopedSd = ka::scoped_set_and_restore(_sd, sd.get());
  sd->listenStandalone("tcp://127.0.0.1:0");

  argc = 0; // no option given
  qi::ApplicationSession app(argc, argv, qi::ApplicationSession::Option_NoAutoExit,
                             sd->endpoints()[0]);
  auto scopedApp = ka::scoped_set_and_restore(_app, &app);
  app.atStop(&onStop);

  auto res = RUN_ALL_TESTS();

  // We must explicitly close the standalone session before destroying the app, otherwise it might
  // use resources that are owned by the app.
  sd->close();

  return res;
}
