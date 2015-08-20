/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/signal.hpp>
#include <qi/future.hpp>
#include <qi/signalspy.hpp>
#include <qi/anyobject.hpp>
#include <qi/application.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>

qiLogCategory("test");

class Foo
{
public:
  void func(int)               { }
  void func1(qi::Atomic<int>* r, int)
  {
    // hackish sleep so that asynchronous trigger detection is safer
    qi::os::msleep(1); ++*r;
  }
  void func2(qi::Atomic<int>* r, int, int) { ++*r; }
};
void foo(qi::Atomic<int>* r, int, int)     { ++*r; }
void foo2(qi::Atomic<int>* r, char, char)  { ++*r; }
void foo3(qi::Atomic<int>* r, Foo *)       { ++*r; }
void foolast(int, qi::Promise<void> prom, qi::Atomic<int>* r) { prom.setValue(0); ++*r; }

TEST(TestSignal, TestCompilation)
{
  qi::Atomic<int>        res = 0;
  qi::Signal<int> s;
  Foo*                   f = (Foo*)1;
  qi::Promise<void>      prom;

  //do not count
  s.connect(qi::AnyFunction::from(&Foo::func, f));
  s.connect(boost::bind<void>(&Foo::func, f, _1));

  s.connect(boost::bind(&foo, &res, 12, _1));
  s.connect(boost::bind(&foo2, &res, 12, _1));
  s.connect(boost::bind<void>(&Foo::func1, f, &res, _1));
  s.connect(boost::bind<void>(&Foo::func2, f, &res, 5, _1));
  s.connect(boost::bind(&foo3, &res, f));
  s.connect(boost::bind(&foolast, _1, prom, &res));

  s(42);

  while (*res != 6)
    qi::os::msleep(10);

  ASSERT_EQ(6, *res);
  ASSERT_TRUE(prom.future().isFinished());
  ASSERT_FALSE(prom.future().hasError());
}

void write42(boost::shared_ptr<int> ptr)
{
  *ptr = 42;
}

TEST(TestSignal, SharedPtr)
{
  // Redundant with Copy test, but just to be sure, check that shared_ptr
  // is correctly transmited.
  qi::Signal<boost::shared_ptr<int> > sig;
  sig.connect(qi::AnyFunction::from(&write42)).setCallType(qi::MetaCallType_Queued);
  {
    boost::shared_ptr<int> ptr(new int(12));
    sig(ptr);
  };
}

void byRef(int& i, bool* done)
{
  qiLogDebug() <<"byRef " << &i << ' ' << done;
  i = 12;
  *done = true;
}


TEST(TestSignal, AutoDisconnect)
{
  // Test automatic disconnection when passing shared_ptrs
  qi::Atomic<int> r = 0;
  boost::shared_ptr<Foo> foo(new Foo());
  qi::Signal<qi::Atomic<int>*, int> sig;
  sig.connect(&Foo::func1, boost::weak_ptr<Foo>(foo), _1, _2).setCallType(qi::MetaCallType_Direct);
  sig(&r, 0);
  ASSERT_EQ(1, *r);
  foo.reset();
  sig(&r, 0);
  ASSERT_EQ(1, *r);
}

void waitFuture(qi::Atomic<int>& cnt, qi::Promise<void> start, qi::Future<void> f)
{
  if (++cnt == 2)
    start.setValue(0);
  f.wait();

  // force disconnection, may trigger a segfault
  throw qi::PointerLockException();
}

TEST(TestSignal, NonBlockingDestroy)
{
  // disconnect is blocking, but signal destruction should not be

  qi::Promise<void> start;
  qi::Promise<void> finish;
  qi::Atomic<int> cnt = 0;

  {
    qi::Signal<void> sig;
    sig.connect(boost::bind(waitFuture, boost::ref(cnt), start, finish.future()));

    sig();
    sig();

    start.future().wait();
    // now that the callback is running, destroy the signal
  }

  // all went well, unblock the callback
  finish.setValue(0);
}

