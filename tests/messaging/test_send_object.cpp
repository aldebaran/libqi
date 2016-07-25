/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qi/session.hpp>
#include <qi/signalspy.hpp>
#include <qi/anymodule.hpp>
#include <testsession/testsessionpair.hpp>

qiLogCategory("test");

int timeoutMs = 300;
qi::Duration timeout = qi::MilliSeconds(timeoutMs);

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  qi::log::addFilter("qi.*", qi::LogLevel_Debug);
  qi::log::addFilter("qigateway.*", qi::LogLevel_Debug);
  qi::log::addFilter("qimessaging.*", qi::LogLevel_Debug);
  return RUN_ALL_TESTS();
}

void test_service(const qi::AnyObject &o)
{
  ASSERT_TRUE(o);
  int res = o.call<int>("testMethod", 12);
  ASSERT_EQ(13, res);
}

TEST(SendObject, pass_obj)
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

TEST(SendObject, load_received_obj)
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

TEST(SendObject, unregister_obj)
{
  TestSessionPair p;

  qi::SessionPtr s = p.server();

  int index = s->loadService("naoqi.testanymodule.test");
  qi::AnyObject o = s->service("test");
  test_service(o);

  s->unregisterService(index).wait();
  ASSERT_ANY_THROW(s->service("test"));

  s->close();
}

class SubObjectToPing
{
public:
  void subping() { qiLogInfo() << "subping !"; }
};

QI_REGISTER_OBJECT(SubObjectToPing, subping)

class ObjectToPing
{
public:
  ObjectToPing() { propToPing.set(boost::make_shared<SubObjectToPing>()); }
  void ping() { qiLogInfo() << "ping !" << std::endl; }
  qi::Property<qi::AnyObject> propToPing;
};

QI_REGISTER_OBJECT(ObjectToPing, ping, propToPing)

class ObjectEmitter
{
public:
  ObjectEmitter()
  {
    object.set(boost::make_shared<ObjectToPing>());
  }

  void emitObject(qi::AnyObject o) { QI_EMIT onTruc(o); }
  qi::AnyObject identity(qi::AnyObject o) {return o;}
  void receiveObject(qi::AnyObject o)
  {
    auto gettingProperty = o.property<void>("propToPing");
    auto state = gettingProperty.waitFor(timeout);
    std::stringstream errorMessage;
    errorMessage << "Unexpected state";
    if (state == qi::FutureState_FinishedWithError)
      errorMessage << ", error: " << gettingProperty.error();
    else
      errorMessage << ": " << state;
    EXPECT_EQ(qi::FutureState_FinishedWithValue, state) << errorMessage.str();
  }

  std::vector<qi::AnyObject> identities(const std::vector<qi::AnyObject>& objects)
  {
    return objects;
  }

  std::vector<qi::AnyObject> generateObjects()
  {
    return std::vector<qi::AnyObject>{boost::make_shared<ObjectToPing>()};
  }

  qi::Signal<qi::AnyObject> onTruc;
  qi::Property<qi::AnyObject> object;
  qi::Property<std::vector<qi::AnyObject>> vectorOfObjects;
};

QI_REGISTER_OBJECT(ObjectEmitter, emitObject, identity, identities, generateObjects,
                   receiveObject, onTruc, vectorOfObjects, object)


TEST(SendObject, pass_obj_made_from_module)
{
  qi::AnyModule testModule = qi::import("naoqi.testanymodule");
  auto obj = testModule.call<qi::AnyObject>("test");
  auto obj2 = obj;
  ASSERT_EQ(obj, obj2);
  ASSERT_EQ(1, obj.call<int>("testMethod", 0)); // just checking, in case of

  TestSessionPair p;
  p.server()->registerService("plop", boost::make_shared<ObjectEmitter>());

  qi::AnyObject remotePlop = p.client()->service("plop");
  qi::Promise<void> receivingObject;
  remotePlop.connect("onTruc", boost::function<void(qi::AnyObject)>([&](qi::AnyObject o)
  {
    ASSERT_EQ(o, obj);
    ASSERT_EQ(1, o.call<int>("testMethod", 0)); // this is the real test
    receivingObject.setValue(0);
  }));
  remotePlop.async<void>("emitObject", obj);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, receivingObject.future().waitFor(timeout));
}

