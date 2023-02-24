/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <map>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/atomic.hpp>
#include <qi/session.hpp>

#include <testsession/testsessionpair.hpp>
#include <qi/testutils/testutils.hpp>

qiLogCategory("test");

/*
 * Bound object do not have the same event id as "raw" object.
 * BoundObject add an offset of 10.
 *
 * So we take both id, the local and the remote one.
 */

static qi::Promise<int> *payload1;

namespace
{
qi::Atomic<int> i{0};

void onFire1(const int& pl)
{
  std::cout << "onFire:" << pl << std::endl;
  std::cout.flush();
  payload1->setValue(pl);
}

static qi::Promise<int> *payload2;

void onFire2(const int& pl)
{
  std::cout << "onFire2:" << pl << std::endl;
  std::cout.flush();
  payload2->setValue(pl);
}

void callbackCounter(const int&)
{
  ++i;
  std::cout << "callback called " << i._value <<" times" << std::endl;
}

void callbackCounterBis(const int&, std::string)
{
  ++i;
  std::cout << "callback called " << i.load() <<" times" << std::endl;
}
} // anonymous

class ObjectEventRemoteConnect: public ::testing::Test
{
public:
  ObjectEventRemoteConnect()
    : oe1(0)
    , oe2(0)
    , om1(0)
    , om2(0)
    , p2(TestSessionPair::ShareServiceDirectory, p1)
  {
    qi::DynamicObjectBuilder obs1, obs2;
    se1 = obs1.advertiseSignal<const int&>("fire1");
    se2 = obs2.advertiseSignal<const int&>("fire2");
    sm1 = obs1.advertiseMethod("onFire1", &onFire1);
    callbackId = obs1.advertiseMethod("callbackCounter", &callbackCounter);
    secondCallbackId = obs1.advertiseMethod("callbackCounter", &callbackCounterBis);
    sm2 = obs2.advertiseMethod("onFire2", &onFire2);
    oserver1 = obs1.object();
    oserver2 = obs2.object();
  }

protected:
  void SetUp()
  {
    // Two objects with a fire event and a onFire method.
    unsigned int nbLocalServices = TestMode::getTestMode() == TestMode::Mode_Nightmare ? 2 : 1;
    unsigned int nbConnectedServices = TestMode::getTestMode() == TestMode::Mode_Nightmare ? 5 : 3;

    ASSERT_TRUE(p1.server()->registerService("coin1", oserver1).hasValue(1000));
    ASSERT_TRUE(p2.server()->registerService("coin2", oserver2).hasValue(1000));
    EXPECT_EQ(nbLocalServices, p1.server()->services(qi::Session::ServiceLocality_Local).value().size());
    EXPECT_EQ(nbLocalServices, p2.server()->services(qi::Session::ServiceLocality_Local).value().size());

    EXPECT_TRUE(test::verifyBeforeDuration(
        [&] { return nbConnectedServices == p1.client()->services().value().size(); },
        serviceWaitDefaultTimeout));
    EXPECT_TRUE(test::verifyBeforeDuration(
        [&] { return nbConnectedServices == p2.client()->services().value().size(); },
        serviceWaitDefaultTimeout));

    qi::Future<qi::AnyObject> fut;
    fut = p2.client()->service("coin1");
    ASSERT_FALSE(fut.hasError());
    oclient1 = fut.value();

    fut = p1.client()->service("coin2");
    ASSERT_FALSE(fut.hasError());
    oclient2 = fut.value();

    oe1 = oclient1.metaObject().signalId("fire1::(i)");
    oe2 = oclient2.metaObject().signalId("fire2::(i)");
    qiLogDebug() << "Object E1:" << oe1 << " - Server E1:" << se1;
    qiLogDebug() << "Object E2:" << oe2 << " - Server E2:" << se2;
    ASSERT_TRUE(oe1 >= 10 && oe1 < 1000);
    ASSERT_TRUE(oe2 >= 10 && oe2 < 1000);

    om1 = oclient1.metaObject().methodId("onFire1::(i)");
    om2 = oclient2.metaObject().methodId("onFire2::(i)");
    qiLogDebug() << "Object M1:" << om1 << " - Server M1:" << sm1;
    qiLogDebug() << "Object M2:" << om2 << " - Server M2:" << sm2;
    ASSERT_TRUE(om1 >= 10 && om1 < 1000);
    ASSERT_TRUE(om2 >= 10 && om2 < 1000);

    payload1 = &prom1;
    payload2 = &prom2;
  }

