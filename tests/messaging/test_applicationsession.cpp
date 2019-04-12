/*
 ** Author(s):
 **  - Ly William Chhim <lwchhim@aldebaran-robotics.com>
 **
 ** Copyright (C) 2013 Aldebaran Robotics
 */

#include <array>

#include <gtest/gtest.h>

#include <qi/applicationsession.hpp>
#include <qi/future.hpp>

static bool _stopped = false;
static qi::Session* _sd = nullptr;
static qi::ApplicationSession* _app = nullptr;
static char** _argv = nullptr;
static int _argc;
static std::string _url;
static qi::Promise<void> _sync;

void onStop()
{
  _stopped = true;
  _sync.setValue(nullptr);
}

TEST(QiApplicationSession, defaultConnect)
{
  ASSERT_FALSE(_app->session()->isConnected());
  _app->startSession();
  ASSERT_TRUE(_app->session()->isConnected());

  ASSERT_EQ(qi::Url(_url), _app->session()->url());

  ASSERT_FALSE(_stopped);
  _sd->close();
  _sync.future().wait();
  ASSERT_TRUE(_stopped);

  EXPECT_THROW(_app->session()->connect("ftp://invalidurl:42"),
               qi::FutureUserException);
  ASSERT_FALSE(_app->session()->isConnected());
}

TEST(QiApplicationSession, checkArgs)
{
  EXPECT_EQ(1, _app->argc());
  EXPECT_EQ(std::string("foo"), _app->argv()[0]);

  EXPECT_EQ(1, _argc);
  EXPECT_EQ(std::string("foo"), _argv[0]);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  auto sd = qi::makeSession();
  auto scopedSd = ka::scoped_set_and_restore(_sd, sd.get());
  sd->listenStandalone("tcp://127.0.0.1:0");
  _url = sd->endpoints()[0].str();

  std::array<char*, 5> args {
    const_cast<char*>("foo"),
    const_cast<char*>("--qi-url"),
    const_cast<char*>(_url.c_str()),
    const_cast<char*>("--qi-listen-url"),
    const_cast<char*>("tcp://localhost:0"),
  };
  _argc = static_cast<int>(args.size());
  auto scopedArgv = ka::scoped_set_and_restore(_argv, args.data());

  qi::log::addFilter("qi.application*", qi::LogLevel_Debug);

  qi::ApplicationSession app(
        _argc, _argv,
        qi::ApplicationSession::Config()
            .setOption(qi::ApplicationSession::Option_None)
            .setConnectUrl("tcp://127.0.0.1:9559")
            .setListenUrls({ "tcp://127.0.0.1:9559" }));
  auto scopedApp = ka::scoped_set_and_restore(_app, &app);
  app.atStop(&onStop);

  auto res = RUN_ALL_TESTS();

  // We must explicitly close the standalone session before destroying the app, otherwise it might
  // use resources that are owned by the app.
  sd->close();

  return res;
}
