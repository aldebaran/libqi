#include <gtest/gtest.h>
#include <qi/application.hpp>

#include <qimessaging/session.hpp>
#include <qimessaging/servicedirectory.hpp>

#include <addtask_proxy.hpp>
#include <adder_proxy.hpp>

qiLogCategory("test.adder");

int nDead = 0;

// We the client provide the Adder
class Adder1
{
public:
  ~Adder1() { ++nDead;}
  int add(int v) { return v;}
  qi::Signal<int> onChange;
  qi::Property<int> uid;
};

QI_REGISTER_OBJECT(Adder1, add, onChange, uid);

class Adder2
{
public:
  ~Adder2() { ++nDead;}
  int val() { return v;}
  int add(int v2) { return v + v2;}
  qi::Signal<int> onChange;
  qi::Property<int> uid;
  int v;
};

QI_REGISTER_OBJECT(Adder2, val, add, onChange, uid);


void inc(int& v)
{
  ++v;
}
TEST(Test, One)
{
  qi::ServiceDirectory sd;
  sd.listen("tcp://localhost:0");
  qi::Session server, client;
  server.connect(sd.endpoints()[0]);
  client.connect(sd.endpoints()[0]);
  server.listen("tcp://localhost:0");
  ASSERT_EQ(1u, server.loadService("adder_fromclient").size());
  // test building the proxy from a GenericObject ourselve
  AdderProxy ap(client.service("AdderService").value()->call<qi::AnyObject>("create"));
  // Test getting it directly using typesystem conversion.
  AdderProxyPtr ap2 = client.service("AdderService").value()->call<AdderProxyPtr>("create");
  ASSERT_TRUE(!!ap2);
  ap.value.set(2);
  qiLogInfo() << "Creating adders";
  boost::shared_ptr<Adder1> a1(new Adder1);
  boost::shared_ptr<Adder2> a2(new Adder2);
  a2->v = 1;
  int onChangeHit = 0;
  a2->onChange.connect(boost::bind(&inc, boost::ref(onChangeHit)));
  qiLogInfo() << "Registering adders";
  ap.registerTask(a1);
  ap.registerTask(a2);
  qiLogInfo() << "Triggering set";
  ap.value.set(2);
  for (unsigned i=0; i<10 && onChangeHit !=1; ++i) qi::os::msleep(50);
  ASSERT_EQ(1, onChangeHit);
  qiLogInfo() << "addAll";
  ASSERT_EQ(5, ap.addAll(1)); // 2+2+1
  qiLogInfo() << "reseting adders";
  a1.reset();
  ap.popTask();
  a2.reset();
  for (unsigned i=0; i<10 && nDead !=1; ++i) qi::os::msleep(50);
  ASSERT_EQ(1, nDead);

  ASSERT_EQ(2, ap.addAll(1));
  ap.popTask();
  for (unsigned i=0; i<10 && nDead !=2; ++i) qi::os::msleep(50);
  ASSERT_EQ(2, nDead);
  ASSERT_EQ(0, ap.addAll(1));
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  //TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