  void TearDown()
  {
    payload1 = payload2 = 0;
  }

public:
  unsigned int se1, se2;
  unsigned int sm1, sm2;
  unsigned int callbackId, secondCallbackId;

  unsigned int oe1, oe2;
  unsigned int om1, om2;

  qi::Promise<int>     prom1, prom2;
  TestSessionPair      p1;
  TestSessionPair      p2;
  qi::AnyObject        oserver1, oserver2;
  qi::AnyObject        oclient1, oclient2;
};


// We try to test all possible combinations of where we connect, where we emit.

TEST_F(ObjectEventRemoteConnect, Connect1)
{
  oclient1.connect(oe1, oclient1, om1).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect2)
{
  oclient1.connect(oe1, oclient1, om1).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}


TEST_F(ObjectEventRemoteConnect, Connect3)
{
  oclient1.connect(oe1, oserver1, sm1).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect4)
{
  oclient1.connect(oe1, oserver1, sm1).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect5)
{
  oserver1.connect(se1, oclient1, om1).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect6)
{
  oserver1.connect(se1, oclient1, om1).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect7)
{
  oserver1.connect(se1, oserver1, sm1).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect8)
{
  oserver1.connect(se1, oserver1, sm1).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect10)
{
  oclient1.connect(oe1, oclient2, om2).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect11)
{
  oclient1.connect(oe1, oclient2, om2).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}


