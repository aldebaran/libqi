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
#include <qimessaging/object.hpp>
#include <qimessaging/service_directory.hpp>
#include <qimessaging/gateway.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>

static std::string reply(const std::string &msg)
{
  return msg;
}

static std::string connectionAddr;


TEST(QiSession, simpleConnectionToSd)
{
  qi::Session session;
  bool connected = session.connect(connectionAddr);
  ASSERT_TRUE(connected);

  EXPECT_TRUE(session.isConnected());

  session.close();

  EXPECT_TRUE(!session.isConnected());
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

TEST(QiSession, getSimpleService)
{
  qi::Session session;
  bool connected = session.connect(connectionAddr);
  EXPECT_TRUE(connected);

  qi::Object obj;
  obj.advertiseMethod("reply", &reply);

  unsigned int servicePort = qi::os::findAvailablePort(0);
  std::stringstream serviceAddr;
  serviceAddr << "tcp://127.0.0.1:" << servicePort;

  session.listen(serviceAddr.str());
  // Wait for service id, otherwise register is asynchronous.
  session.registerService("serviceTest", &obj).wait();
  ASSERT_TRUE(session.waitForServiceReady("serviceTest"));

  qi::Object *object = session.service("serviceTest");
  EXPECT_TRUE(object);

  delete object;
  session.close();
  EXPECT_FALSE(session.isConnected());

}

TEST(QiSession, getUnregisterService)
{
  qi::Session session;
  bool connected = session.connect(connectionAddr);
  EXPECT_TRUE(connected);

  qi::Object *object = session.service("serviceTest");
  EXPECT_FALSE(object);

  session.close();
  EXPECT_FALSE(session.isConnected());
}

TEST(QiSession, getCloseService)
{
  qi::Session session, client;
  bool connected = session.connect(connectionAddr);
  EXPECT_TRUE(connected);

  qi::Object obj;
  obj.advertiseMethod("reply", &reply);

  unsigned int servicePort = qi::os::findAvailablePort(0);
  std::stringstream serviceAddr;
  serviceAddr << "tcp://127.0.0.1:" << servicePort;

  session.listen(serviceAddr.str());
  session.registerService("serviceTest", &obj);
  session.close();

  client.connect(connectionAddr);
  EXPECT_TRUE(client.isConnected());

  qi::Object *object = client.service("serviceTest");
  EXPECT_FALSE(object);

  session.close();
  EXPECT_FALSE(session.isConnected());
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  ::testing::InitGoogleTest(&argc, argv);

  qi::ServiceDirectory sd;
  unsigned int sdPort = qi::os::findAvailablePort(5555);
  std::stringstream sdAddr;
  sdAddr << "tcp://127.0.0.1:" << sdPort;
  connectionAddr = sdAddr.str();

  sd.listen(sdAddr.str());

  return RUN_ALL_TESTS();
}
