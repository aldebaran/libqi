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

int timeoutMs = 500;
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

class ObjectEmitter
{
public:
  void emitObject(qi::AnyObject o) { QI_EMIT onTruc(o); }
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

  qi::Signal<qi::AnyObject> onTruc;
  qi::Property<std::vector<qi::AnyObject>> vectorOfObjects;
};

QI_REGISTER_OBJECT(ObjectEmitter, emitObject, receiveObject, onTruc, vectorOfObjects)

class SubObjectToPing
{
public:
  void subping() { qiLogInfo() << "subping !" << std::endl; }
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

TEST(SendObject, pass_obj_made_from_module)
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
    int i = o.call<int>("testMethod", 0);
    ASSERT_EQ(1, i); // this is the real test
    receivingObject.setValue(0);
  }));
  emitter.async<void>("emitObject", obj);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, receivingObject.future().waitFor(qi::MilliSeconds(1000)));
}

TEST(SendObject, pass_obj_made_from_service_to_another_function_of_service)
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
  EXPECT_EQ(qi::FutureState_FinishedWithValue, receiving.waitFor(timeout*2));
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

QI_REGISTER_OBJECT(Cookie, eat, taste, eaten)
QI_REGISTER_OBJECT(CookieBox, makeCookie, give, take)

TEST(SendObject, give_and_take_object_function)
{
  TestSessionPair p;
  p.server()->registerService("CookieBox", boost::make_shared<CookieBox>());
  qi::AnyObject cookieBoxProxy = p.client()->service("CookieBox");
  qi::AnyObject cookie = cookieBoxProxy.call<qi::AnyObject>("makeCookie", true);
  cookieBoxProxy.call<void>("give", cookie);
  qi::AnyObject tookCookie = cookieBoxProxy.call<qi::AnyObject>("take");
  EXPECT_TRUE(tookCookie.call<bool>("eat"));
}

TEST(SendObject, give_and_take_object_property)
{
  TestSessionPair p;
  p.server()->registerService("CookieBox", boost::make_shared<CookieBox>());
  qi::AnyObject cookieBoxProxy = p.client()->service("CookieBox");
  qi::AnyObject cookie = cookieBoxProxy.call<qi::AnyObject>("makeCookie", true);
  cookieBoxProxy.call<void>("give", cookie);
  qi::AnyObject tookCookie = cookieBoxProxy.call<qi::AnyObject>("take");
  EXPECT_TRUE(tookCookie.property<bool>("taste").value(500));
}

TEST(SendObject, give_and_take_object_signal)
{
  TestSessionPair p;
  p.server()->registerService("CookieBox", boost::make_shared<CookieBox>());
  qi::AnyObject cookieBoxProxy = p.client()->service("CookieBox");
  qi::AnyObject cookie = cookieBoxProxy.call<qi::AnyObject>("makeCookie", true);
  cookieBoxProxy.call<void>("give", cookie);

  qi::AnyObject takenCookie = cookieBoxProxy.call<qi::AnyObject>("take");
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
  ASSERT_TRUE(cookieLostSpy.waitUntil(1, qi::MilliSeconds{100}));
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
  ASSERT_TRUE(cookieLostSpy.waitUntil(1, qi::MilliSeconds{100}));
}