TEST(Module, EqualityOfRemoteObjects)
{
  TestSessionPair p;
  p.server()->registerService("plop", boost::make_shared<ObjectEmitter>());

  qi::DynamicObjectBuilder builder;
  auto o = builder.object();

  qi::AnyObject remotePlop = p.client()->service("plop");
  auto remoteObject = remotePlop.call<qi::AnyObject>("identity", o);
  EXPECT_EQ(o, remoteObject);

  qi::DynamicObjectBuilder builder2;
  auto o2 = builder2.object();
  auto remoteObject2 = remotePlop.call<qi::AnyObject>("identity", o2);
  EXPECT_EQ(o2, remoteObject2);
  EXPECT_NE(o2, remoteObject);
  EXPECT_NE(remoteObject2, remoteObject);
  EXPECT_NE(remoteObject2, o);
}

class ObjectStore
{
  qi::AnyObject obj;
public:
  qi::AnyObject get() const
  {
    return obj;
  }
  void set(qi::AnyObject o)
  {
    obj = o;
  }
};

QI_REGISTER_OBJECT(ObjectStore, get, set);

struct Dummy
{
  int one() const
  {
    return 1;
  }
};

QI_REGISTER_OBJECT(Dummy, one);

TEST(Module, EqualityOfRemoteObjectsMoreIndirections)
{
  qi::AnyObject originalObject(boost::make_shared<Dummy>());
  TestSessionPair pairA;
  pairA.server()->registerService("serviceA", boost::make_shared<ObjectStore>());
  qi::AnyObject clientA = pairA.client()->service("serviceA");
  clientA.call<void>("set", originalObject);
  qi::AnyObject objA = clientA.call<qi::AnyObject>("get");
  EXPECT_EQ(originalObject, objA);

  TestSessionPair pairB;
  pairB.server()->registerService("serviceB", boost::make_shared<ObjectStore>());
  qi::AnyObject clientB = pairB.client()->service("serviceB");
  clientB.call<void>("set", objA);
  qi::AnyObject objB = clientB.call<qi::AnyObject>("get");
  EXPECT_EQ(originalObject, objB);

  TestSessionPair pairC;
  pairC.server()->registerService("serviceC", boost::make_shared<ObjectStore>());
  qi::AnyObject clientC = pairC.client()->service("serviceC");
  clientC.call<void>("set", objB);
  qi::AnyObject objC = clientC.call<qi::AnyObject>("get");
  EXPECT_EQ(originalObject, objC);
  EXPECT_EQ(objA, objC);
  EXPECT_EQ(objB, objC);
}

class ObjectEmitterFactory
{
public:
  qi::AnyObject makeObjectEmitter() { return boost::make_shared<ObjectEmitter>(); }
};

QI_REGISTER_OBJECT(ObjectEmitterFactory, makeObjectEmitter)

TEST(SendObject, pass_obj_made_from_module_to_an_obj_made_from_service)
{
  qi::AnyModule testModule = qi::import("naoqi.testanymodule");
  auto obj = testModule.call<qi::AnyObject>("test");
  ASSERT_EQ(1, obj.call<int>("testMethod", 0)); // just checking, in case of

  TestSessionPair p;
  p.server()->registerService("EmitterFactory", boost::make_shared<ObjectEmitterFactory>());

  qi::AnyObject emitterFactory = p.client()->service("EmitterFactory");
  auto emitter = emitterFactory.call<qi::AnyObject>("makeObjectEmitter");

  qi::Promise<void> receivingObject;
  emitter.connect("onTruc", boost::function<void(qi::AnyObject)>([&](qi::AnyObject o)
  {
    ASSERT_EQ(o, obj);
    int i = o.call<int>("testMethod", 0);
    ASSERT_EQ(1, i); // this is the real test
    receivingObject.setValue(0);
  }));
  emitter.async<void>("emitObject", obj);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, receivingObject.future().waitFor(timeout));
}

TEST(SendObject, emitter_from_factory_transmits_objects_through_property_then_receive_object)
{
  TestSessionPair p;
  p.server()->registerService("EmitterFactory", boost::make_shared<ObjectEmitterFactory>());

  qi::AnyObject emitterFactory = p.client()->service("EmitterFactory");
  auto emitter = emitterFactory.call<qi::AnyObject>("makeObjectEmitter");

  // create vector
  std::vector<qi::AnyObject> vecObj{
    boost::make_shared<ObjectToPing>(), boost::make_shared<ObjectToPing>()
  };

  emitter.setProperty<std::vector<qi::AnyObject>>("vectorOfObjects", vecObj);
  auto vectorOfObjectsReceived = emitter.property<std::vector<qi::AnyObject>>("vectorOfObjects").value();
  auto objectToReceive = vectorOfObjectsReceived[0];
  auto receiving = emitter.async<void>("receiveObject", objectToReceive);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, receiving.wait(100));
}

