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
#include <qimessaging/genericobject.hpp>
#include <qimessaging/genericobjectbuilder.hpp>
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
  std::stringstream ss;
  qi::ServiceDirectory sd;

  ss << "tcp://127.0.0.1:" << qi::os::findAvailablePort(3000);
  sd.listen(ss.str());

  bool connected = session.connect(ss.str()).wait();
  ASSERT_TRUE(connected);

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  unsigned int servicePort = qi::os::findAvailablePort(0);
  std::stringstream serviceAddr;
  serviceAddr << "tcp://127.0.0.1:" << servicePort;

  session.listen(serviceAddr.str());

  // Wait for service id, otherwise register is asynchronous.
  session.registerService("serviceTest", obj).wait();
  ASSERT_TRUE(session.waitForServiceReady("serviceTest"));

  qi::ObjectPtr object = session.service("serviceTest");
  EXPECT_TRUE(object);

  session.close();
  EXPECT_FALSE(session.isConnected());

  std::vector<qi::ServiceInfo> services = session.services();
  EXPECT_EQ(0U, services.size());
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
