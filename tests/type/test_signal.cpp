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

namespace
{
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
} // anonymous

void fire(int &a) { a = 44; }

TEST(TestSignal, RegisterSignalAndMethodWithSameSignature)
{
  qi::DynamicObjectBuilder ob;
  qi::Property<int> prop;

  ob.advertiseSignal<int&>("fire1");
  ASSERT_THROW(ob.advertiseMethod("fire1", &fire), std::runtime_error);
  ASSERT_THROW(ob.advertiseProperty("fire1", &prop), std::runtime_error);

  ob.advertiseMethod("fire2", &fire);
  ASSERT_THROW(ob.advertiseSignal<int&>("fire2"), std::runtime_error);
  ASSERT_THROW(ob.advertiseProperty("fire2", &prop), std::runtime_error);

  ob.advertiseProperty("fire3", &prop);
  ASSERT_THROW(ob.advertiseSignal<int&>("fire3"), std::runtime_error);
  ASSERT_THROW(ob.advertiseMethod("fire3", &fire), std::runtime_error);

}

TEST(TestSignal, RegisterSignalAndMethodWithDifferentSignature)
{
  qi::DynamicObjectBuilder ob;
  qi::Property<char> prop;

  ob.advertiseSignal<float&>("fire1");
  ASSERT_NO_THROW(ob.advertiseMethod("fire1", &fire));
  ASSERT_NO_THROW(ob.advertiseProperty("fire1", &prop));

  ob.advertiseMethod("fire2", &fire);
  ASSERT_NO_THROW(ob.advertiseSignal<float&>("fire2"));
  ASSERT_NO_THROW(ob.advertiseProperty("fire2", &prop));

  ob.advertiseProperty("fire3", &prop);
  ASSERT_NO_THROW(ob.advertiseSignal<float&>("fire3"));
  ASSERT_NO_THROW(ob.advertiseMethod("fire3", &fire));
}


TEST(TestSignal, TestCompilation)
{
  qi::Atomic<int>        res{0};
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

  while (res.load() != 6)
    qi::os::msleep(10);

  ASSERT_EQ(6, res.load());
  ASSERT_TRUE(prom.future().isFinished());
  ASSERT_FALSE(prom.future().hasError());
}

TEST(TestSignal, SignalHasNoSubscriberByDefault)
{
  qi::Signal<int> signal;
  ASSERT_FALSE(signal.hasSubscribers());
}

TEST(TestSignal, EmitWithNoSubscriber)
{
  qi::Signal<int> signal;
  QI_EMIT signal(42);
}

TEST(TestSignal, EmitSharedPointerWithNoSubscriber)
{
  qi::Signal<boost::shared_ptr<int>> signal;
  QI_EMIT signal(boost::make_shared<int>(42));
}

void write42(boost::shared_ptr<int> ptr)
{
  *ptr = 42;
}

TEST(TestSignal, EmitSharedPointerWithDirectSubscriber)
{
  // Redundant with Copy test, but just to be sure, check that shared_ptr
  // is correctly transmited.
  qi::Signal<boost::shared_ptr<int> > sig;
  sig.connect(qi::AnyFunction::from(&write42)).setCallType(qi::MetaCallType_Direct);
  {
    auto ptr = boost::make_shared<int>(12);
    QI_EMIT sig(ptr);
  };
}

TEST(TestSignal, EmitSharedPointerWithQueuedSubscriber)
{
  // Redundant with Copy test, but just to be sure, check that shared_ptr
  // is correctly transmited.
  qi::Signal<boost::shared_ptr<int> > sig;
  sig.connect(qi::AnyFunction::from(&write42)).setCallType(qi::MetaCallType_Queued);
  {
    auto ptr = boost::make_shared<int>(12);
    QI_EMIT sig(ptr);
  };
}

void byRef(int& i, bool* done)
{
  qiLogDebug() <<"byRef " << &i << ' ' << done;
  i = 12;
  *done = true;
}

TEST(TestSignal, FunctionDestroyedOnDisconnection)
{
  std::atomic<bool> destroyed{false};
  std::shared_ptr<int> sharedInt{new int{42}, [&](int* i){ delete i; destroyed = true; }};
  qi::Signal<void> signal;
  qi::SignalLink link = signal.connect([sharedInt]{});
  sharedInt.reset();
  ASSERT_FALSE(destroyed);
  signal.disconnect(link);
  ASSERT_TRUE(destroyed);
}



