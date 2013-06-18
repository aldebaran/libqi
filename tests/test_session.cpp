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
  EXPECT_ANY_THROW(session.connect("tcp://127.0.0.1:1234"));

  EXPECT_FALSE(session.isConnected());

  session.close();
  EXPECT_FALSE(session.isConnected());
}

TEST(QiSession, simpleConnectionToInvalidAddrToSd)
{
  qi::Session session;
  qi::Future<void> fConnected = session.connect("tcp://0.0.0.0:0");

  fConnected.wait();

  EXPECT_TRUE(fConnected.hasError());
  EXPECT_FALSE(session.isConnected());

  session.close();
  EXPECT_FALSE(session.isConnected());
}

TEST(QiSession, simpleConnectionToInvalidSd)
{
  qi::Session session;
  qi::Future<void> fConnected = session.connect("invalidAddress");
  fConnected.wait(3000);
  EXPECT_TRUE(fConnected.hasError());
  EXPECT_FALSE(session.isConnected());

  session.close();
  EXPECT_FALSE(session.isConnected());
}

TEST(QiSession, testClose)
{
  qi::Session session;

  qi::ServiceDirectory sd;

  qi::Future<void> f = sd.listen("tcp://127.0.0.1:0");
  f.wait(3000);
  ASSERT_TRUE(!f.hasError());

  f = session.connect(sd.endpoints()[0]);
  f.wait(3000);
  ASSERT_TRUE(!f.hasError());

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  f = session.listen("tcp://127.0.0.1:0");
  f.wait(3000);
  ASSERT_TRUE(!f.hasError());

  // Wait for service id, otherwise register is asynchronous.
  qi::Future<unsigned int> idx = session.registerService("serviceTest", obj);
  ASSERT_FALSE(idx.hasError());

  qi::ObjectPtr object = session.service("serviceTest");
  EXPECT_TRUE(object);


  session.close();
  EXPECT_FALSE(session.isConnected());

  EXPECT_ANY_THROW(session.services().value());

  f = session.connect(sd.endpoints()[0]);
  f.wait(3000);
  ASSERT_TRUE(!f.hasError());

  EXPECT_ANY_THROW(session.unregisterService(idx.value()));
}

TEST(QiSession, getSimpleService)
{
  TestSessionPair pair;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  pair.server()->registerService("serviceTest", obj);

  qi::ObjectPtr object = pair.server()->service("serviceTest");
  EXPECT_TRUE(object);
}

TEST(QiSession, getSimpleServiceTwice)
{
  TestSessionPair pair;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  pair.server()->registerService("serviceTest", obj);

  qi::Future<qi::ObjectPtr> f1 = pair.client()->service("serviceTest");
  qi::Future<qi::ObjectPtr> f2 = pair.client()->service("serviceTest");
  f1.wait();
  f2.wait();

  EXPECT_TRUE(f1.value().get() == f2.value().get());
}

TEST(QiSession, getSimpleServiceTwiceUnexisting)
{
  TestSessionPair pair;

  qi::Future<qi::ObjectPtr> f1 = pair.server()->service("xxxLOL");
  qi::Future<qi::ObjectPtr> f2 = pair.server()->service("xxxLOL");
  f1.wait();
  f2.wait();

  EXPECT_TRUE(f1.hasError());
  EXPECT_TRUE(f2.hasError());
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

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  p.server()->registerService("serviceTest", obj);
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
  TestSessionPair p;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());


  ASSERT_GT(p.server()->registerService("service", obj), static_cast<unsigned int>(0));
  EXPECT_ANY_THROW({p.server()->registerService("service", obj).value();});

}

TEST(QiSession, Services)
{
  TestSessionPair p;
  qi::Session* s1 = p.client();
  qi::Session* s2 = p.server();

  qi::DynamicObjectBuilder ob;
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


TEST(QiSession, TestServiceDirectoryEndpoints)
{
  qi::ServiceDirectory sd;

  qi::Future<void> f = sd.listen("tcp://0.0.0.0:0");
  f.wait(3000);
  ASSERT_TRUE(!f.hasError());

  // but it's possible to get joinable addresses.
  ASSERT_NE(sd.endpoints().at(0).host(),"0.0.0.0");
  ASSERT_NE(sd.endpoints().at(0).port(), 0);
}

void onConnected(qi::Session *ses, qi::Promise<void> continueBaby) {
  ses->services().value();
  continueBaby.setValue(0);
}

TEST(QiSession, getCallInConnect)
{
  TestSessionPair pair;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  pair.server()->registerService("serviceTest", obj);
  qi::ObjectPtr object = pair.server()->service("serviceTest");

  EXPECT_TRUE(object);
  qi::Session ses;
  qi::Promise<void> finito;

  ses.connected.connect(boost::bind<void>(&onConnected, &ses, finito));
  ses.connect(pair.serviceDirectoryEndpoints()[0]);
  qi::Future<void> ff = finito.future();

  ff.wait(2000);
  EXPECT_TRUE(ff.isFinished());
  EXPECT_TRUE(ses.isConnected());
}

TEST(QiSession, asyncConnect) {
  qi::ServiceDirectory sd;
  sd.listen("tcp://127.0.0.1:0");

  qi::Session s;
  s.connect(sd.endpoints()[0]).async();
  try {
    //we dont know the success or failure of the operation.
    qi::ObjectPtr obj = s.service("IDontWantToSegfaultHere");
  } catch(...) {
  }
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  ::TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
