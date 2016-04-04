/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qi/session.hpp>
#include <qi/anymodule.hpp>
#include <testsession/testsessionpair.hpp>

qiLogCategory("test");

void test_service(const qi::AnyObject &o)
{
  ASSERT_TRUE(o);
  int res = o.call<int>("testMethod", 12);
  ASSERT_EQ(13, res);
}

TEST(Module, pass_obj)
{
  TestSessionPair p;

  qi::SessionPtr s = p.server();

  s->loadService("naoqi.testanymodule.test");
  qi::AnyObject o = s->service("test");

  qiLogInfo() << "check first service" << std::endl;
  test_service(o);

  qi::AnyObject o2 = o.call<qi::AnyObject>("make_other_object");

  s->registerService("test_bis", o2);

  std::cout << "check second service" << std::endl;
  test_service(o2);
  qi::AnyObject o3 = s->service("test_bis");
  test_service(o3);

  s->close();
}

TEST(Module, load_received_obj)
{
  TestSessionPair p;

  qi::SessionPtr s = p.server();

  s->loadService("naoqi.testanymodule.test");
  qi::AnyObject o = s->service("test");
  test_service(o);

  s->registerService("test_bis", o);
  qi::AnyObject o2 = s->service("test_bis");
  test_service(o2);

  s->close();
}

TEST(Module, unregister_obj)
{
  TestMode::forceTestMode(TestMode::Mode_SD);
  TestSessionPair p;

  qi::SessionPtr s = p.server();

  int index = s->loadService("naoqi.testanymodule.test");
  qi::AnyObject o = s->service("test");
  test_service(o);

  s->unregisterService(index).wait();
  ASSERT_ANY_THROW(s->service("test"));

  s->close();
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

class ObjectEmitter
{
public:
  void emitObject(qi::AnyObject o) { QI_EMIT onTruc(o); }
  qi::Signal<qi::AnyObject> onTruc;
};

QI_REGISTER_OBJECT(ObjectEmitter, emitObject, onTruc)

TEST(Module, pass_obj_made_from_module)
{
  qi::AnyModule testModule = qi::import("naoqi.testanymodule");
  auto obj = testModule.call<qi::AnyObject>("test");
  ASSERT_EQ(1, obj.call<int>("testMethod", 0)); // just checking, in case of

  TestSessionPair p;
  p.server()->registerService("plop", boost::make_shared<ObjectEmitter>());

  qi::AnyObject remotePlop = p.client()->service("plop");
  qi::Promise<void> receivingObject;
  remotePlop.connect("onTruc", boost::function<void(qi::AnyObject)>([&](qi::AnyObject o)
  {
    ASSERT_EQ(1, o.call<int>("testMethod", 0)); // this is the real test
    receivingObject.setValue(0);
  }));
  remotePlop.async<void>("emitObject", obj);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, receivingObject.future().waitFor(qi::MilliSeconds(1000)));
}

class ObjectEmitterFactoryService
{
public:
  qi::AnyObject makeObjectEmitter() { return boost::make_shared<ObjectEmitter>(); }
};

QI_REGISTER_OBJECT(ObjectEmitterFactoryService, makeObjectEmitter)

TEST(Module, pass_obj_made_from_module_to_an_obj_made_from_service)
{
  qi::AnyModule testModule = qi::import("naoqi.testanymodule");
  auto obj = testModule.call<qi::AnyObject>("test");
  ASSERT_EQ(1, obj.call<int>("testMethod", 0)); // just checking, in case of

  TestSessionPair p;
  p.server()->registerService("EmitterFactory", boost::make_shared<ObjectEmitterFactoryService>());

  qi::AnyObject emitterFactory = p.client()->service("EmitterFactory");
  auto emitter = emitterFactory.call<qi::AnyObject>("makeObjectEmitter");

  qi::Promise<void> receivingObject;
  emitter.connect("onTruc", boost::function<void(qi::AnyObject)>([&](qi::AnyObject o)
  {
    int i = o.call<int>("testMethod", 0);
    ASSERT_EQ(1, i); // this is the real test
    receivingObject.setValue(0);
  }));
  emitter.async<void>("emitObject", obj);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, receivingObject.future().waitFor(qi::MilliSeconds(1000)));
}
