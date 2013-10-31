/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/



#include <map>
#include <gtest/gtest.h>
#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qi/atomic.hpp>
#include <qimessaging/session.hpp>

#include <testsession/testsessionpair.hpp>

qiLogCategory("test");

/*
 * Bound object do not have the same event id as "raw" object.
 * BoundObject add an offset of 10.
 *
 * So we take both id, the local and the remote one.
 */

static qi::Promise<int> *payload1;
qi::Atomic<int> i = 0;

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

void callbackCounter(const int& value)
{
  ++i;
  std::cout << "callback called " << i._value <<" times" << std::endl;
}

void callbackCounterBis(const int& value, std::string secondValue)
{
  ++i;
  std::cout << "callback called " << *i <<" times" << std::endl;
}

class TestObject: public ::testing::Test
{
public:
  TestObject() : p2(p1)
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

    std::vector<qi::ServiceInfo> services = p1.client()->services();
    EXPECT_EQ(nbConnectedServices, services.size());
    services = p2.client()->services();
    EXPECT_EQ(nbConnectedServices, services.size());

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

TEST_F(TestObject, Connect1)
{
  oclient1.connect(oe1, oclient1, om1).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect2)
{
  oclient1.connect(oe1, oclient1, om1).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}


TEST_F(TestObject, Connect3)
{
  oclient1.connect(oe1, oserver1, sm1).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect4)
{
  oclient1.connect(oe1, oserver1, sm1).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect5)
{
  oserver1.connect(se1, oclient1, om1).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect6)
{
  oserver1.connect(se1, oclient1, om1).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect7)
{
  oserver1.connect(se1, oserver1, sm1).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect8)
{
  oserver1.connect(se1, oserver1, sm1).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload1->future().hasValue(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect10)
{
  oclient1.connect(oe1, oclient2, om2).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect11)
{
  oclient1.connect(oe1, oclient2, om2).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}


TEST_F(TestObject, Connect12)
{
  oclient1.connect(oe1, oserver2, sm2).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect13)
{
  oclient1.connect(oe1, oserver2, sm2).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect14)
{
  oserver1.connect(se1, oclient2, om2).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect15)
{
  oserver1.connect(se1, oclient2, om2).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect16)
{
  oserver1.connect(se1, oserver2, sm2).wait(2000);
  oserver1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect17)
{
  oserver1.connect(se1, oserver2, sm2).wait(2000);
  oclient1.post("fire1", 12);
  ASSERT_TRUE(payload2->future().hasValue(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, multipleConnect)
{
  int additional_timeout = 5;//time to wait after having received the correct number of callbacks
  qi::SignalLink link1 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  qi::SignalLink link2 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  qi::SignalLink link3 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  qi::SignalLink link4 = oclient2.connect(se2, oserver1, callbackId).value(2000);

  ASSERT_NE(link1, link2);
  ASSERT_NE(link2, link3);
  ASSERT_NE(link3, link4);

  oclient2.post("fire2", 42);
  oclient2.post("fire2", 42);

  int waiting_time = 0;
  while(i._value != 8 && waiting_time < 10000)//waiting 10 seconds max
  {
    qi::os::msleep(additional_timeout); //additional timeout to wait for unwanted callback
    waiting_time += additional_timeout;
  }
  qi::os::msleep(additional_timeout);
  ASSERT_EQ(i._value, 8);

  //disconnect 3/4 callbacks
  oclient2.disconnect(link2);
  oclient2.disconnect(link3);
  oclient2.disconnect(link4);

  oclient2.post("fire2", 42);//post signal twice
  oclient2.post("fire2", 42);

  while(i._value != 10 && waiting_time < 10000)//waiting 10 seconds max
  {
    qi::os::msleep(additional_timeout); //additional timeout to wait for unwanted callback
    waiting_time += additional_timeout;
  }
  qi::os::msleep(additional_timeout); //additional timeout to wait for unwanted callback
  ASSERT_EQ(i._value, 10);
  //i = 0;

  //reconnect callbacks
  link2 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  link3 = oclient2.connect(se2, oserver1, callbackId).value(2000);
  link4 = oclient2.connect(se2, oserver1, callbackId).value(2000);

  oclient2.post("fire2", 42);//post signal twice
  oclient2.post("fire2", 42);

  while((*i) != 10 && waiting_time < 10000)//waiting 10 seconds max
  {
    qi::os::msleep(additional_timeout); //additional timeout to wait for unwanted callback
    waiting_time += additional_timeout;
  }
  qi::os::msleep(additional_timeout); //additional timeout to wait for unwanted callback
  ASSERT_EQ(*i, 18);

  oclient2.disconnect(link1);
  oclient2.disconnect(link2);
  oclient2.disconnect(link3);
  oclient2.disconnect(link4);

  oclient2.post("fire2", 42);

  qi::os::msleep(additional_timeout); //additional timeout to wait for unwanted callback
  ASSERT_EQ(*i, 18);

}

TEST_F(TestObject, serviceDirectoryEvent)
{
  i = 0;
  qi::AnyObject sd = p1.client()->service("ServiceDirectory");

  typedef std::map<unsigned int, qi::MetaSignal> SignalMap;
  SignalMap s_map;
  s_map = sd.metaObject().signalMap();

  unsigned int signal_id;
  for (SignalMap::iterator it = s_map.begin(); it!= s_map.end(); ++it)
  {
    if (it->second.name() == "serviceAdded")
    {
      signal_id = it->second.uid();
    }
  }
  sd.connect(signal_id, oserver1, secondCallbackId);
  sd.connect(signal_id, oserver1, secondCallbackId);
  sd.connect(signal_id, oserver1, secondCallbackId);
  sd.connect(signal_id, oserver1, secondCallbackId);

  ASSERT_TRUE(p1.server()->registerService("test", oserver1).hasValue(1000));

  int waiting_time = 0;
  while(*i != 4 && waiting_time < 10000)
  {
    qi::os::msleep(10);
    waiting_time += 10;
  }
  qi::os::msleep(10);
  ASSERT_EQ(*i, 4);
}

int main(int argc, char *argv[])
{
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
