/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/


#include <list>
#include <iostream>

#include <gtest/gtest.h>

#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qi/eventloop.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/objecttypebuilder.hpp>
#include <qitype/proxysignal.hpp>
#include <qitype/proxyproperty.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <testsession/testsessionpair.hpp>

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
 
qiLogCategory("test");

class Foo
{
public:
  Foo() : _count1(0), _count2(0) {}
  void subscribe1();
  void subscribe2();
  void unsubscribe1();
  void unsubscribe2();
  int count1() { return _count1;}
  int count2() { return _count2;}
  void on1(int a, int b) { _count1 += a+b;}
  void on2() { ++_count2;}
  qi::Signal<int, int> sig1;
  qi::Signal<> sig2;

  int _count1, _count2;
  qi::SignalLink _l1, _l2;
};

void Foo::subscribe1()
{
  _l1 = sig1.connect(boost::bind(&Foo::on1, this, _1, _2));
}
void Foo::unsubscribe1()
{
  sig1.disconnect(_l1);
}
void Foo::subscribe2()
{
  _l2 = sig2.connect(boost::bind(&Foo::on2, this));
}
void Foo::unsubscribe2()
{
  sig2.disconnect(_l2);
}


QI_REGISTER_OBJECT(Foo, subscribe1, subscribe2, unsubscribe1, unsubscribe2,
  count1, count2, sig1, sig2);

TEST(Proxy, Signal)
{
  boost::shared_ptr<Foo> foo(new Foo);
  qi::AnyObject gfoo = qi::AnyReference(foo).toObject();
  ASSERT_TRUE(!!gfoo);
  qi::details::printMetaObject(std::cerr, gfoo->metaObject());
  // The session must die before foo.
  TestSessionPair p;
  p.server()->registerService("foo", gfoo);
  qi::AnyObject client = p.client()->service("foo");
  ASSERT_EQ(0, client->call<int>("count1"));
  qi::ProxySignal<void(int, int)> proxy1(client, "sig1");
  foo->subscribe1();
  proxy1(1, 2);
  PERSIST_ASSERT(, foo->count1() == 3, 500);
  // small hack, reuse foo function to test callback on proxy signal
  Foo foo2;
  qi::SignalLink l =  proxy1.connect(boost::bind(&Foo::on1, &foo2, _1, _2));
  proxy1(3, 4);
  PERSIST_ASSERT(, foo->count1() == 10, 500);
  PERSIST_ASSERT(, foo2.count1() == 7, 500);
  proxy1.disconnect(l);
  proxy1(1, 1);
  PERSIST_ASSERT(, foo->count1() == 12, 500);
  PERSIST_ASSERT(, foo2.count1() == 7, 500);
}

class Bar
{
public:
  Bar() :_count(0){}
  void set(int v) {prop.set(v);}
  int get() { return prop.get();}
  int count() { return _count;}
  void onProp(int v)
  {
    qiLogDebug() << "onprop " << v <<" " << this;
    _count += v;
  }
  void subscribe()
  {
    _link = prop.connect(boost::bind(&Bar::onProp, this, _1));
  }
  void unsubscribe()
  {
    prop.disconnect(_link);
  }
  qi::Property<int> prop;
  qi::SignalLink _link;
  int _count;
};

QI_REGISTER_OBJECT(Bar, subscribe, unsubscribe, count, get, set, prop);

TEST(Proxy, Property)
{
  boost::shared_ptr<Bar> bar(new Bar);
  qi::AnyObject gbar = qi::AnyReference(bar).toObject();
  ASSERT_TRUE(!!gbar);
  // The session must die before bar.
  TestSessionPair p;
  p.server()->registerService("bar", gbar);
  // we need that to force two clients
  p.server()->registerService("bar2", gbar);
  qi::AnyObject client = p.client()->service("bar");
  ASSERT_EQ(0, client->call<int>("count"));

  qi::ProxyProperty<int> pp(client, "prop");
  bar->set(1);
  ASSERT_EQ(1, pp.get());
  pp.set(2);
  ASSERT_EQ(2, bar->get());
  // althoug PropertyProxy::set is itself synchronous, notify on remote end
  // may be asynchronous, so subscribe below may come too soon and catch
  // the pp.set above
  qi::os::msleep(100);
  qiLogDebug() << "subscribe";
  bar->subscribe();
  qi::os::msleep(100);
  qiLogDebug() << "set 3";
  pp.set(3);
  // this is an event, all notify are asychronous
  PERSIST_ASSERT(, bar->count() == 3, 500);

  Bar bar2;
  qi::SignalLink l = pp.connect(boost::bind(&Bar::onProp, &bar2, _1));
  bar->set(4);
  // this one is async (remote notify of local property set)
  PERSIST_ASSERT(, bar2.count() == 4, 500);
  pp.disconnect(l);
  bar->set(5); // we expect an async op *not* to happen, no choice but wait.
  qi::os::msleep(200);
  ASSERT_EQ(4, bar2.count());
  // reconnect to see if disconnect did not break anything
  l = pp.connect(boost::bind(&Bar::onProp, &bar2, _1));
  bar->set(4);
  PERSIST_ASSERT(, bar2.count() == 8, 500);

  // proxy-proxy
  qi::AnyObject client2 = p.client()->service("bar2");
  qi::ProxyProperty<int> pp2(client2, "prop");
  Bar bar3;
  pp2.connect(boost::bind(&Bar::onProp, &bar3, _1));
  qiLogDebug() << "set 2";
  pp.set(2);
  PERSIST(, bar3.count() == 2, 1000);
  ASSERT_EQ(2, bar3.count());
  PERSIST(, bar2.count() == 10, 500);
  ASSERT_EQ(10, bar2.count());
  qiLogDebug() << "set 3";
  pp2.set(3);
  PERSIST_ASSERT(, bar2.count() == 13, 500);
  PERSIST_ASSERT(, bar3.count() == 5, 500);
}



int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
