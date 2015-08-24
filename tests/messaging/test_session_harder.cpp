/*
 ** Author(s):
 **  - Cedric GESTES <cgestes@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <vector>
#include <string>

#include <gtest/gtest.h>

#include <qi/session.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <qi/type/typeinterface.hpp>
#include <boost/thread.hpp>
#include <testsession/testsessionpair.hpp>

qiLogCategory("test");

struct MyStruct {
  int i;
  int j;
  std::string titi;
};

QI_TYPE_STRUCT(MyStruct, i, j, titi);

static std::string reply(const std::string &msg)
{
  return msg;
}

static MyStruct reply2(const MyStruct &mystruct) {
  return mystruct;
}


qi::AnyObject newObject() {
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  ob.advertiseMethod("reply2", &reply2);
  return ob.object();
}

void alternateModule(qi::SessionPtr session) {
  while (!boost::this_thread::interruption_requested())
  {
    boost::this_thread::interruption_point();
    //a--;
    qi::AnyObject obj = newObject();
    qi::Future<unsigned int> fut = session->registerService("TestToto", obj);
    if (fut.hasError()) {
      std::cout << "Error registering service: " << fut.error() << std::endl;
      continue;
    }
    std::cout << "Service TestToto registered" << std::endl;
    qi::os::msleep(50);
    qi::Future<void> futun = session->unregisterService(fut.value());
    if (futun.hasError()) {
      std::cout << "Error unregistering service: " << futun.error() << std::endl;
      continue;
    }
    std::cout << "Service TestToto unregistered" << std::endl;
  }
}


TEST(QiSession, RegisterUnregisterTwoSession)
{
  int a = 1000;
  if (TestMode::getTestMode() == TestMode::Mode_SSL)
  {
    a /= 100;
  }
  TestSessionPair p;
  EXPECT_TRUE(p.client()->isConnected());

  boost::thread worker(boost::bind(&alternateModule, p.server()));
  while (a) {
    a--;
    try {
      qi::Future<qi::AnyObject> fut = p.client()->service("TestToto");
      if (fut.hasError()) {
        std::cout << "Call error:" << fut.error() << std::endl;
        continue;
      }
      std::string ret = fut.value().call<std::string>("reply", "plif");
    }
    catch(const std::exception& e)
    {
      std::cout << "Call exception: " << e.what() << std::endl;
    }
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
    try
    {
      qi::Future<qi::AnyObject> fut = p.server()->service("TestToto");
      if (fut.hasError()) {
        std::cout << "Call error:" << fut.error() << std::endl;
        continue;
      }
      fut.value().call<std::string>("reply", "plif");
    }
    catch(const std::exception& e)
    {
      std::cout << "Call exception: " << e.what() << std::endl;
    }
  }
  worker.interrupt();
  worker.join();
}

TEST(QiSession, RegisterUnregisterTwoSessionStruct)
{
  int a = 1000;
  if (TestMode::getTestMode() == TestMode::Mode_SSL)
  {
    a /= 100;
  }
  TestSessionPair p;
  EXPECT_TRUE(p.client()->isConnected());

  boost::thread worker(boost::bind(&alternateModule, p.server()));
  while (a) {
    a--;
    try
    {
      qi::Future<qi::AnyObject> fut = p.client()->service("TestToto");
      if (fut.hasError()) {
        std::cout << "Call error:" << fut.error() << std::endl;
        continue;
      }
      MyStruct ms;
      ms.i = 32;
      ms.j = 42;
      ms.titi = "tutu";
      qi::Future<MyStruct> ret = fut.value().async<MyStruct>("reply2", ms);
      ret.wait();
      if (ret.hasError()) {
        std::cout << "returned an error:" << fut.error() << std::endl;
      continue;
      }
      ASSERT_EQ(ms.i, ret.value().i);
      ASSERT_EQ(ms.j, ret.value().j);
      ASSERT_EQ(ms.titi, ret.value().titi);
    }
     catch(const std::exception& e)
    {
      std::cout << "Call exception: " << e.what() << std::endl;
    }
  }
  worker.interrupt();
  worker.join();
}


TEST(QiSession, RegisterUnregisterSameSessionStruct)
{
  int a = 1000;
  if (TestMode::getTestMode() == TestMode::Mode_SSL)
  {
    a /= 100;
  }
  TestSessionPair p;
  EXPECT_TRUE(p.client()->isConnected());

  boost::thread worker(boost::bind(&alternateModule, p.server()));
  while (a) {
    a--;
    try
    {
      qi::Future<qi::AnyObject> fut = p.server()->service("TestToto");
      if (fut.hasError()) {
        std::cout << "Call error:" << fut.error() << std::endl;
        continue;
      }
      MyStruct ms;
      ms.i = 32;
      ms.j = 42;
      ms.titi = "tutu";
      qi::Future<MyStruct> ret = fut.value().async<MyStruct>("reply2", ms);
      ret.wait();
      if (ret.hasError()) {
        std::cout << "returned an error:" << fut.error() << std::endl;
        continue;
      }
      ASSERT_EQ(ms.i, ret.value().i);
      ASSERT_EQ(ms.j, ret.value().j);
      ASSERT_EQ(ms.titi, ret.value().titi);
    }
    catch(const std::exception& e)
    {
      std::cout << "Call exception: " << e.what() << std::endl;
    }
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

  qi::AnyObject obj = newObject();
  constellation1.server()->registerService("test1", obj);
  constellation2.server()->registerService("test2", obj);
  constellation3.server()->registerService("test3", obj);

  qi::Future<void> f;
  f = traveler.connect(constellation1.serviceDirectoryEndpoints()[0].str());
  f.wait(3000);
  ASSERT_TRUE(!f.hasError());
  qi::AnyObject proxy = constellation1.server()->service("test1");
  std::string res = proxy.call<std::string>("reply", "plaf");
  ASSERT_TRUE(res.compare("plaf") == 0);
  traveler.close();

  f = traveler.connect(constellation2.serviceDirectoryEndpoints()[0].str());
  f.wait(3000);
  ASSERT_TRUE(!f.hasError());
  proxy = constellation2.server()->service("test2");
  ASSERT_TRUE(!!proxy);
  res = proxy.call<std::string>("reply", "plaf");
  ASSERT_TRUE(res.compare("plaf") == 0);
  traveler.close();

  f = traveler.connect(constellation3.serviceDirectoryEndpoints()[0].str());
  f.wait(3000);
  if (f.hasError())
    qiLogError() << f.error();
  ASSERT_TRUE(!f.hasError());
  proxy = constellation3.server()->service("test3");
  res = proxy.call<std::string>("reply", "plaf");
  ASSERT_TRUE(res.compare("plaf") == 0);
  traveler.close();
}


int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
