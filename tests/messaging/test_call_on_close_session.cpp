/*
** Author(s):
**  - Guillaume OREAL <goreal@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <future>
#include <vector>
#include <string>
#include <gtest/gtest.h>
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

TEST(QiSession, CallOnCloseSession)
{
  qi::SessionPtr sd = qi::makeSession();
  sd->listenStandalone("tcp://0.0.0.0:0").async();

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  int timeToWaitMs = 1;

  for(int j = 0; j < 10 ; j ++)
  {
    for (int i = 0; i < 10; i++)
    {
      TestSessionPair p{TestMode::Mode_SD};
      std::cout << "time to wait is:" << timeToWaitMs << std::endl;

      qi::SessionPtr s1 = p.server();
      qi::SessionPtr s2 = p.client();

      s1->registerService("service1", ob.object());

      qi::AnyObject myService;
      myService = s2->service("service1");

      auto myCall = [&](qi::AnyObject myService)
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
      };

      std::async(std::launch::async, myCall, myService);
      qi::os::msleep(timeToWaitMs);
      s1->close();
      qi::os::msleep(3);
    }
    timeToWaitMs = timeToWaitMs + 2;
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

  for(int i = 0; i < 100; ++i)
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
