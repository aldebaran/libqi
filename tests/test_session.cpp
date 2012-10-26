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
  session.connect("tcp://127.0.0.1:1234");

  EXPECT_FALSE(session.isConnected());

  session.close();
  EXPECT_FALSE(session.isConnected());
}

TEST(QiSession, simpleConnectionToInvalidAddrToSd)
{
  qi::Session session;
  qi::Future<bool> fConnected = session.connect("tcp://0.0.0.0:0");

  fConnected.wait();

  EXPECT_TRUE(fConnected.hasError());
  EXPECT_FALSE(session.isConnected());

  session.close();
  EXPECT_FALSE(session.isConnected());
}

TEST(QiSession, simpleConnectionToInvalidSd)
{
  qi::Session session;
  qi::Future<bool> fConnected = session.connect("invalidAddress");

  fConnected.wait();

  EXPECT_FALSE(fConnected.value());
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

  EXPECT_ANY_THROW(session.services().value());

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

  EXPECT_ANY_THROW({
    qi::ObjectPtr object = p.client()->service("windowsVista(c)");
  });

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

  // Todo later, expect same behavior.
  if (TestMode::getTestMode() != TestMode::Mode_Direct)
  {
    EXPECT_ANY_THROW({p.client()->service("serviceTest").value();});
  }

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
  EXPECT_ANY_THROW({session->registerService("service", obj).value();});

  delete session;
}

TEST(QiSession, Services)
{
  TestSessionPair p;
  qi::Session* s1 = p.client();
  s1->listen("tcp://0.0.0.0:0"); // client does not listen by default
  qi::Session* s2 = p.server();

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());
  s1->registerService("srv1.1", obj);
  s1->registerService("srv1.2", obj);
  s2->registerService("srv2.1", obj);
  s2->registerService("srv2.2", obj);
  std::vector<qi::ServiceInfo> srv1 = s1->services();
  std::vector<qi::ServiceInfo> srv2 = s2->services();
  // serviceDirectory is listed to
  if (srv1.size() != 5)
  {
    for (unsigned i=0; i<srv1.size(); ++i)
      std::cerr << srv1[i].name() << " ";
    std::cerr << std::endl;
  }

  ASSERT_EQ(5U, s1->services().value().size());
  ASSERT_EQ(5U, s2->services().value().size());
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
