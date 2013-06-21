/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qitype/anyobject.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsessionpair.hpp>

qiLogCategory("test");

void test_service(const qi::AnyObject &o)
{
  ASSERT_TRUE(o);
  int res = o->call<int>("testMethod", 12);
  ASSERT_EQ(13, res);
}

TEST(Module, pass_obj)
{
  TestMode::forceTestMode(TestMode::Mode_SD);
  TestSessionPair p;

  qi::Session* s = p.server();

  s->loadService("naoqi/testmodule");
  qi::AnyObject o = s->service("test");

  qiLogInfo() << "check first service" << std::endl;
  test_service(o);

  qi::AnyObject o2 = o->call<qi::AnyObject>("make_other_object");

  s->registerService("test_bis", o2);

  std::cout << "check second service" << std::endl;
  test_service(o2);
  qi::AnyObject o3 = s->service("test_bis");
  test_service(o3);

  s->close();
}

TEST(Module, load_received_obj)
{
  TestMode::forceTestMode(TestMode::Mode_SD);
  TestSessionPair p;

  qi::Session* s = p.server();

  s->loadService("naoqi/testmodule");
  qi::AnyObject o = s->service("test");
  test_service(o);

  s->registerService("test_bis", o);
  qi::AnyObject o2 = s->service("test_bis");
  test_service(o2);

  s->close();
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
