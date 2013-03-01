/*
 ** Author(s):
 **  - Laurent Lec <llec@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <gtest/gtest.h>

#include <qimessaging/session.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qi/application.hpp>
#include <testsession/testsessionpair.hpp>

int _argc = 0;
char** _argv = 0;

static std::string reply(const std::string &msg)
{
  return msg + "bim";
}

TEST(QiApplication, destroyAppBeforeObject)
{
  qi::ObjectPtr object;
  {
    qi::Application a(_argc, _argv);
    TestSessionPair pair;
    qi::GenericObjectBuilder ob;
    ob.advertiseMethod("reply", &reply);
    qi::ObjectPtr obj(ob.object());

    pair.server()->registerService("serviceTest", obj);
    object = pair.server()->service("serviceTest");
    std::string r = object->call<std::string>("reply", "lol");
  }

  ASSERT_TRUE(true);
}


int main(int argc, char** argv)
{
  _argc = argc;
  _argv = argv;
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  ::TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
