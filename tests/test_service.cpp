/*
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <vector>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <qimessaging/session.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/dynamicobject.hpp>
#include <qitype/objecttypebuilder.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/gateway.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <testsession/testsessionpair.hpp>

qiLogCategory("test");

static std::string reply(const std::string &msg)
{
  return msg;
}

/* For asynchronous things where no synchronisation mechanism
 * is possible, loop the check and wait a small delay,
 * instead of one big sleep that will slow us down
 *
 */
#define PERSIST_CHECK(code, cond, what, msdelay)  \
do                                           \
{                                            \
  code;                                      \
  for(unsigned i=0; i<50 && !(cond); ++i)    \
  {                                          \
    qi::os::msleep(1 + msdelay / 50);        \
    code;                                    \
  }                                          \
  what(cond);                                \
} while(0)

#define PERSIST_ASSERT(code, cond, msdelay)  \
 PERSIST_CHECK(code, cond, ASSERT_TRUE, msdelay)
#define PERSIST_EXPECT(code, cond, msdelay)  \
 PERSIST_CHECK(code, cond, EXPECT_TRUE, msdelay)
#define PERSIST(code, cond, msdelay)  \
 PERSIST_CHECK(code, cond, ,msdelay)
//check for server closed
//check for socket disconnected
//check for service unregistered
//check for service unregistered, then readded

TEST(QiService, RemoteObjectCacheServerClose)
{
  TestSessionPair p;
  if (p.server() == p.client()) // we close and not unregister, so does not work in direct mode
    return;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  p.server()->registerService("serviceTest", obj);
  qi::Future<qi::AnyObject> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value()->call<std::string>("reply", "titi").value());

  p.server()->close();

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), 1000);
}


TEST(QiService, RemoteObjectCacheUnregister)
{
  TestSessionPair p;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  unsigned int idx = p.server()->registerService("serviceTest", obj);
  qi::Future<qi::AnyObject> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value()->call<std::string>("reply", "titi").value());

  p.server()->unregisterService(idx);

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), 1000);

}


TEST(QiService, RemoteObjectCacheABAUnregister)
{
  TestSessionPair p;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  unsigned int idx = p.server()->registerService("serviceTest", obj);
  qi::Future<qi::AnyObject> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value()->call<std::string>("reply", "titi").value());

  p.server()->unregisterService(idx);

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), 1000);

  unsigned int idx2 = p.server()->registerService("serviceTest", obj);
  //new service should not have a previoulsy registered ID
  EXPECT_NE(idx2, idx);

  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  qi::Future<std::string> fret = fut.value()->call<std::string>("reply", "titi");
  if (fret.hasError()) {
    std::cout << "Error returned:" << fret.error();
  }
  EXPECT_FALSE(fret.hasError());
  EXPECT_EQ(std::string("titi"), fret.value());
}


TEST(QiService, RemoteObjectCacheABANewServer)
{
  TestSessionPair p;
  qi::Session     ses;
  if (p.server() == p.client()) // we close and not unregister, so does not work in direct mode
    return;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  unsigned int idx = p.server()->registerService("serviceTest", obj);
  qi::Future<qi::AnyObject> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value()->call<std::string>("reply", "titi").value());

  p.server()->close();

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), 1000);

  qi::Future<void> f = ses.connect(p.client()->url().str());
  f.wait(8000);
  EXPECT_FALSE(f.hasError());
  ses.listen("tcp://0.0.0.0:0");
  unsigned int idx2 = ses.registerService("serviceTest", obj);
  //new service should not have a previoulsy registered ID
  EXPECT_NE(idx2, idx);

  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  qi::Future<std::string> fret = fut.value()->call<std::string>("reply", "titi");
  if (fret.hasError()) {
    std::cout << "Error returned:" << fret.error();
  }
  EXPECT_FALSE(fret.hasError());
  EXPECT_EQ(std::string("titi"), fret.value());
}


TEST(QiService, RemoteObjectNackTransactionWhenServerClosed)
{
  TestSessionPair p;
  if (p.server() == p.client()) // we close and not unregister, so does not work in direct mode
    return;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("msleep", &qi::os::msleep);
  qi::AnyObject obj(ob.object());

  p.server()->registerService("serviceTest", obj);
  qi::Future<qi::AnyObject> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  qi::Future<void> fret = fut.value()->call<void>("msleep", 2000);
  qi::Future<void> fclose = p.server()->close();
  fclose.wait(1000);
  EXPECT_TRUE(fclose.isFinished());
  EXPECT_FALSE(fclose.hasError(1));

  fret.wait(1000);
  //once the server is close, the answer should be ready.
  EXPECT_TRUE(fret.isFinished());
  //the service is closed, so it can't send an answer.
  EXPECT_TRUE(fret.hasError(1000));
}