TEST(TestSignal, AutoDisconnect)
{
  // Test automatic disconnection when passing shared_ptrs
  qi::Atomic<int> r{0};
  boost::shared_ptr<Foo> foo(new Foo());
  qi::Signal<qi::Atomic<int>*, int> sig;
  sig.connect(&Foo::func1, boost::weak_ptr<Foo>(foo), _1, _2).setCallType(qi::MetaCallType_Direct);
  sig(&r, 0);
  ASSERT_EQ(1, r.load());
  foo.reset();
  sig(&r, 0);
  ASSERT_EQ(1, r.load());
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
  qi::Atomic<int> cnt{0};

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
  sig1.connect(*sig2);
  qi::SignalSpy spy(*sig2);
  sig1(10);
  ASSERT_TRUE(spy.waitUntil(1, qi::MilliSeconds(300)));
  ASSERT_EQ(10, spy.record(0).arg<int>(0));

  // Test autodisconnect
  delete sig2;
  sig1(20);
  ASSERT_FALSE(spy.waitUntil(2, qi::MilliSeconds(300)));
}

class SignalTest {
public:
  qi::Signal<int> sig;
  qi::Signal<int> sig2;
};


TEST(TestSignal, SignalSignal2)
{
  SignalTest st;
  qi::SignalSpy spy(st.sig2);
  st.sig.connect(st.sig2);
  qiLogDebug() << "sigptrs are " << &st.sig << " " << &st.sig2;
  st.sig(4242);
  ASSERT_TRUE(spy.waitUntil(1, qi::MilliSeconds(300)));
  assert(spy.recordCount() == 1u);
  EXPECT_EQ(4242, spy.lastRecord().arg<int>(0));
}

TEST(TestSignal, SignalN)
{
  qi::Signal<int> sig;
  qi::SignalSpy spy(sig);
  sig(5);
  ASSERT_TRUE(spy.waitUntil(1, qi::MilliSeconds(300)));
  ASSERT_EQ(5, spy.lastRecord().arg<int>(0));
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

qi::Future<void> onSubs(std::atomic<bool>& var, bool subs)
{
  var = subs;
  return qi::Future<void>{0};
}

void callback(int i)
{}

TEST(TestSignal, OnSubscriber)
{
  std::atomic<bool> subscribers(false);

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

TEST(TestSignal, SignalSubscriberDoesNotUnsubscribeAtDestruction)
{
  int count = 0;
  qi::Signal<void> signal;
  {
    auto subscriber = signal.connect([&]{ ++count; })
        .setCallType(qi::MetaCallType_Direct);
    QI_UNUSED(subscriber); // we are just keeping it alive for a moment

    signal();
    ASSERT_EQ(1, count);
  }
  signal();
  ASSERT_EQ(2, count);
}

// ===========================================================
// Signal Spy
// -----------------------------------------------------------
TEST(TestSignalSpy, Counter)
{
  qi::Signal<int> sig;
  qi::SignalSpy sp(sig);
  QI_EMIT sig(1);
  QI_EMIT sig(1);
  ASSERT_TRUE(sp.waitUntil(2, qi::MilliSeconds(300)));
  ASSERT_EQ(sp.recordCount(), 2u);

  qi::DynamicObjectBuilder ob;
  ob.advertiseSignal("signal", &sig);
  qi::AnyObject obj(ob.object());
  qi::SignalSpy sp2(obj, "signal");
  QI_EMIT sig(1);
  QI_EMIT sig(1);
  ASSERT_TRUE(sp2.waitUntil(2, qi::MilliSeconds(300)));
  ASSERT_EQ(sp2.recordCount(), 2u);
}

TEST(TestSignalSpy, Async)
{
  qi::Signal<int> sig;
  qi::SignalSpy sp(sig);
  qi::async(boost::bind(boost::ref(sig), 1));
  qi::async(boost::bind(boost::ref(sig), 1));
  ASSERT_TRUE(sp.waitUntil(2, qi::Seconds(1)));
  ASSERT_EQ(sp.recordCount(), 2u);
}

TEST(TestSignalSpy, StoringTypedValueRecords)
{
  const std::vector<int> ints{ 1, 42, 13, 2016 };
  const std::vector<std::string> strings{ "poil", "slip", "banane", "pancréas" };
  qi::Signal<int, std::string> signal;
  qi::SignalSpy spy(signal);

  signal(ints[0], strings[0]);
  ASSERT_EQ(1u, spy.recordCount());
  auto record = spy.record(0);
  ASSERT_EQ(ints[0], record.arg<int>(0));
  ASSERT_EQ(strings[0], record.arg<std::string>(1));

  for(auto i = 1u; i < ints.size(); ++i)
  {
    signal(ints[i], strings[i]);
  }
  spy.waitUntil(ints.size(), qi::MilliSeconds(300));

  for(auto i = 1u; i < ints.size(); ++i)
  {
    record = spy.record(i);
    EXPECT_EQ(ints[i], record.arg<int>(0));
    EXPECT_EQ(strings[i], record.arg<std::string>(1));
  }

  auto records = spy.allRecords();
  ASSERT_EQ(ints.size(), records.size());
  for(auto i = 0u; i < records.size(); ++i)
  {
    EXPECT_EQ(ints[i], records[i].arg<int>(0));
    EXPECT_EQ(strings[i], records[i].arg<std::string>(1));
  }
}
