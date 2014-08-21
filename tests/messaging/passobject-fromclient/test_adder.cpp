#include <gtest/gtest.h>
#include <qi/application.hpp>

#include <qi/session.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/type/objectfactory.hpp>

#include "adder.hpp"

using qi::Object;

qiLogCategory("test.adder");

class LogReseter
{
public:
  LogReseter() {}
  ~LogReseter() { qi::log::removeLogHandler("consoleloghandler");}
};
int nDead = 0;

// We the client provide the Adder
class Adder1: public AddTask
{
public:
  Adder1() { qiLogDebug() << "Adder1 " << this;}
  ~Adder1() { ++nDead;}
  int add(int v) { return v;}
};

QI_REGISTER_IMPLEMENTATION(AddTask, Adder1);

class Adder2: public AddTask
{
public:
  Adder2()
    : v(0)
  { qiLogDebug() << "Adder2 " << this;}
  ~Adder2() { ++nDead;}
  int val() { return v;}
  int add(int v2) { return v + v2;}
  int v;
};

QI_REGISTER_IMPLEMENTATION(AddTask, Adder2);



void inc(int& v)
{
  ++v;
}
void init(qi::Session& server)
{
  qi::loadObject("adder_fromclient"); // will only work first time
  server.registerService("AdderService", qi::createObject("AdderService"));
}
TEST(Test, One)
{
  qi::Session server, client;
  server.listenStandalone("tcp://localhost:0");
  client.connect(server.endpoints()[0]);
  init(server);
  // test building the proxy from a GenericObject ourselve
  Object<Adder> ap(client.service("AdderService").value().call<qi::AnyObject>("create"));
  // Test getting it directly using typesystem conversion.
  Object<Adder> ap2 = client.service("AdderService").value().call<Object<Adder> >("create");
  ASSERT_TRUE(!!ap2);
  ap->value.set(2);
  qiLogInfo() << "Creating adders";
  qi::Object<Adder1> a1(new Adder1);
  qi::Object<Adder2> a2(new Adder2);
  LogReseter lr;
  a2->v = 1;
  int onChangeHit = 0;
  a2->onChange.connect(boost::bind(&inc, boost::ref(onChangeHit)));
  qiLogInfo() << "Registering adders";
  ap->registerTask(a1);
  ap->registerTask(a2);
  qiLogInfo() << "Triggering set";
  ap->value.set(2);
  for (unsigned i=0; i<10 && onChangeHit !=1; ++i) qi::os::msleep(50);
  ASSERT_EQ(1, onChangeHit);
  qiLogInfo() << "addAll";
  ASSERT_EQ(5, ap->addAll(1)); // 2+2+1
  qiLogInfo() << "reseting adders";
  a1.reset();
  qiLogInfo() << "popping task";
  ap->popTask(); // pops a2
  qiLogInfo() << "reseting 2";
  a2.reset();
  qiLogInfo() << "waiting";
  for (unsigned i=0; i<10 && nDead !=1; ++i) qi::os::msleep(50);
  qiLogInfo() << "done";
  ASSERT_EQ(1, nDead);

  ASSERT_EQ(2, ap->addAll(1));
  qiLogInfo() << "popping task";
  ap->popTask();
  qiLogInfo() << "waiting";
  for (unsigned i=0; i<10 && nDead !=2; ++i) qi::os::msleep(50);
  qiLogInfo() << "done";
  ASSERT_EQ(2, nDead);
  ASSERT_EQ(0, ap->addAll(1));
}

TEST(Test, explicitSharedPtr)
{
  nDead = 0;
  qi::Session server, client;
  server.listenStandalone("tcp://localhost:0");
  client.connect(server.endpoints()[0]);
  init(server);
  // test building the proxy from a GenericObject ourselve
  Object<Adder> ap(client.service("AdderService").value().call<qi::AnyObject>("create"));
  ASSERT_TRUE(!!ap);
  boost::shared_ptr<Adder1> a1(new Adder1);
  ap->value.set(2);
  ap->registerTask(a1);
  ASSERT_EQ(2, ap->addAll(1));
  a1.reset();
  qiLogInfo() << "popping task";
  ap->popTask();
  qiLogInfo() << "waiting";
  for (unsigned i=0; i<10 && nDead !=1; ++i) qi::os::msleep(50);
  qiLogInfo() << "done";
  ASSERT_EQ(1, nDead);
 }

 TEST(Test, explicitSharedPtr2)
{
  nDead = 0;
  qi::Session server, client;
  server.listenStandalone("tcp://localhost:0");
  client.connect(server.endpoints()[0]);
  init(server);
  // test building the proxy from a GenericObject ourselve
  Object<Adder> ap(client.service("AdderService").value().call<qi::AnyObject>("create"));
  ASSERT_TRUE(!!ap);
  // test with reversed ordering
  boost::shared_ptr<Adder1> a1 = boost::shared_ptr<Adder1>(new Adder1);
  ap->value.set(2);
  ap->registerTask(a1);
  qi::os::msleep(200);
  ASSERT_EQ(2, ap->addAll(1));
  ap->popTask();
  qi::os::msleep(200);
  a1.reset();
  for (unsigned i=0; i<10 && nDead !=1; ++i) qi::os::msleep(50);
  ASSERT_EQ(1, nDead);

  // test with registerAnyTask
  a1 = boost::shared_ptr<Adder1>(new Adder1);
  ap->registerAnyTask(a1);
  qi::os::msleep(200);
  ASSERT_EQ(2, ap->addAll(1));
  ap->popTask();
  qi::os::msleep(200);
  a1.reset();
  for (unsigned i=0; i<10 && nDead !=1; ++i) qi::os::msleep(50);
  ASSERT_EQ(2, nDead);
}

TEST(Test, explicitSharedPtr3)
{
  nDead = 0;
  qi::Session server, client;
  server.listenStandalone("tcp://localhost:0");
  client.connect(server.endpoints()[0]);
  init(server);
  // test building the proxy from a GenericObject ourselve
  Object<Adder> ap(client.service("AdderService").value().call<qi::AnyObject>("create"));
  ASSERT_TRUE(!!ap);
  // test with generic call for registerTask (conversion path differs)
  boost::shared_ptr<Adder1> a1 = boost::shared_ptr<Adder1>(new Adder1);
  ap->value.set(2);
  ap.call<void>("registerTask", a1);
  qi::os::msleep(200);
  ASSERT_EQ(2, ap->addAll(1));
  a1.reset();
  ap->popTask();
  for (unsigned i=0; i<10 && nDead !=1; ++i) qi::os::msleep(50);
  ASSERT_EQ(1, nDead);

  // test with registerAnyTask
  a1 = boost::shared_ptr<Adder1>(new Adder1);
  ap.call<void>("registerAnyTask", a1);
  qi::os::msleep(200);
  ASSERT_EQ(2, ap->addAll(1));
  ap->popTask();
  qi::os::msleep(200);
  a1.reset();
  for (unsigned i=0; i<10 && nDead !=1; ++i) qi::os::msleep(50);
  ASSERT_EQ(2, nDead);
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  //TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