TEST(SendObject, emitter_from_factory_transmits_objects_through_property_then_ping_property)
{
  TestSessionPair p;
  p.server()->registerService("EmitterFactory", boost::make_shared<ObjectEmitterFactory>());

  qi::AnyObject emitterFactory = p.client()->service("EmitterFactory");
  auto emitter = emitterFactory.call<qi::AnyObject>("makeObjectEmitter");

  // create vector
  std::vector<qi::AnyObject> vecObj{
    boost::make_shared<ObjectToPing>(), boost::make_shared<ObjectToPing>()
  };

  emitter.setProperty<std::vector<qi::AnyObject>>("vectorOfObjects", vecObj);
  auto vectorOfObjectsReceived = emitter.property<std::vector<qi::AnyObject>>("vectorOfObjects").value();
  auto objectToPing = vectorOfObjectsReceived[0];
  ASSERT_EQ(qi::FutureState_FinishedWithValue, objectToPing.property<void>("propToPing").wait(timeoutMs));
}

TEST(SendObject, object_emitter_service_transmits_objects_through_property_then_ping_property)
{
  TestSessionPair p;
  p.server()->registerService("ObjectEmitter", boost::make_shared<ObjectEmitter>());
  qi::AnyObject emitter = p.client()->service("ObjectEmitter");

  // create vector
  std::vector<qi::AnyObject> vecObj{
    boost::make_shared<ObjectToPing>(), boost::make_shared<ObjectToPing>()
  };

  emitter.setProperty<std::vector<qi::AnyObject>>("vectorOfObjects", vecObj);
  auto vectorOfObjectsReceived = emitter.property<std::vector<qi::AnyObject>>("vectorOfObjects").value();
  auto objectToPing = vectorOfObjectsReceived[0];
  ASSERT_EQ(qi::FutureState_FinishedWithValue, objectToPing.property<void>("propToPing").wait(timeoutMs));
}

TEST(SendObject, object_emitter_service_identities_then_ping_property)
{
  TestSessionPair p;
  p.server()->registerService("ObjectEmitter", boost::make_shared<ObjectEmitter>());

  qi::AnyObject emitter = p.client()->service("ObjectEmitter");

  // create vector
  std::vector<qi::AnyObject> vecObj{
    boost::make_shared<ObjectToPing>(), boost::make_shared<ObjectToPing>()
  };

  auto sameObjects = emitter.call<std::vector<qi::AnyObject>>("identities", vecObj);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, sameObjects[0].property<void>("propToPing").wait(timeoutMs));
}

TEST(SendObject, object_emitter_service_identities_then_ping)
{
  TestSessionPair p;
  p.server()->registerService("ObjectEmitter", boost::make_shared<ObjectEmitter>());

  qi::AnyObject emitter = p.client()->service("ObjectEmitter");

  // create vector
  std::vector<qi::AnyObject> vecObj{
    boost::make_shared<ObjectToPing>(), boost::make_shared<ObjectToPing>()
  };

  auto sameObjects = emitter.call<std::vector<qi::AnyObject>>("identities", vecObj);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, sameObjects[0].async<void>("ping").wait(timeoutMs));
}

TEST(SendObject, object_emitter_service_generate_objects_then_ping)
{
  TestSessionPair p;
  p.server()->registerService("ObjectEmitter", boost::make_shared<ObjectEmitter>());
  qi::AnyObject emitter = p.client()->service("ObjectEmitter");
  auto objects = emitter.call<std::vector<qi::AnyObject>>("generateObjects");
  ASSERT_EQ(qi::FutureState_FinishedWithValue, objects[0].async<void>("ping").wait(timeoutMs));
}

TEST(SendObject, object_emitter_service_identity_then_ping)
{
  TestSessionPair p;
  p.server()->registerService("ObjectEmitter", boost::make_shared<ObjectEmitter>());

  qi::AnyObject emitter = p.client()->service("ObjectEmitter");
  qi::AnyObject object{boost::make_shared<ObjectToPing>()};

  auto sameObject = emitter.call<qi::AnyObject>("identity", object);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, sameObject.async<void>("ping").wait(timeoutMs));
}

