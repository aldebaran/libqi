/*
** Author(s):
**  - Guillaume OREAL <goreal@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/
#include <qic/session.h>
#include <qic/application.h>
#include <gtest/gtest.h>
#include <qi/os.hpp>
#include <boost/thread/thread.hpp>
int myargc;
char ** myargv;

void app_stop(qi_application_t* app)
{
  qi_application_stop(app);
  return;
}

void app_run(qi_application_t* app)
{
  qi_application_run(app);
  return;
}

TEST(testapplication, basictest)
{
  qi_application_t*     app = qi_application_create(&myargc, myargv);

  boost::thread myThd(boost::bind(&app_run, app));
  qi::os::msleep(1);
  boost::thread myThd2(boost::bind(&app_stop, app));

  myThd.join();
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  myargc = argc;
  myargv = argv;

  return RUN_ALL_TESTS();
}
