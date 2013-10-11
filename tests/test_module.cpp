/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>

#include <qi/application.hpp>

#include <qitype/anyobject.hpp>
#include <qimessaging/session.hpp>
#include <qitype/objectfactory.hpp>

qi::Session* session;
TEST(Module, Load)
{
  session->loadService("naoqi/testmodule");

  qi::AnyObject o = session->service("test");
  ASSERT_TRUE(o);
  int res = o.call<int>("testMethod", 12);
  ASSERT_EQ(13, res);
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  qi::Session s;

  s.listenStandalone("tcp://127.0.0.1:0");
  session = &s;
  return RUN_ALL_TESTS();
}

