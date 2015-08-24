/*
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
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

#include <testsession/testsessionpair.hpp>

qiLogCategory("test");

qi::Atomic<int> nThreadFinished;
void create_session(bool bare)
{
  {
    qi::Session s;
    if (!bare)
      s.listen("tcp://localhost:0");
  }
  ++nThreadFinished;
}


// KEEP ME FIRST
TEST(QiSession, create)
{
  nThreadFinished = 0;
  /* A lot of static init is going on, check that it is thread safe
  */
  boost::thread(create_session, true);
  boost::thread(create_session, true);
  boost::thread(create_session, true);
  boost::thread(create_session, true);
  qi::os::msleep(50);
  boost::thread(create_session, false);
  boost::thread(create_session, false);
  boost::thread(create_session, false);
  boost::thread(create_session, false);
  qi::os::msleep(50);
  boost::thread(create_session, false);
  boost::thread(create_session, true);
  boost::thread(create_session, false);
  boost::thread(create_session, true);
  while (*nThreadFinished != 12)
    qi::os::msleep(10);
}

static void session_close(qi::SessionPtr s, qi::Atomic<int>* counter)
{
  s->close();
  ++(*counter);
}

TEST(QiSession, multiClose)
{
  qi::Atomic<int> counter;
  TestSessionPair p;
  boost::thread(session_close, p.client(), &counter);
  boost::thread(session_close, p.client(), &counter);
  boost::thread(session_close, p.server(), &counter);
  boost::thread(session_close, p.server(), &counter);
  boost::thread(session_close, p.server(), &counter);
  while (*counter != 5)
    qi::os::msleep(50);
}

static std::string reply(const std::string &msg)
{
  return msg;
}

TEST(QiSession, simpleConnectionToSd)
{
  TestSessionPair p;

  EXPECT_TRUE(p.client()->isConnected());
}

TEST(QiSession, multipleConnectionToNonReachableSd)
{
  qi::Session session;
  qi::Future<void> f = session.connect("tcp://127.0.0.1:1234");
  f.wait();
  EXPECT_TRUE(f.hasError());
  f = session.connect("tcp://127.0.0.1:1234");
  f.wait();
  EXPECT_TRUE(f.hasError());
  f = session.connect("tcp://127.0.0.1:1234");
  f.wait();
  EXPECT_TRUE(f.hasError());
}

TEST(QiSession, connectOnSecondAttempt)
{
  qi::Session session;
  qi::Future<void> f = session.connect("tcp://127.0.0.1:1234");
  f.wait();
  EXPECT_TRUE(f.hasError());
  qi::Session s2;
  s2.listenStandalone("tcp://127.0.0.1:0");
  f = session.connect(s2.url());
  f.wait();
  if (f.hasError())
    qiLogWarning() << f.error();
  EXPECT_FALSE(f.hasError());
}

TEST(QiSession, multipleConnectSuccess)
{
  qi::Session session, sd;
  sd.listenStandalone("tcp://127.0.0.1:0");
  qiLogVerbose() <<"connect";
  qi::Future<void> f = session.connect(sd.url());
  qiLogVerbose() <<"wait";
  f.wait();
  if (f.hasError())
    qiLogWarning() << f.error();
  EXPECT_FALSE(f.hasError());
  qiLogVerbose() <<"close";
  session.close();
  qiLogVerbose() <<"connect";
  f = session.connect(sd.url());
  qiLogVerbose() <<"wait";
  f.wait();
  if (f.hasError())
    qiLogWarning() << f.error();
  EXPECT_FALSE(f.hasError());
  qiLogVerbose() <<"close";
  session.close();
  qiLogVerbose() <<"connect";
  f = session.connect(sd.url());
  qiLogVerbose() <<"wait";
  f.wait();
  if (f.hasError())
    qiLogWarning() << f.error();
  EXPECT_FALSE(f.hasError());
  qiLogVerbose() <<"close";
  session.close();
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

  qi::Session sd;

  qi::Future<void> f = sd.listenStandalone("tcp://127.0.0.1:0");
  f.wait(3000);
  ASSERT_TRUE(!f.hasError());

  f = session.connect(sd.endpoints()[0]);
  f.wait(3000);
  ASSERT_TRUE(!f.hasError());

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  f = session.listen("tcp://127.0.0.1:0");
  f.wait(3000);
  ASSERT_TRUE(!f.hasError());

  // Wait for service id, otherwise register is asynchronous.
  qi::Future<unsigned int> idx = session.registerService("serviceTest", obj);
  ASSERT_FALSE(idx.hasError());

  qi::AnyObject object = session.service("serviceTest");
  EXPECT_TRUE(object);


  session.close();
  EXPECT_FALSE(session.isConnected());

  EXPECT_ANY_THROW(session.services().value());

  f = session.connect(sd.endpoints()[0]);
  f.wait(3000);
  if (f.hasError())
    qiLogError() << f.error();
  ASSERT_TRUE(!f.hasError());

  EXPECT_ANY_THROW(session.unregisterService(idx.value()));
}

TEST(QiSession, getSimpleService)
{
  TestSessionPair pair;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  pair.server()->registerService("serviceTest", obj);

  qi::AnyObject object = pair.server()->service("serviceTest");
  EXPECT_TRUE(object);
}

