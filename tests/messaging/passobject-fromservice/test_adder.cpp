#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/type/objectfactory.hpp>

#include <qi/session.hpp>

#include <adder.hpp>

qiLogCategory("test.adder");

using qi::Object;
using qi::AnyObject;
using qi::WeakObject;
void inc(int& v)
{
  ++v;
}
void init(qi::Session& server)
{
  qi::loadObject("adder_fromservice"); // will only work first time
  server.registerService("AdderService", qi::createObject("AdderService"));
}


TEST(Test, One)
{
  qi::Session server, client;
  server.listenStandalone("tcp://localhost:0");
  client.connect(server.endpoints()[0]);
  server.listen("tcp://localhost:0");
  init(server);
  // test building the proxy from a GenericObject ourselve
  Object<Adder> ap(client.service("AdderService").value().call<qi::AnyObject>("create"));
  // Test getting it directly using typesystem conversion.
  Object<Adder> ap2 = client.service("AdderService").value().call<Object<Adder> >("create");
  ASSERT_TRUE(!!ap2);
  ap->value.set(2);
  qiLogVerbose() << "MakeTask";
  Object<AddTask> atask = ap->makeTask(3);
  qiLogVerbose() << "add";
  ASSERT_EQ(6, atask->add(1));
  qiLogVerbose() << "done";
  Object<AddTask> atask2 = ap->makeTask(4);
  ASSERT_EQ(7, atask2->add(1));
  int onChangeHit = 0;
  atask->onChange.connect(boost::bind(&inc, boost::ref(onChangeHit)));
  atask2->onChange.connect(boost::bind(&inc, boost::ref(onChangeHit)));
  qiLogVerbose() << "SetProp";
  ap->value.set(3);
  qiLogVerbose() << "Wait";
  for (unsigned i=0; i<10 && onChangeHit !=2; ++i) qi::os::msleep(50);
  qiLogVerbose() << "Ready";
  ASSERT_EQ(2, onChangeHit);
  ASSERT_EQ(7, atask->add(1));
  atask.reset();
  qi::os::msleep(10);
  ap->value.set(1);
  for (unsigned i=0; i<10 && onChangeHit !=3; ++i) qi::os::msleep(50);
  ASSERT_EQ(3, onChangeHit);
  ASSERT_EQ(6, atask2->add(1));
}

TEST(Test, SharedPtr)
{
  qi::Session server, client;
  server.listenStandalone("tcp://localhost:0");
  client.connect(server.endpoints()[0]);
  server.listen("tcp://localhost:0");
  init(server);
  // test building the proxy from a GenericObject ourselve
  Object<Adder> ap(client.service("AdderService").value().call<qi::AnyObject>("create"));
  ASSERT_TRUE(!!ap);
  ap->value.set(2);
  boost::shared_ptr<AddTask> atask = ap->makeTask(3).asSharedPtr();
  ASSERT_EQ(1, ap->nTasks());
  qi::os::msleep(100);
  ASSERT_EQ(1, ap->nTasks());
  atask.reset();
  for (unsigned i=0; i<10 && ap->nTasks() !=0; ++i) qi::os::msleep(50);
  ASSERT_EQ(0, ap->nTasks());
  atask = ap.call<boost::shared_ptr<AddTask> >("makeTask", 3);
  ASSERT_EQ(1, ap->nTasks());
  qi::os::msleep(100);
  ASSERT_EQ(1, ap->nTasks());
  atask.reset();
  for (unsigned i=0; i<10 && ap->nTasks() !=0; ++i) qi::os::msleep(50);
  ASSERT_EQ(0, ap->nTasks());
}

TEST(Test, AnyObject)
{ // test the makeTask returning a AnyObject instead of Object<Task>
  qi::Session server, client;
  server.listenStandalone("tcp://localhost:0");
  client.connect(server.endpoints()[0]);
  server.listen("tcp://localhost:0");
  init(server);
  // test building the proxy from a GenericObject ourselve
  Object<Adder> ap(client.service("AdderService").value().call<qi::AnyObject>("create"));
  ASSERT_TRUE(!!ap);
  ap->value.set(2);

  AnyObject atask = ap->makeAnyTask(3);
  ASSERT_EQ(1, ap->nTasks());
  qi::os::msleep(100);
  ASSERT_EQ(1, ap->nTasks());
  ASSERT_EQ(5, atask.call<int>("add", 0).value());
  atask.reset();
  for (unsigned i=0; i<10 && ap->nTasks() !=0; ++i) qi::os::msleep(50);
  ASSERT_EQ(0, ap->nTasks());

  atask = ap.call<AnyObject>("makeAnyTask", 3);
  ASSERT_EQ(1, ap->nTasks());
  ASSERT_EQ(5, atask.call<int>("add", 0).value());
  qi::os::msleep(100);
  ASSERT_EQ(1, ap->nTasks());
  atask.reset();
  for (unsigned i=0; i<10 && ap->nTasks() !=0; ++i) qi::os::msleep(50);
  ASSERT_EQ(0, ap->nTasks());

  Object<AddTask> task = ap.call<AnyObject>("makeAnyTask", 3);
  ASSERT_EQ(1, ap->nTasks());
  ASSERT_EQ(5, task->add(0));
  qi::os::msleep(100);
  ASSERT_EQ(1, ap->nTasks());
  task.reset();
  for (unsigned i=0; i<10 && ap->nTasks() !=0; ++i) qi::os::msleep(50);
  ASSERT_EQ(0, ap->nTasks());
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  //TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
