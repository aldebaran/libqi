/*
 ** Author(s):
 **  - Cedric GESTES <cgestes@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <vector>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <qimessaging/session.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/gateway.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <boost/thread.hpp>
#include <testsession/testsessionpair.hpp>

static std::string reply(const std::string &msg)
{
  return msg;
}

qi::ObjectPtr newObject() {
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  return ob.object();
}

void alternateModule(qi::Session *session) {
  while (!boost::this_thread::interruption_requested())
  {
    boost::this_thread::interruption_point();
    //a--;
    qi::ObjectPtr obj = newObject();
    qi::Future<unsigned int> fut = session->registerService("TestToto", obj);
    if (fut.hasError()) {
      std::cout << "Error registering service: " << fut.error() << std::endl;
      continue;
    }
    std::cout << "Service TestToto registered" << std::endl;
    qi::Future<void> futun = session->unregisterService(fut.value());
    if (futun.hasError()) {
      std::cout << "Error unregistering service: " << futun.error() << std::endl;
      continue;
    }
    std::cout << "Service TestToto unregistered" << std::endl;
    qi::os::msleep(50);
  }
}


TEST(QiSession, RegisterUnregisterTwoSession)
{
  int a = 1000;
  TestSessionPair p;
  EXPECT_TRUE(p.client()->isConnected());

  boost::thread worker(boost::bind(&alternateModule, p.server()));
  while (a) {
    a--;
    qi::Future<qi::ObjectPtr> fut = p.client()->service("TestToto");
    if (fut.hasError()) {
      std::cout << "Call error:" << fut.error();
      continue;
    }
    std::string ret = fut.value()->call<std::string>("reply", "plif");
    std::cout << "ret:" << ret << std::endl;
  }
  worker.interrupt();
  worker.join();
}


TEST(QiSession, RegisterUnregisterSameSession)
{
  int a = 1000;
  TestSessionPair p;
  EXPECT_TRUE(p.client()->isConnected());

  boost::thread worker(boost::bind(&alternateModule, p.server()));
  while (a) {
    a--;
    qi::Future<qi::ObjectPtr> fut = p.server()->service("TestToto");
    if (fut.hasError()) {
      std::cout << "Call error:" << fut.error() << std::endl;
      continue;
    }
    std::string ret = fut.value()->call<std::string>("reply", "plif");
    std::cout << "ret:" << ret << std::endl;
  }
  worker.interrupt();
  worker.join();
}

TEST(QiSession, ConnectToMultipleConstellation)
{
  TestSessionPair constellation1;
  TestSessionPair constellation2;
  TestSessionPair constellation3;
  qi::Session     traveler;

  qi::ObjectPtr obj = newObject();
  constellation1.server()->registerService("test1", obj);
  constellation2.server()->registerService("test2", obj);
  constellation3.server()->registerService("test3", obj);

  ASSERT_TRUE(traveler.connect(constellation1.serviceDirectoryEndpoints()[0].str()));
  qi::ObjectPtr proxy = constellation1.server()->service("test1");
  std::string res = proxy->call<std::string>("reply", "plaf");
  ASSERT_TRUE(res.compare("plaf") == 0);
  traveler.close();

  ASSERT_TRUE(traveler.connect(constellation2.serviceDirectoryEndpoints()[0].str()));
  proxy = constellation1.server()->service("test2");
  res = proxy->call<std::string>("reply", "plaf");
  ASSERT_TRUE(res.compare("plaf") == 0);
  traveler.close();

  ASSERT_TRUE(traveler.connect(constellation3.serviceDirectoryEndpoints()[0].str()));
  proxy = constellation1.server()->service("test3");
  res = proxy->call<std::string>("reply", "plaf");
  ASSERT_TRUE(res.compare("plaf") == 0);
  traveler.close();
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  ::testing::InitGoogleTest(&argc, argv);
  TestMode::initTestMode(argc, argv);
  return RUN_ALL_TESTS();
}