TEST(QiSession, getSimpleServiceTwice)
{
  TestSessionPair pair;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  pair.server()->registerService("serviceTest", obj);

  qi::Future<qi::AnyObject> f1 = pair.client()->service("serviceTest");
  qi::Future<qi::AnyObject> f2 = pair.client()->service("serviceTest");
  f1.wait();
  f2.wait();

  EXPECT_TRUE(f1.value().asGenericObject() == f2.value().asGenericObject());
}

TEST(QiSession, getSimpleServiceTwiceUnexisting)
{
  TestSessionPair pair;

  qi::Future<qi::AnyObject> f1 = pair.server()->service("xxxLOL");
  qi::Future<qi::AnyObject> f2 = pair.server()->service("xxxLOL");
  f1.wait();
  f2.wait();

  EXPECT_TRUE(f1.hasError());
  EXPECT_TRUE(f2.hasError());
}

TEST(QiSession, getUnregisterService)
{
  TestSessionPair p;

  EXPECT_ANY_THROW({
    qi::AnyObject object = p.client()->service("windowsVista(c)");
  });

  p.client()->close();
  EXPECT_FALSE(p.client()->isConnected());
}

TEST(QiSession, getCloseService)
{
  TestSessionPair p;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

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
  qi::AnyObject obj(ob.object());


  ASSERT_GT(p.server()->registerService("service", obj), static_cast<unsigned int>(0));
  EXPECT_ANY_THROW({p.server()->registerService("service", obj).value();});

}

TEST(QiSession, Services)
{
  TestSessionPair p;
  qi::SessionPtr s1 = p.client();
  qi::SessionPtr s2 = p.server();

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());
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
  qi::Session sd;

  qi::Future<void> f = sd.listenStandalone("tcp://0.0.0.0:0");
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
  qi::AnyObject obj(ob.object());

  pair.server()->registerService("serviceTest", obj);
  qi::AnyObject object = pair.server()->service("serviceTest");

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
  qi::Session sd;
  sd.listenStandalone("tcp://127.0.0.1:0");

  qi::Session s;
  s.connect(sd.endpoints()[0]).async();
  try {
    //we dont know the success or failure of the operation.
    qi::AnyObject obj = s.service("IDontWantToSegfaultHere");
  } catch(...) {
  }
}

TEST(QiSession, urlOnClosed)
{
  qi::Session sd;
  sd.listenStandalone("tcp://127.0.0.1:0");
  qi::Session s;
  EXPECT_ANY_THROW(s.url());
  s.connect(sd.endpoints()[0]);
  EXPECT_NO_THROW(s.url());
  s.close();
  EXPECT_ANY_THROW(s.url());
}

TEST(QiSession, serviceRegisteredCtrl)
{
  // Control test for the test serviceRegistered, to ensure we properly detect
  // remote services
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  qi::Session sd;
  sd.listenStandalone("tcp://127.0.0.1:0");
  qi::Session s;
  s.connect(sd.endpoints()[0]);
  sd.registerService("s", obj);
  qi::AnyObject c = s.service("s");
  ASSERT_TRUE(c);
  qi::DynamicObject* dobj = (qi::DynamicObject*)c.asGenericObject()->value;
  qi::DynamicObject* sdobj = (qi::DynamicObject*)obj.asGenericObject()->value;
  ASSERT_NE(dobj, sdobj);

  c = sd.service("s");
  ASSERT_TRUE(c);
  dobj = (qi::DynamicObject*)c.asGenericObject()->value;
  sdobj = (qi::DynamicObject*)obj.asGenericObject()->value;
  ASSERT_EQ(dobj, sdobj);
}


void fetch_service(qi::Session& s, const std::string& name, qi::AnyObject& ao)
{
  ao = s.service(name);
}

TEST(QiSession, serviceRegistered)
{
  // Check a nasty race situation where a service is advertised as registered
  // by the session before being realy present
  // The symptom is not a session.service() failure, but a spurious use of
  // remote mode.
  qi::Session sd;
  sd.listenStandalone("tcp://127.0.0.1:0");
  qi::AnyObject ao;
  sd.serviceRegistered.connect(&fetch_service, boost::ref(sd), _2, boost::ref(ao));

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());
  sd.registerService("s", obj);
  for (unsigned i=0; i<200 && !ao; ++i)
    qi::os::msleep(10);
  // check we got the object, and that it is not a remoteobject
  ASSERT_TRUE(ao);
  qi::DynamicObject* dobj = (qi::DynamicObject*)ao.asGenericObject()->value;
  ASSERT_TRUE(dobj);

  ASSERT_EQ(obj.asGenericObject()->value, ao.asGenericObject()->value);
}

TEST(QiSession, reuseSd)
{
  TestSessionPair pair;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  pair.client()->registerService("serviceTest", obj);

  qi::AnyObject object = pair.sd()->service("serviceTest");
  EXPECT_TRUE(object);
  EXPECT_EQ("foo", object.call<std::string>("reply", "foo"));
}

TEST(QiSession, WaitForService)
{
  TestSessionPair pair;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  unsigned int sid = pair.sd()->registerService("serviceTest", obj);
  qi::Future<void> future = pair.client()->waitForService("serviceTest");
  EXPECT_TRUE(future.hasValue());
  pair.sd()->unregisterService(sid);

  future = pair.client()->waitForService("serviceTest");
  sid = pair.sd()->registerService("serviceTest", obj);
  EXPECT_TRUE(future.hasValue());
}

TEST(QiSession, WaitForServiceCanceled)
{
  TestSessionPair pair;

  qi::Future<void> future = pair.client()->waitForService("serviceTest");
  future.cancel();
  EXPECT_TRUE(future.isCanceled());
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
