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
#include <qitype/genericobject.hpp>
#include <qitype/objecttypebuilder.hpp>
#include <qitype/proxysignal.hpp>
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
  qi::Signal<void(int, int)> sig1;
  qi::Signal<void()> sig2;
  int _count1, _count2;
  qi::SignalBase::Link _l1, _l2;
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

int registerFoo()
{
  qi::ObjectTypeBuilder<Foo> b;
  b.advertiseMethod("subscribe1", &Foo::subscribe1);
  b.advertiseMethod("subscribe2", &Foo::subscribe2);
  b.advertiseMethod("unsubscribe1", &Foo::unsubscribe1);
  b.advertiseMethod("unsubscribe2", &Foo::unsubscribe2);
  b.advertiseMethod("count1", &Foo::count1);
  b.advertiseMethod("count2", &Foo::count2);
  b.advertiseEvent("sig1", &Foo::sig1);
  b.advertiseEvent("sig2", &Foo::sig2);
  b.registerType();
  return 0;
}
static int useless = registerFoo();

TEST(Proxy, Signal)
{
  boost::shared_ptr<Foo> foo(new Foo);
  qi::ObjectPtr gfoo = qi::GenericValueRef(foo).toObject();
  ASSERT_TRUE(!!gfoo);
  // The session must die before foo.
  TestSessionPair p;
  p.server()->registerService("foo", gfoo);
  qi::ObjectPtr client = p.client()->service("foo");
  ASSERT_EQ(0, client->call<int>("count1"));
  qi::ProxySignal<void(int, int)> proxy1(client, "sig1");
  foo->subscribe1();
  proxy1(1, 2);
  PERSIST_ASSERT(, foo->count1() == 3, 500);
  // small hack, reuse foo function to test callback on proxy signal
  Foo foo2;
  qi::SignalBase::Link l =  proxy1.connect(boost::bind(&Foo::on1, &foo2, _1, _2));
  proxy1(3, 4);
  PERSIST_ASSERT(, foo->count1() == 10, 500);
  PERSIST_ASSERT(, foo2.count1() == 7, 500);
  proxy1.disconnect(l);
  proxy1(1, 1);
  PERSIST_ASSERT(, foo->count1() == 12, 500);
  PERSIST_ASSERT(, foo2.count1() == 7, 500);
}


int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