class Foo
{
public:
  int ping(int i) { return i + prop.get();}
  qi::Property<int> prop;
};

void inc (int* daInt, int unused)
{
  ++(*daInt);
}

TEST(QiService, ClassProperty)
{
  TestSessionPair p;

  qi::ObjectTypeBuilder<Foo> builder;
  builder.advertiseMethod("ping", &Foo::ping);
  ASSERT_TRUE(builder.advertiseProperty("offset", &Foo::prop) > 0);

  Foo f;
  qi::AnyObject obj = builder.object(&f);

  p.server()->registerService("foo", obj);

  qi::AnyObject client = p.client()->service("foo");
  qi::details::printMetaObject(std::cerr, obj->metaObject());
  std::cerr <<"--" << std::endl;
  qi::details::printMetaObject(std::cerr, client->metaObject());
  qiLogDebug() << "setProp";
  client->setProperty<int>("offset", 1).value();
  qiLogDebug() << "setProp done";
  ASSERT_EQ(1, f.prop.get());
  ASSERT_EQ(2, client->call<int>("ping", 1));
  f.prop.set(2);
  ASSERT_EQ(3, client->call<int>("ping", 1));
  ASSERT_EQ(2, client->property<int>("offset"));

  // test event
  int hit = 0;
  f.prop.connect(boost::bind(&inc, &hit, _1));
  obj->connect("offset", boost::bind(&inc, &hit,_1));
  client->connect("offset", boost::bind(&inc, &hit,_1));
  f.prop.set(1);
  PERSIST_ASSERT(, hit == 3, 500);
  client->setProperty("offset", 2);
  PERSIST_ASSERT(, hit == 6, 500);

  // test error handling
   EXPECT_TRUE(client->setProperty("canard", 5).hasError());
   EXPECT_TRUE(client->setProperty("offset", "astring").hasError());
}

int prop_ping(qi::PropertyBase* &p, int v)
{
  return p->value().toInt() + v;
}

TEST(QiService, GenericProperty)
{
  TestSessionPair p;
  qi::DynamicObject* dobj = new qi::DynamicObject();
  qi::DynamicObjectBuilder builder(dobj);
  unsigned int propId = builder.advertiseProperty<int>("offset");
  qi::PropertyBase* prop;
  builder.advertiseMethod("ping",
    (boost::function<int (int)>)boost::bind(&prop_ping, boost::ref(prop), _1));
  qi::AnyObject obj = builder.object();
  prop = dobj->property(propId);
  prop->setValue(qi::AnyValue(qi::AnyReference(0)));
  p.server()->registerService("foo", obj);

  qi::AnyObject client = p.client()->service("foo");

  client->setProperty("offset", 1);
  ASSERT_EQ(1, prop->value().toInt());
  ASSERT_EQ(2, client->call<int>("ping", 1));
  prop->setValue(qi::AnyValue(qi::AnyReference(2)));
  ASSERT_EQ(3, client->call<int>("ping", 1));
  ASSERT_EQ(2, client->property<int>("offset"));

  // test event
  int hit = 0;
  qiLogVerbose() << "Connecting to signal";
  ASSERT_NE(qi::SignalBase::invalidLink, prop->signal()->connect(boost::bind(&inc, &hit, _1)));
  ASSERT_NE(qi::SignalBase::invalidLink, obj->connect("offset", boost::bind(&inc, &hit, _1)));
  ASSERT_NE(qi::SignalBase::invalidLink, client->connect("offset", boost::bind(&inc, &hit, _1)));
  qiLogVerbose() << "Triggering prop set";
  prop->setValue(qi::AnyValue(qi::AnyReference(1)));
  PERSIST(, hit == 3, 500);
  qi::os::msleep(500);
  EXPECT_EQ(3, hit);
  client->setProperty<int>("offset", 2);
  PERSIST(, hit == 6, 500);
  qi::os::msleep(500);
  EXPECT_EQ(6, hit);
  if (client != obj)
  {
    client->call<void>("setProperty", "offset", 3);
    EXPECT_EQ(3, prop->value().toInt());
  }

  // test error handling
  EXPECT_TRUE(client->setProperty("canard", 5).hasError());
  EXPECT_TRUE(client->setProperty("offset", "astring").hasError());
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
