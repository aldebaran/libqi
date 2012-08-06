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
#include <qimessaging/server.hpp>
#include <qimessaging/service_directory.hpp>
#include <qimessaging/gateway.hpp>
#include <qi/os.hpp>

static std::string reply(const std::string &msg)
{
  return msg;
}

static std::string connectionAddr;


TEST(QiSession, simpleConnectionToSd)
{
  qi::Session session;
  session.connect(connectionAddr);
  ASSERT_TRUE(session.waitForConnected(1000));

  EXPECT_TRUE(session.isConnected());

  session.disconnect();
  bool disconnected = session.waitForDisconnected(1000);

  EXPECT_TRUE(disconnected);
}

TEST(QiSession, simpleConnectionToNonReachableSd)
{
  qi::Session session;
  session.connect("tcp://127.0.1.123:1245");
  bool connected = session.waitForConnected(1000);

  EXPECT_FALSE(connected);
  EXPECT_FALSE(session.isConnected());

  session.disconnect();
  bool disconnected = session.waitForDisconnected(1000);

  EXPECT_TRUE(disconnected);
}

TEST(QiSession, simpleConnectionToInvalidAddrToSd)
{
  qi::Session session;
  session.connect("tcp://0.0.0.0:0");
  bool connected = session.waitForConnected(1000);

  EXPECT_FALSE(connected);
  EXPECT_FALSE(session.isConnected());

  session.disconnect();
  bool disconnected = session.waitForDisconnected(1000);

  EXPECT_TRUE(disconnected);
}

TEST(QiSession, simpleConnectionToInvalidSd)
{
  qi::Session session;
  session.connect("invalidAddress");
  bool connected = session.waitForConnected(1000);

  EXPECT_FALSE(connected);
  EXPECT_FALSE(session.isConnected());

  session.disconnect();
  bool disconnected = session.waitForDisconnected(1000);

  EXPECT_TRUE(disconnected);
}

TEST(QiSession, getSimpleService)
{
  qi::Session session;
  session.connect(connectionAddr);
  bool connected = session.waitForConnected(1000);
  EXPECT_TRUE(connected);

  qi::Object obj;
  qi::Server srv;
  obj.advertiseMethod("reply", &reply);

  unsigned int servicePort = qi::os::findAvailablePort(0);
  std::stringstream serviceAddr;
  serviceAddr << "tcp://127.0.0.1:" << servicePort;

  srv.listen(&session, serviceAddr.str());
  // Wait for service id, otherwise register is asynchronous.
  srv.registerService("serviceTest", &obj).wait();
  ASSERT_TRUE(session.waitForServiceReady("serviceTest"));

  qi::Object *object = session.service("serviceTest");
  EXPECT_TRUE(object);

  delete object;
  srv.close();
  session.disconnect();
  bool disconnected = session.waitForDisconnected(1000);

  EXPECT_TRUE(disconnected);
}

TEST(QiSession, getUnregisterService)
{
  qi::Session session;
  session.connect(connectionAddr);
  bool connected = session.waitForConnected(1000);

  EXPECT_TRUE(connected);

  qi::Object *object = session.service("serviceTest");
  EXPECT_FALSE(object);

  session.disconnect();
  bool disconnected = session.waitForDisconnected(1000);

  EXPECT_TRUE(disconnected);
}

TEST(QiSession, getCloseService)
{
  qi::Session session;
  session.connect(connectionAddr);
  bool connected = session.waitForConnected(1000);
  EXPECT_TRUE(connected);

  qi::Object obj;
  qi::Server srv;
  obj.advertiseMethod("reply", &reply);

  unsigned int servicePort = qi::os::findAvailablePort(0);
  std::stringstream serviceAddr;
  serviceAddr << "tcp://127.0.0.1:" << servicePort;

  srv.listen(&session, serviceAddr.str());
  srv.registerService("serviceTest", &obj);
  srv.close();

  qi::Object *object = session.service("serviceTest");
  EXPECT_FALSE(object);

  session.disconnect();
  bool disconnected = session.waitForDisconnected(1000);

  EXPECT_TRUE(disconnected);
}
int main(int argc, char **argv)
{
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
