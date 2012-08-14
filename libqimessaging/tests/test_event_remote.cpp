/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/



#include <map>
#include <gtest/gtest.h>
#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/service_directory.hpp>

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
  }

protected:
  void SetUp()
  {
    ASSERT_TRUE(sd.listen("tcp://127.0.0.1:0"));
    ASSERT_TRUE(session.connect(sd.listenUrl()));
    ASSERT_TRUE(session.waitForConnected());

    oserver.advertiseEvent<void (*)(const int&)>("fire");

    ASSERT_TRUE(session.listen("tcp://0.0.0.0:0"));
    ASSERT_GT(session.registerService("coin", &oserver).wait(), 0);
    EXPECT_EQ(1U, session.registeredServices().size());

    ASSERT_TRUE(sclient.connect(sd.listenUrl()));
    ASSERT_TRUE(sclient.waitForConnected());
    std::vector<qi::ServiceInfo> services = sclient.services();
    EXPECT_EQ(2U, services.size());
    oclient = sclient.service("coin");
    payload = &prom;
  }

  void TearDown()
  {
    payload = 0;
    sclient.disconnect();
    sclient.close();
    session.disconnect();
    sd.close();
  }

public:
  qi::Promise<int>     prom;
  qi::ServiceDirectory sd;
  qi::Session          session;
  qi::Object           oserver;
  qi::Session          sclient;
  qi::Object          *oclient;
};


TEST_F(TestObject, Simple)
{
  int linkId = oclient->connect("fire", &onFire);
  qi::os::msleep(800);
  EXPECT_LT(0, linkId);
  oserver.emitEvent("fire", 42);
  ASSERT_TRUE(payload->future().wait(2000));
  EXPECT_EQ(42, payload->future().value());
}


TEST_F(TestObject, RemoteEmit)
{
  int linkId = oclient->connect("fire", &onFire);
  qi::os::msleep(800);
  EXPECT_LT(0, linkId);
  oclient->emitEvent("fire", 43);
  ASSERT_TRUE(payload->future().wait(2000));
  EXPECT_EQ(43, payload->future().value());
}




TEST_F(TestObject, CoDeco)
{
  for (unsigned i=0; i<5; ++i)
  {
    payload->reset();
    int linkId = oclient->connect("fire", &onFire);
    int exp;
    qi::os::msleep(800);
    EXPECT_GE(linkId, 0);
    oserver.emitEvent("fire", (int)(50 + i));
    ASSERT_TRUE(payload->future().wait(2000));
    exp = 50 + i;
    EXPECT_EQ(exp, payload->future().value());

    payload->reset();
    oserver.emitEvent("fire", (int)(51 + i));
    ASSERT_TRUE(payload->future().wait(2000));
    exp = 51 + i;
    EXPECT_EQ(exp, payload->future().value());

    oclient->disconnect(linkId);

    payload->reset();
    oserver.emitEvent("fire", (int)(50 + i));
    ASSERT_FALSE(payload->future().wait(200));
  }
}

int main(int argc, char *argv[])
{
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