TEST(TestSignal, BadArity)
{
  // Test runtime detection of arity errors
  qi::Signal<> s;
  // avoid using s.connect() which will catch the problem at compile-time
  EXPECT_ANY_THROW(s.connect(qi::SignalSubscriber(qi::AnyFunction::from(&foo))));
  EXPECT_ANY_THROW(s.connect(qi::SignalSubscriber(qi::AnyFunction::from(&foo2))));
  EXPECT_ANY_THROW(s.connect(qi::AnyFunction::from((boost::function<void(qi::Atomic<int>*, int)>)boost::bind(&Foo::func1, (Foo*)0, _1, _2))));
}

void lol(int v, int& target)
{
  target = v;
}
TEST(TestSignal, SignalSignal)
{
  qi::SignalF<void (int)> sig1;
  qi::SignalF<void (int)> *sig2 = new  qi::SignalF<void (int)>();
  int res = 0;
  sig1.connect(*sig2);
  sig2->connect(boost::bind<void>(&lol, _1, boost::ref(res)));
  sig1(10);
  qi::os::msleep(300);
  ASSERT_EQ(10, res);
  // Test autodisconnect
  delete sig2;
  sig1(20);
  qi::os::msleep(300);
  ASSERT_EQ(10, res);
}

class SignalTest {
public:
  SignalTest() : payload(0) {}
  qi::Signal<int> sig;
  qi::Signal<int> sig2;
  void callback(int val) {
    payload = val;
  }
  int payload;
};


TEST(TestSignal, SignalSignal2)
{
  SignalTest st;
  st.sig2.connect(&SignalTest::callback, boost::ref(st), _1);
  st.sig.connect(st.sig2);
  qiLogDebug() << "sigptrs are " << &st.sig << " " << &st.sig2;
  st.sig(4242);
  for (unsigned i=0; i<50 && st.payload != 4242; ++i)
    qi::os::msleep(20);
  EXPECT_EQ(st.payload, 4242);
}

TEST(TestSignal, SignalN)
{
  qi::Signal<int> sig;
  int res = 0;
  sig.connect(boost::bind<void>(&lol, _1, boost::ref(res)));
  sig(5);
  qi::os::msleep(300);
  ASSERT_EQ(5, res);
}

class SigHolder
{
public:
  qi::Signal<> s0;
  qi::Signal<int> s1;
  qi::Signal<int, int> s2;
  void fire0() { s0();}
  void fire1(int i) { s1(i); }
  void fire2(int i, int j) { s2(i, j);}
};

QI_REGISTER_OBJECT(SigHolder, s0, s1, s2, fire0, fire1, fire2)

TEST(TestSignal, SignalNBind)
{
  int res = 0;
  boost::shared_ptr<SigHolder> so(new SigHolder);
  qi::AnyObject op = qi::AnyReference::from(so).to<qi::AnyObject>();
  qi::detail::printMetaObject(std::cerr, op.metaObject());
  op.connect("s1", (boost::function<void(int)>)boost::bind<void>(&lol, _1, boost::ref(res)));
  op.post("s1", 2);
  for (unsigned i=0; i<30 && res!=2; ++i) qi::os::msleep(10);
  ASSERT_EQ(2, res);
  so->s1(3);
  for (unsigned i=0; i<30 && res!=3; ++i) qi::os::msleep(10);
  ASSERT_EQ(3, res);
  so->s0.connect( boost::bind<void>(&lol, 42, boost::ref(res)));
  op.post("s0");
  for (unsigned i=0; i<30 && res!=42; ++i) qi::os::msleep(10);
  ASSERT_EQ(42, res);
}


void foothrow() {
  throw 42;
}

TEST(TestSignal, SignalThrow)
{
  qi::Signal<> sig;
  sig.connect(boost::bind<void>(&foothrow));
  ASSERT_NO_THROW(sig());
  qi::os::msleep(50);
}


