/*
** Author(s):
**  - Guillaume OREAL <goreal@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <vector>
#include <string>
#include <gtest/gtest.h>

#include <boost/thread/thread.hpp>
#include <qi/session.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <testsession/testsessionpair.hpp>

qiLogCategory("QiSession.Test");

static std::string reply(const std::string &msg)
{
  qi::os::msleep(300);
  std::cout << msg << std::endl;
  return msg;
}

void myCall(qi::AnyObject myService)
{
  try
  {
    myService.call<std::string>("reply::s(s)", "ok");
    qi::os::msleep(300);
  }
  catch(std::exception e)
  {
    std::cout << e.what() << std::endl;
  }
}

TEST(QiSession, CallOnCloseSession)
{
  qi::Session sd;
  qi::Future<void> f = sd.listenStandalone("tcp://0.0.0.0:0");
  int timeToWait = 1;

  for(int j = 0; j < 10 ; j ++)
  {
    for (int i = 0; i < 20; i++)
    {
      TestSessionPair p;
      std::cout << "time to wait is:" << timeToWait << std::endl;

      qi::SessionPtr s1 = p.server();
      qi::SessionPtr s2 = p.client();

      qi::DynamicObjectBuilder ob;
      ob.advertiseMethod("reply", &reply);
      qi::AnyObject obj(ob.object());

      s1->registerService("service1", obj);

      qi::AnyObject myService;
      myService = s2->service("service1");

      boost::thread myThread(boost::bind(&myCall, myService));

      qi::os::msleep(timeToWait);
      s1->close();
      qi::os::msleep(3);
    }
    timeToWait = timeToWait +1;
  }
}

TEST(QiSession, GettingServiceWhileDisconnecting)
{
  qi::SessionPtr server = qi::makeSession();
  server->listenStandalone("tcp://0.0.0.0:0");

  qi::DynamicObjectBuilder builder;
  qi::AnyObject object(builder.object());

  std::string serviceName = "sarace";
  server->registerService(serviceName, object);

  qi::SessionPtr client = qi::makeSession();

  for(int i = 0; i < 1000; ++i)
  {
    client->connect(server->endpoints()[0]);
    qi::Future<void> closing = client->close().async();
    try
    {
      qi::AnyObject remoteObject = client->service(serviceName);
      bool remoteObjectWasFound = remoteObject;
      ASSERT_TRUE(remoteObjectWasFound);
    }
    catch(const qi::FutureException& e)
    {
      qiLogDebug() << "Got expected error: " << e.what();
    }
    catch(const std::exception& e)
    {
      qiLogDebug() << "Got standard error: " << e.what();
    }
    closing.wait();
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  qi::Application app(argc, argv);
  TestMode::forceTestMode(TestMode::Mode_SD);
  return RUN_ALL_TESTS();
}
