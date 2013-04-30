#include <gtest/gtest.h>
#include <qi/application.hpp>

#include <qimessaging/session.hpp>
#include <qimessaging/servicedirectory.hpp>

#include <addtask_proxy.hpp>
#include <adder_proxy.hpp>

qiLogCategory("test.adder");


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
  ASSERT_EQ(1u, server.loadService("adder").size());
  // test building the proxy from a GenericObject ourselve
  AdderProxy ap(client.service("AdderService").value()->call<qi::ObjectPtr>("create"));
  // Test getting it directly using typesystem conversion.
  AdderProxyPtr ap2 = client.service("AdderService").value()->call<AdderProxyPtr>("create");
  ASSERT_TRUE(!!ap2);
  ap.value.set(2);
  qiLogVerbose() << "MakeTask";
  AddTaskProxyPtr atask = ap.makeTask(3);
  qiLogVerbose() << "add";
  ASSERT_EQ(6, atask->add(1));
  qiLogVerbose() << "done";
  AddTaskProxyPtr atask2 = ap.makeTask(4);
  ASSERT_EQ(7, atask2->add(1));
  int onChangeHit = 0;
  atask->onChange.connect(boost::bind(&inc, boost::ref(onChangeHit)));
  atask2->onChange.connect(boost::bind(&inc, boost::ref(onChangeHit)));
  ap.value.set(3);
  for (unsigned i=0; i<10 && onChangeHit !=2; ++i) qi::os::msleep(50);
  ASSERT_EQ(2, onChangeHit);
  ASSERT_EQ(7, atask->add(1));
  atask.reset();
  qi::os::msleep(10);
  ap.value.set(1);
  for (unsigned i=0; i<10 && onChangeHit !=3; ++i) qi::os::msleep(50);
  ASSERT_EQ(3, onChangeHit);
  ASSERT_EQ(6, atask2->add(1));
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  //TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