TEST_F(ObjectEventRemoteConnect, Connect12)
{
  oclient1.connect(oe1, oserver2, sm2).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect13)
{
  oclient1.connect(oe1, oserver2, sm2).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect14)
{
  oserver1.connect(se1, oclient2, om2).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect15)
{
  oserver1.connect(se1, oclient2, om2).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect16)
{
  oserver1.connect(se1, oserver2, sm2).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(ObjectEventRemoteConnect, Connect17)
{
  oserver1.connect(se1, oserver2, sm2).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

void slowDisconnect(qi::Promise<void> ready, qi::Promise<void> done, qi::Future<void> wait, qi::AnyObject obj, boost::shared_ptr<qi::SignalLink> link)
{
  ready.setValue(0);
  wait.wait();
  try {
    obj.disconnect(*link);
  }
  catch (...) {}
  done.setValue(0);
}

// disconnect multiple times in parallel
TEST_F(ObjectEventRemoteConnect, disconnectDeadlock)
{
  qi::Promise<void> doDisc, ready, discDone;
  boost::shared_ptr<qi::SignalLink> link = boost::make_shared<qi::SignalLink>();
  *link = oclient1.connect("fire1", boost::function<void(int)>(boost::bind(slowDisconnect, ready, discDone, doDisc.future(), oclient1, link))).value();
  oserver1.post("fire1", 24);
  ready.future().wait();
  // a callback is running, trigger other disconnect
  qi::Future<void> discDone2 = qi::async(boost::bind(boost::function<void(qi::AnyObject, qi::SignalLink)>(&qi::AnyObject::disconnect), oclient1, *link));
  // wait and disconnect inside the callback
  std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
  doDisc.setValue(0);

  // don't timeout
  discDone.future().wait();
  discDone2.wait();
}

TEST_F(ObjectEventRemoteConnect, multipleConnect)
{
  std::chrono::milliseconds additional_timeout{ 5 };//time to wait after having received the correct number of callbacks
  i = 0;
  qi::SignalLink link1 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  ASSERT_TRUE(qi::isValidSignalLink(link1));
  qi::SignalLink link2 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  ASSERT_TRUE(qi::isValidSignalLink(link2));
  qi::SignalLink link3 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  ASSERT_TRUE(qi::isValidSignalLink(link3));
  qi::SignalLink link4 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  ASSERT_TRUE(qi::isValidSignalLink(link4));

  ASSERT_NE(link1, link2);
  ASSERT_NE(link2, link3);
  ASSERT_NE(link3, link4);

  oclient2.post("fire2", 42);
  oclient2.post("fire2", 42);

  std::chrono::milliseconds waiting_time{ 0 };
  while(i._value != 8 && waiting_time < std::chrono::seconds{ 10 }) //waiting 10 seconds max
  {
    std::this_thread::sleep_for(additional_timeout); //additional timeout to wait for unwanted callback
    waiting_time += additional_timeout;
  }
  std::this_thread::sleep_for(additional_timeout);
  ASSERT_EQ(i._value, 8);

  //disconnect 3/4 callbacks
  oclient2.disconnect(link2);
  oclient2.disconnect(link3);
  oclient2.disconnect(link4);

  oclient2.post("fire2", 42);//post signal twice
  oclient2.post("fire2", 42);

  while(i._value != 10 && waiting_time < std::chrono::seconds{ 10 })//waiting 10 seconds max
  {
    std::this_thread::sleep_for(additional_timeout);
    waiting_time += additional_timeout;
  }
  std::this_thread::sleep_for(additional_timeout);
  ASSERT_EQ(i._value, 10);
  //i = 0;

  //reconnect callbacks
  link2 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  link3 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  link4 = oclient2.connect(se2, oserver1, callbackId).value(2000);

  oclient2.post("fire2", 42);//post signal twice
  oclient2.post("fire2", 42);

  while((i.load()) != 18 && waiting_time < std::chrono::seconds{ 10 })//waiting 10 seconds max
  {
    std::this_thread::sleep_for(additional_timeout); //additional timeout to wait for unwanted callback
    waiting_time += additional_timeout;
  }
  std::this_thread::sleep_for(additional_timeout); //additional timeout to wait for unwanted callback
  ASSERT_EQ(i.load(), 18);

  oclient2.disconnect(link1);
  oclient2.disconnect(link2);
  oclient2.disconnect(link3);
  oclient2.disconnect(link4);

  oclient2.post("fire2", 42);

  std::this_thread::sleep_for(additional_timeout); //additional timeout to wait for unwanted callback
  ASSERT_EQ(i.load(), 18);

}

TEST_F(ObjectEventRemoteConnect, serviceDirectoryEvent)
{
  const std::chrono::milliseconds additional_timeout{ 10 };//time to wait after having received the correct number of callbacks
  i = 0;
  qi::AnyObject sd = p1.client()->service(qi::Session::serviceDirectoryServiceName()).value();

  using SignalMap = std::map<unsigned int, qi::MetaSignal>;
  SignalMap s_map;
  s_map = sd.metaObject().signalMap();

  unsigned int signal_id = 0;
  for (SignalMap::iterator it = s_map.begin(); it!= s_map.end(); ++it)
  {
    if (it->second.name() == "serviceAdded")
    {
      signal_id = it->second.uid();
    }
  }
  ASSERT_NE(signal_id, 0U);
  sd.connect(signal_id, oserver1, secondCallbackId);
  sd.connect(signal_id, oserver1, secondCallbackId);
  sd.connect(signal_id, oserver1, secondCallbackId);
  sd.connect(signal_id, oserver1, secondCallbackId);

  ASSERT_TRUE(p1.server()->registerService("test", oserver1).hasValue(1000));

  std::chrono::milliseconds waiting_time{ 0 };
  while(i.load() != 4 && waiting_time < std::chrono::seconds{ 10 })
  {
    std::this_thread::sleep_for(additional_timeout);
    waiting_time += additional_timeout;
  }
  std::this_thread::sleep_for(additional_timeout);
  ASSERT_EQ(i.load(), 4);
}

TEST(ObjectEventRemoteDyn, PropertyConnectOnDynamicObject)
{
  qi::Property<int> prop;
  qi::DynamicObjectBuilder builder;
  builder.advertiseProperty("prop", &prop);

  TestSessionPair p;
  p.server()->registerService("Serv", builder.object());

  qi::Promise<int> prom;

  qi::AnyObject obj = p.client()->service("Serv").value();
  obj.connect("prop", boost::function<void(int)>([&prom](int i){
          prom.setValue(i);
        }));

  prop.set(42);
  ASSERT_EQ(42, prom.future().value());
}