TEST(SendObject, emitter_from_factory_transmits_single_object_through_property_then_receive_object)
{
  TestSessionPair p;
  p.server()->registerService("EmitterFactory", boost::make_shared<ObjectEmitterFactory>());

  qi::AnyObject emitterFactory = p.client()->service("EmitterFactory");
  auto emitter = emitterFactory.call<qi::AnyObject>("makeObjectEmitter");

  // create vector
  qi::AnyObject objToPing = boost::make_shared<ObjectToPing>();

  emitter.setProperty<qi::AnyObject>("object", objToPing);
  auto objectStored = emitter.property<qi::AnyObject>("object").value();
  auto receiving = emitter.async<void>("receiveObject", objectStored);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, receiving.wait(timeoutMs));
}

TEST(SendObject, object_emitter_service_transmits_single_object_through_property_then_ping_property)
{
  TestSessionPair p;
  p.server()->registerService("ObjectEmitter", boost::make_shared<ObjectEmitter>());

  qi::AnyObject emitter = p.client()->service("ObjectEmitter");
  qi::AnyObject objToPing = boost::make_shared<ObjectToPing>();

  emitter.setProperty<qi::AnyObject>("object", objToPing);
  auto objectStored = emitter.property<qi::AnyObject>("object").value();
  ASSERT_EQ(qi::FutureState_FinishedWithValue, objectStored.property<void>("propToPing").wait(timeoutMs));
}

TEST(SendObject, object_emitter_service_transmits_single_object_through_property_then_ping)
{
  TestSessionPair p;
  p.server()->registerService("ObjectEmitter", boost::make_shared<ObjectEmitter>());
  qi::AnyObject emitter = p.client()->service("ObjectEmitter");

  qi::AnyObject objToPing = boost::make_shared<ObjectToPing>();
  emitter.setProperty<qi::AnyObject>("object", objToPing);
  auto objectStored = emitter.property<qi::AnyObject>("object").value();
  ASSERT_EQ(qi::FutureState_FinishedWithValue, objectStored.async<void>("ping").wait(timeoutMs));
}

TEST(SendObject, object_emitter_service_provides_single_object_through_property_then_ping)
{
  TestSessionPair p;
  p.server()->registerService("ObjectEmitter", boost::make_shared<ObjectEmitter>());
  qi::AnyObject emitter = p.client()->service("ObjectEmitter");
  auto objectStored = emitter.property<qi::AnyObject>("object").value();
  ASSERT_EQ(qi::FutureState_FinishedWithValue, objectStored.async<void>("ping").wait(timeoutMs));
}

class Cookie
{
public:
  Cookie(bool withTaste) : taste(withTaste) {}
  bool eat()
  {
    eaten();
    return true;
  }
  qi::Property<bool> taste;
  qi::Signal<void> eaten;
};

class CookieBox
{
public:
  qi::AnyObject makeCookie(bool withTaste)
  {
    return boost::shared_ptr<Cookie>{
      new Cookie{withTaste},
      [=](Cookie* cookie) {
        qiLogInfo() << "Cookie destruction";
        delete cookie;
        QI_EMIT cookieLost();
      }
    };
  }

  void give(qi::AnyObject c)
  {
    cookie = c;
  }

  qi::AnyObject take()
  {
    return cookie;
  }

  qi::Signal<void> cookieLost;

private:
  qi::AnyObject cookie;
};

struct CookieMonster
{
  void feed(qi::AnyObject cookie)
  {
    cookie.call<bool>("eat");
    qiLogInfo() << "Cookiiiiie!!! Om nom nom nom...";
  }
};

struct CookieMonsterFeeder
{
  void feedMonster(qi::AnyObject cookie, qi::AnyObject cookieMonster)
  {
    cookieMonster.call<void>("feed", cookie);
  }
};

QI_REGISTER_OBJECT(Cookie, eat, taste, eaten)
QI_REGISTER_OBJECT(CookieBox, makeCookie, give, take)
QI_REGISTER_OBJECT(CookieMonster, feed)
QI_REGISTER_OBJECT(CookieMonsterFeeder, feedMonster)

