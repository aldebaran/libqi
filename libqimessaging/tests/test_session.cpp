/*
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
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
#include <testsession/testsessionpair.hpp>

static std::string reply(const std::string &msg)
{
  return msg;
}

TEST(QiSession, simpleConnectionToSd)
{
  TestSessionPair p;

  EXPECT_TRUE(p.client()->isConnected());
}

TEST(QiSession, simpleConnectionToNonReachableSd)
{
  qi::Session session;
  session.connect("tcp://127.0.1.123:1245");

  EXPECT_FALSE(session.isConnected());

  session.close();
  EXPECT_FALSE(session.isConnected());
}

TEST(QiSession, simpleConnectionToInvalidAddrToSd)
{
  qi::Session session;
  bool connected = session.connect("tcp://0.0.0.0:0");

  EXPECT_FALSE(connected);
  EXPECT_FALSE(session.isConnected());

  session.close();
  EXPECT_FALSE(session.isConnected());
}

TEST(QiSession, simpleConnectionToInvalidSd)
{
  qi::Session session;
  bool connected = session.connect("invalidAddress");

  EXPECT_FALSE(connected);
  EXPECT_FALSE(session.isConnected());

  session.close();
  EXPECT_FALSE(session.isConnected());
}

TEST(QiSession, testClose)
{
  qi::Session session;

  qi::ServiceDirectory sd;

  sd.listen("tcp://127.0.0.1:0");

  bool connected = session.connect(sd.listenUrl()).wait();
  ASSERT_TRUE(connected);

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  session.listen("tcp://127.0.0.1:0");

  // Wait for service id, otherwise register is asynchronous.
  qi::Future<unsigned int> idx = session.registerService("serviceTest", obj);
  ASSERT_FALSE(idx.hasError());
  ASSERT_TRUE(session.waitForServiceReady("serviceTest"));

  qi::ObjectPtr object = session.service("serviceTest");
  EXPECT_TRUE(object);


  session.close();
  EXPECT_FALSE(session.isConnected());

  std::vector<qi::ServiceInfo> services = session.services();
  EXPECT_EQ(0U, services.size());

  connected = session.connect(sd.listenUrl());
  ASSERT_TRUE(connected);
  session.unregisterService(idx.value());
}

TEST(QiSession, getSimpleService)
{
  TestSessionPair pair;

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  // Wait for service id, otherwise register is asynchronous.
  pair.server()->registerService("serviceTest", obj).wait();
  ASSERT_TRUE(pair.server()->waitForServiceReady("serviceTest"));

  qi::ObjectPtr object = pair.server()->service("serviceTest");
  EXPECT_TRUE(object);
}

TEST(QiSession, getUnregisterService)
{
  TestSessionPair p;

  qi::ObjectPtr object = p.client()->service("windowsVista(c)");
  EXPECT_FALSE(object);

  p.client()->close();
  EXPECT_FALSE(p.client()->isConnected());
}

TEST(QiSession, getCloseService)
{
  TestSessionPair p;

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  p.server()->registerService("serviceTest", obj);
  p.server()->waitForServiceReady("serviceTest");
  p.server()->close();

  qi::ObjectPtr object = p.client()->service("serviceTest");

  // Todo later, expect same behavior.
  if (TestMode::getTestMode() != TestMode::Mode_Direct)
    EXPECT_FALSE(object);

  p.client()->close();
  EXPECT_FALSE(p.client()->isConnected());
}

TEST(QiSession, AlreadyRegistered)
{
  qi::Session*  session;
  std::stringstream ss;
  qi::ServiceDirectory sd;

  ss << "tcp://127.0.0.1:" << qi::os::findAvailablePort(3000);
  sd.listen(ss.str());

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  session = new qi::Session();
  EXPECT_TRUE(session->connect(ss.str()).wait(1000));
  EXPECT_TRUE(session->listen("tcp://0.0.0.0:0"));

  ASSERT_GT(session->registerService("service", obj), 0);
  ASSERT_LE(session->registerService("service", obj), 0);

  delete session;
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
