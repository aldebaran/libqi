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
#include <qi/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <qi/testutils/testutils.hpp>

qiLogCategory("test");
static qi::Promise<int> *payload;

void onFire(const int& pl)
{
  std::cout << "onFire:" << pl << std::endl;
  std::cout.flush();
  payload->setValue(pl);
}

class ObjectEventRemote: public ::testing::Test
{
public:
  ObjectEventRemote()
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseSignal<const int&>("fire");
    oserver = ob.object();
  }

protected:
  void SetUp()
  {
    // In nightmare mode, there is a hidden service registered...
    unsigned int nbServices = TestMode::getTestMode() == TestMode::Mode_Nightmare ? 2 : 1;

    ASSERT_GT(p.server()->registerService("coin", oserver).value(), static_cast<unsigned int>(0));
    EXPECT_EQ(nbServices, p.server()->services(qi::Session::ServiceLocality_Local).value().size());

    std::vector<qi::ServiceInfo> services = p.client()->services().value();
    if (TestMode::getTestMode() == TestMode::Mode_Direct)
    {
      EXPECT_EQ(2U, services.size());
    }
    oclient = p.client()->service("coin").value();
    ASSERT_TRUE(oclient);
    payload = &prom;
  }

  void TearDown()
  {
    payload = 0;
  }

public:
  TestSessionPair      p;
  qi::Promise<int>     prom;
  qi::AnyObject        oserver;
  qi::AnyObject        oclient;
};


TEST_F(ObjectEventRemote, Simple)
{
  qi::SignalLink linkId = oclient.connect("fire", &onFire).value();
  ASSERT_TRUE(qi::isValidSignalLink(linkId));
  oserver.post("fire", 42);
  ASSERT_TRUE(payload->future().hasValue(2000));
  EXPECT_EQ(42, payload->future().value());
}


TEST_F(ObjectEventRemote, RemoteEmit)
{
  qi::SignalLink linkId = oclient.connect("fire", &onFire).value();
  ASSERT_TRUE(qi::isValidSignalLink(linkId));
  oclient.post("fire", 43);
  ASSERT_TRUE(payload->future().hasValue(2000));
  EXPECT_EQ(43, payload->future().value());
}




TEST_F(ObjectEventRemote, CoDeco)
{
  for (unsigned i=0; i<5; ++i)
  {
    *payload = qi::Promise<int>();
    qi::SignalLink linkId = oclient.connect("fire", &onFire).value();
    ASSERT_TRUE(qi::isValidSignalLink(linkId));
    qiLogDebug() << "connected with " << linkId;
    int exp;
    EXPECT_GE(linkId, (unsigned) 0);
    oserver.post("fire", (int)(50 + i));
    ASSERT_TRUE(payload->future().hasValue(2000));
    exp = 50 + i;
    EXPECT_EQ(exp, payload->future().value());

    *payload = qi::Promise<int>();
    oserver.post("fire", (int)(51 + i));
    ASSERT_TRUE(payload->future().hasValue(2000));
    exp = 51 + i;
    EXPECT_EQ(exp, payload->future().value());

    oclient.disconnect(linkId).wait();

    *payload = qi::Promise<int>();
    oserver.post("fire", (int)(50 + i));
    EXPECT_ANY_THROW(payload->future().hasValue(200));
  }
}

TEST(TestSignal, TwoLongPost)
{
  qi::DynamicObjectBuilder gob;
  qi::Signal<int, int> sig;
  gob.advertiseSignal("sig1", &sig);
  qi::AnyObject op = gob.object();

  TestSessionPair p;
  p.server()->registerService("MyService", op);
  qi::AnyObject clientOp = p.client()->service("MyService").value();

  std::atomic_int verifA {0};
  std::atomic_int verifB {0};
  qi::Promise<void> prom;
  auto fut = prom.future();
  clientOp.connect("sig1", [&](int a, int b){
    verifA = a;
    verifB = b;
    prom.setValue(nullptr);
  });

  qi::GenericFunctionParameters params;
  params.push_back(qi::AnyValue(42L).clone());
  params.push_back(qi::AnyValue(43L).clone());

  clientOp.metaPost("sig1", params);
  ASSERT_TRUE(test::finishesWithValue(fut));
  ASSERT_EQ(42, verifA);
  ASSERT_EQ(43, verifB);
}