TEST(SendObject, give_and_take_object_function)
{
  TestSessionPair p;
  p.server()->registerService("CookieBox", boost::make_shared<CookieBox>());
  qi::AnyObject cookieBoxProxy = p.client()->service("CookieBox");
  qi::AnyObject cookie = cookieBoxProxy.call<qi::AnyObject>("makeCookie", true);
  cookieBoxProxy.call<void>("give", cookie);
  qi::AnyObject takenCookie = cookieBoxProxy.call<qi::AnyObject>("take");
  EXPECT_TRUE(takenCookie.call<bool>("eat"));
  EXPECT_EQ(cookie, takenCookie);
}

TEST(SendObject, give_and_take_object_property)
{
  TestSessionPair p;
  p.server()->registerService("CookieBox", boost::make_shared<CookieBox>());
  qi::AnyObject cookieBoxProxy = p.client()->service("CookieBox");
  qi::AnyObject cookie = cookieBoxProxy.call<qi::AnyObject>("makeCookie", true);
  cookieBoxProxy.call<void>("give", cookie);
  qi::AnyObject takenCookie = cookieBoxProxy.call<qi::AnyObject>("take");
  EXPECT_TRUE(takenCookie.property<bool>("taste").value(timeoutMs));
  EXPECT_EQ(cookie, takenCookie);
}

TEST(SendObject, give_and_take_object_signal)
{
  TestSessionPair p;
  p.server()->registerService("CookieBox", boost::make_shared<CookieBox>());
  qi::AnyObject cookieBoxProxy = p.client()->service("CookieBox");
  qi::AnyObject cookie = cookieBoxProxy.call<qi::AnyObject>("makeCookie", true);
  cookieBoxProxy.call<void>("give", cookie);

  qi::AnyObject takenCookie = cookieBoxProxy.call<qi::AnyObject>("take");
  EXPECT_EQ(cookie, takenCookie);
  qi::Promise<bool> eaten;
  auto connecting =
      takenCookie.connect("eaten", boost::function<void()>([&eaten]
  {
    eaten.setValue(true);
  })).async();
  EXPECT_EQ(qi::FutureState_FinishedWithValue, connecting.waitFor(timeout));

  auto eating = takenCookie.async<bool>("eat");
  EXPECT_TRUE(eating.value(timeoutMs));
  EXPECT_TRUE(eaten.future().value(timeoutMs));
}

TEST(SendObject, two_client_objects_call_each_other_on_service_side)
{
  TestSessionPair p;
  auto cookieFeeder = boost::make_shared<CookieMonsterFeeder>();
  p.server()->registerService("CookieMonsterFeeder", cookieFeeder);

  qi::AnyObject cookieTransmitterRemote = p.client()->service("CookieMonsterFeeder");
  auto transmitting = cookieTransmitterRemote.async<void>(
        "feedMonster", boost::make_shared<Cookie>(false), boost::make_shared<CookieMonster>());
  ASSERT_EQ(qi::FutureState_FinishedWithValue, transmitting.wait(timeoutMs));
}

TEST(SendObject, object_referenced_by_remote_only_is_destroyed_on_disconnection)
{
  TestSessionPair p;
  auto cookieBox = boost::make_shared<CookieBox>();
  p.server()->registerService("CookieBox", cookieBox);
  qi::AnyObject cookieBoxRemote = p.client()->service("CookieBox");

  qiLogInfo() << "Getting a cookie!";
  auto cookie = cookieBoxRemote.call<qi::AnyObject>("makeCookie", true);
  QI_UNUSED(cookie);
  qi::SignalSpy cookieLostSpy{cookieBox->cookieLost};

  qiLogInfo() << "Closing the session.";
  p.client()->close();

  // in this case, the references are only counted using the shared pointers
  if (p.mode() == TestMode::Mode_Direct)
    ASSERT_FALSE(cookieLostSpy.waitUntil(1, timeout));

  // In every other case, a remote object intervenes
  else
    ASSERT_TRUE(cookieLostSpy.waitUntil(1, timeout));
}

TEST(SendObject, object_referenced_by_remote_only_is_destroyed_on_unreference)
{
  TestSessionPair p;
  auto cookieBox = boost::make_shared<CookieBox>();
  p.server()->registerService("CookieBox", cookieBox);
  qi::AnyObject cookieBoxRemote = p.client()->service("CookieBox");
  auto cookie = cookieBoxRemote.call<qi::AnyObject>("makeCookie", true);
  qi::SignalSpy cookieLostSpy{cookieBox->cookieLost};
  cookie.reset();
  ASSERT_TRUE(cookieLostSpy.waitUntil(1, timeout));
}
