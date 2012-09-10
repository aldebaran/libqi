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

static qi::Promise<int> *payload1;

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



class TestObject: public ::testing::Test
{
public:
  TestObject()
  {
    qi::ObjectBuilder obs1, obs2;
    e1 = obs1.advertiseEvent<void (*)(const int&)>("fire1");
    e2 = obs2.advertiseEvent<void (*)(const int&)>("fire2");
    m1 = obs1.advertiseMethod("onFire1", &onFire1);
    m2 = obs2.advertiseMethod("onFire2", &onFire2);
    oserver1 = obs1.object();
    oserver2 = obs2.object();
  }

protected:
  void SetUp()
  {
    // Two objects with a fire event and a onFire method.
    ASSERT_TRUE(sd.listen("tcp://127.0.0.1:0"));
    ASSERT_TRUE(session1.connect(sd.listenUrl()));
    ASSERT_TRUE(session2.connect(sd.listenUrl()));
    ASSERT_TRUE(session1.listen("tcp://0.0.0.0:0"));
    ASSERT_TRUE(session2.listen("tcp://0.0.0.0:0"));
    ASSERT_GT(session1.registerService("coin1", oserver1).wait(), 0);
    ASSERT_GT(session2.registerService("coin2", oserver2).wait(), 0);
    EXPECT_EQ(1U, session1.services(qi::Session::ServiceLocality_Local).value().size());
    EXPECT_EQ(1U, session2.services(qi::Session::ServiceLocality_Local).value().size());
    ASSERT_TRUE(sclient.connect(sd.listenUrl()));
    std::vector<qi::ServiceInfo> services = sclient.services();
    EXPECT_EQ(3U, services.size());
    oclient1 = sclient.service("coin1");
    oclient2 = sclient.service("coin2");
    payload1 = &prom1;
    payload2 = &prom2;
  }

  void TearDown()
  {
    payload1 = payload2 = 0;
    sclient.close();
    session1.close();
    session2.close();
    sd.close();
  }

public:
  unsigned int e1, e2;
  unsigned int m1, m2;
  qi::Promise<int>     prom1, prom2;
  qi::ServiceDirectory sd;
  qi::Session          session1, session2;
  qi::Object           oserver1, oserver2;
  qi::Session          sclient;
  qi::Object           oclient1, oclient2;
};


// We try to test all possible combinations of where we connect, where we emit.

TEST_F(TestObject, Connect1)
{
  oclient1.connect(e1, oclient1, m1);
  oclient1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload1->future().wait(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect2)
{
  oclient1.connect(e1, oclient1, m1);
  qi::os::msleep(800);
  oserver1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload1->future().wait(2000));
  EXPECT_EQ(12, payload1->future().value());
}


TEST_F(TestObject, Connect3)
{
  oclient1.connect(e1, oserver1, m1);
  qi::os::msleep(800);
  oserver1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload1->future().wait(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect4)
{
  oclient1.connect(e1, oserver1, m1);
  oclient1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload1->future().wait(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect5)
{
  oserver1.connect(e1, oclient1, m1);
  qi::os::msleep(800);
  oclient1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload1->future().wait(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect6)
{
  oserver1.connect(e1, oclient1, m1);
  oserver1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload1->future().wait(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect7)
{
  oserver1.connect(e1, oserver1, m1);
  oserver1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload1->future().wait(2000));
  EXPECT_EQ(12, payload1->future().value());
}

TEST_F(TestObject, Connect8)
{
  oserver1.connect(e1, oserver1, m1);
  qi::os::msleep(800);
  oclient1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload1->future().wait(2000));
  EXPECT_EQ(12, payload1->future().value());
}









TEST_F(TestObject, Connect10)
{
  oclient1.connect(e1, oclient2, m2);
  oclient1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload2->future().wait(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect11)
{
  oclient1.connect(e1, oclient2, m2);
  qi::os::msleep(800);
  oserver1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload2->future().wait(2000));
  EXPECT_EQ(12, payload2->future().value());
}


TEST_F(TestObject, Connect12)
{
  oclient1.connect(e1, oserver2, m2);
  qi::os::msleep(800);
  oserver1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload2->future().wait(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect13)
{
  oclient1.connect(e1, oserver2, m2);
  oclient1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload2->future().wait(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect14)
{
  oserver1.connect(e1, oclient2, m2);
  qi::os::msleep(800);
  oclient1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload2->future().wait(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect15)
{
  oserver1.connect(e1, oclient2, m2);
  oserver1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload2->future().wait(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect16)
{
  oserver1.connect(e1, oserver2, m2);
  oserver1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload2->future().wait(2000));
  EXPECT_EQ(12, payload2->future().value());
}

TEST_F(TestObject, Connect17)
{
  oserver1.connect(e1, oserver2, m2);
  qi::os::msleep(800);
  oclient1.emitEvent("fire1", 12);
  ASSERT_TRUE(payload2->future().wait(2000));
  EXPECT_EQ(12, payload2->future().value());
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