void dynTest0(int& tgt)                     { tgt |= 1;}
void dynTest1(int& tgt, int)                { tgt |= 2;}
void dynTest2(int& tgt, int, int)           { tgt |= 4;}
void dynTest3(int& tgt, int, std::string)   { tgt |= 8;}
void dynTestN(int& tgt, const qi::AnyArguments& a)   { tgt |= 32;}
qi::AnyReference dynTestN2(int& tgt, const std::vector<qi::AnyReference>& a)   { tgt |= 16; return qi::AnyReference();}


TEST(TestSignal, Dynamic)
{
  qi::SignalBase s(qi::Signature("m"));
  int trig = 0;
  s.setCallType(qi::MetaCallType_Direct);
  s.connect((boost::function<void()>) boost::bind(&dynTest0, boost::ref(trig)));
  s.connect((boost::function<void(int)>) boost::bind(&dynTest1, boost::ref(trig), _1));
  s.connect((boost::function<void(int, int)>) boost::bind(&dynTest2, boost::ref(trig), _1, _2));
  s.connect((boost::function<void(int, std::string)>) boost::bind(&dynTest3, boost::ref(trig), _1, _2));
  s.connect(qi::AnyFunction::fromDynamicFunction(boost::bind(&dynTestN2, boost::ref(trig), _1)));
  s.connect((boost::function<void(const qi::AnyArguments&)>)(boost::bind(&dynTestN, boost::ref(trig), _1)));
  int a, b;
  std::string c;
  qi::GenericFunctionParameters params;
  s.trigger(params);
  EXPECT_EQ(49, trig);
  params.push_back(qi::AnyReference::from(a));
  trig = 0;
  s.trigger(params);
  EXPECT_EQ(50, trig);
  params.push_back(qi::AnyReference::from(b));
  trig = 0;
  s.trigger(params);
  EXPECT_EQ(52, trig);
  params[1] = qi::AnyReference::from(c);
  trig = 0;
  s.trigger(params);
  EXPECT_EQ(56, trig);
}

void onSubs(boost::atomic<bool>& var, bool subs)
{
  var = subs;
}

void callback(int i)
{}

TEST(TestSignal, OnSubscriber)
{
  boost::atomic<bool> subscribers(false);

  qi::Signal<int> sig(boost::bind(onSubs, boost::ref(subscribers), _1));
  ASSERT_FALSE(subscribers);
  qi::SignalLink l = sig.connect(&callback);
  ASSERT_TRUE(subscribers);
  ASSERT_TRUE(sig.disconnect(l));
  ASSERT_FALSE(subscribers);
}

void store2(qi::Promise<int> variable1, qi::Promise<int> variable2, int value1, int value2)
{
  variable1.setValue(value1);
  variable2.setValue(value2);
}

TEST(TestSignal, SignalToSignalWithExtraArgument)
{
  qi::Promise<int> target1, target2;
  qi::Signal<int> signal1;
  qi::Signal<int, int> signal2;
  signal1.connect(boost::bind(boost::ref(signal2), _1, 42));
  signal2.connect(store2, target1, target2, _1, _2);
  signal1(12);
  EXPECT_EQ(12, target1.future().value());
  EXPECT_EQ(42, target2.future().value());
}

TEST(TestSignalSpy, Counter)
{
  qi::Signal<int> sig;
  qi::SignalSpy sp(sig);
  QI_EMIT sig(1);
  QI_EMIT sig(1);
  qi::os::sleep(1);
  ASSERT_EQ(sp.getCounter(), 2u);

  qi::DynamicObjectBuilder ob;
  ob.advertiseSignal("signal", &sig);
  qi::AnyObject obj(ob.object());
  qi::SignalSpy sp2(obj, "signal");
  QI_EMIT sig(1);
  QI_EMIT sig(1);
  qi::os::sleep(1);
  ASSERT_EQ(sp2.getCounter(), 2u);
}

TEST(TestSignalSpy, Async)
{
  qi::Signal<int> sig;
  qi::SignalSpy sp(sig);
  qi::async<void>(boost::bind(boost::ref(sig), 1));
  qi::async<void>(boost::bind(boost::ref(sig), 1));
  ASSERT_TRUE(sp.waitUntil(2, qi::Seconds(1)));
  ASSERT_EQ(sp.getCounter(), 2u);
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
