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
#include <qimessaging/session.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <testsession/testsessionpair.hpp>

qiLogCategory("test");
static qi::Promise<int> *payload;

void onFire(const int& pl)
{
  std::cout << "onFire:" << pl << std::endl;
  std::cout.flush();
  payload->setValue(pl);
}

class TestObject: public ::testing::Test
{
public:
  TestObject()
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

    ASSERT_GT(p.server()->registerService("coin", oserver), static_cast<unsigned int>(0));
    EXPECT_EQ(nbServices, p.server()->services(qi::Session::ServiceLocality_Local).value().size());

    std::vector<qi::ServiceInfo> services = p.client()->services();
    if (TestMode::getTestMode() == TestMode::Mode_Direct)
      EXPECT_EQ(2U, services.size());
    oclient = p.client()->service("coin");
    ASSERT_TRUE(oclient != 0);
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


TEST_F(TestObject, Simple)
{
  qi::SignalLink linkId = oclient.connect("fire", &onFire);
  EXPECT_LT((unsigned) 0, linkId);
  oserver.post("fire", 42);
  ASSERT_TRUE(payload->future().hasValue(2000));
  EXPECT_EQ(42, payload->future().value());
}


TEST_F(TestObject, RemoteEmit)
{
  qi::SignalLink linkId = oclient.connect("fire", &onFire);
  EXPECT_LT((unsigned) 0, linkId);
  oclient.post("fire", 43);
  ASSERT_TRUE(payload->future().hasValue(2000));
  EXPECT_EQ(43, payload->future().value());
}




TEST_F(TestObject, CoDeco)
{
  for (unsigned i=0; i<5; ++i)
  {
    payload->reset();
    qi::SignalLink linkId = oclient.connect("fire", &onFire);
    qiLogDebug() << "connected with " << linkId;
    int exp;
    EXPECT_GE(linkId, (unsigned) 0);
    oserver.post("fire", (int)(50 + i));
    ASSERT_TRUE(payload->future().hasValue(2000));
    exp = 50 + i;
    EXPECT_EQ(exp, payload->future().value());

    payload->reset();
    oserver.post("fire", (int)(51 + i));
    ASSERT_TRUE(payload->future().hasValue(2000));
    exp = 51 + i;
    EXPECT_EQ(exp, payload->future().value());

    oclient.disconnect(linkId).wait();

    payload->reset();
    oserver.post("fire", (int)(50 + i));
    EXPECT_ANY_THROW(payload->future().hasValue(200));
  }
}

int verifA = 0;
int verifB = 0;

void cb(int a, int b)
{
  verifA = a;
  verifB = b;
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
  clientOp.connect("sig1", &cb);

  qi::GenericFunctionParameters params;
  params.push_back(qi::AnyValue(42L).clone());
  params.push_back(qi::AnyValue(43L).clone());

  clientOp.metaPost("sig1", params);
  for(unsigned int i=0; i<100 && (verifA == 0 || verifB == 0); ++i)
    qi::os::msleep(10);
  ASSERT_EQ(42, verifA);
  ASSERT_EQ(43, verifB);
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
