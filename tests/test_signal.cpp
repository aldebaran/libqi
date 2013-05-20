/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qitype/signal.hpp>
#include <qi/future.hpp>
#include <qi/application.hpp>
#include <qitype/functiontypefactory.hpp>
#include <qitype/genericobject.hpp>

qiLogCategory("test");

class Foo
{
public:
  void func(int)               { }
  void func1(int *r, int)
  {
    // hackish sleep so that asynchronous trigger detection is safer
    qi::os::msleep(1); *r += 1;
  }
  void func2(int *r, int, int) { *r += 1; }
};
void foo(int *r, int, int)     { *r += 1; }
void foo2(int *r, char, char)  { *r += 1; }
void foo3(int *r, Foo *)       { *r += 1; }
void foolast(int, qi::Promise<void> prom, int* r) { prom.setValue(0); *r += 1;}

TEST(TestSignal, TestCompilation)
{
  int                    res = 0;
  qi::Signal<int> s;
  Foo*                   f = (Foo*)1;
  qi::Promise<void>      prom;

  //do not count
  s.connect(qi::makeGenericFunction(&Foo::func, f));
  s.connect(boost::bind<void>(&Foo::func, f, _1));

  s.connect(boost::bind(&foo, &res, 12, _1));
  s.connect(boost::bind(&foo2, &res, 12, _1));
  s.connect(boost::bind<void>(&Foo::func1, f, &res, _1));
  s.connect(boost::bind<void>(&Foo::func2, f, &res, 5, _1));
  s.connect(boost::bind(&foo3, &res, f));
  s.connect(boost::bind(&foolast, _1, prom, &res));

  s(42);
  int timeout = 100;
  while (timeout > 0) {
    qi::os::msleep(10);
    if (res == 6)
      break;
    timeout -= 1;
  }
  ASSERT_EQ(6, res);
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
  sig.connect(qi::makeGenericFunction(&write42), qi::MetaCallType_Queued);
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

TEST(TestSignal, Copy)
{
  // Check that reference argument type are copied when an async call is made
  qi::Signal<int&, bool*> sig;
  qiLogDebug() << "sync";
  sig.connect(qi::makeGenericFunction(byRef), qi::MetaCallType_Direct);
  bool done = false;
  int i = 0;
  qiLogDebug() << "iref is " << &i;
  sig(i, &done);
  ASSERT_TRUE(done); //synchronous
  //ASSERT_EQ(0, i); // byref, but still copies for small types
  qiLogDebug() << "async";
  sig =  qi::Signal<int&, bool*>();
  sig.connect(qi::makeGenericFunction(byRef), qi::MetaCallType_Queued);
  i = 0;
  done = false;
  qiLogDebug() << "done is " << &done;
  sig(i, &done);
  for (unsigned c=0; !done && c<100;++c) qi::os::msleep(10);
  ASSERT_TRUE(done);
  ASSERT_EQ(0, i); // async: was copied
}

TEST(TestSignal, AutoDisconnect)
{
  // Test automatic disconnection when passing shared_ptrs
  int r = 0;
  boost::shared_ptr<Foo> foo(new Foo());
  qi::Signal<int*, int> sig;
  sig.connect(foo, &Foo::func1, qi::MetaCallType_Direct);
  sig(&r, 0);
  ASSERT_EQ(1, r);
  foo.reset();
  sig(&r, 0);
  ASSERT_EQ(1, r);
}

TEST(TestSignal, AutoDisconnectTrack)
{
  // Test auto-disconnect using external tracker
  int r = 0;
  boost::shared_ptr<int> s(new int(2));
  Foo* ptr = new Foo();
  qi::Signal<int*, int> sig;
  sig.connect(ptr, &Foo::func1, qi::MetaCallType_Direct).track(boost::weak_ptr<int>(s));
  sig(&r, 0);
  ASSERT_EQ(1, r);
  sig(&r, 1);
  ASSERT_EQ(2, r);
  s.reset();
  sig(&r, 1);
  ASSERT_EQ(2, r);
}


TEST(TestSignal, BadArity)
{
  qi::Signal<> s;
  // caught at compile-time by signal<T>
  ASSERT_EQ(qi::SignalBase::invalidLink, ((qi::SignalBase&)s).connect(&foo));
  //idem
  ASSERT_EQ(qi::SignalBase::invalidLink, ((qi::SignalBase&)s).connect(&foo2));
  ASSERT_EQ(qi::SignalBase::invalidLink, s.connect((Foo*)0, &Foo::func1));
}

void lol(int v, int& target)
{
  target = v;
}
TEST(TestSignal, SignalSignal)
{
  qi::SignalF<void (int)> sig1;
  qi::SignalF<void (int)> sig2;
  int res = 0;
  sig1.connect(sig2);
  sig2.connect(boost::bind<void>(&lol, _1, boost::ref(res)));
  sig1(10);
  qi::os::msleep(300);
  ASSERT_EQ(10, res);
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

QI_REGISTER_OBJECT(SigHolder, s0, s1, s2, fire0, fire1, fire2);

TEST(TestSignal, SignalNBind)
{
  int res = 0;
  boost::shared_ptr<SigHolder> so(new SigHolder);
  qi::ObjectPtr op = qi::GenericValueRef(so).to<qi::ObjectPtr>();
  qi::details::printMetaObject(std::cerr, op->metaObject());
  op->connect("s1", (boost::function<void(int)>)boost::bind<void>(&lol, _1, boost::ref(res)));
  op->post("s1", 2);
  for (unsigned i=0; i<30 && res!=2; ++i) qi::os::msleep(10);
  ASSERT_EQ(2, res);
  so->s1(3);
  for (unsigned i=0; i<30 && res!=3; ++i) qi::os::msleep(10);
  ASSERT_EQ(3, res);
  so->s0.connect( boost::bind<void>(&lol, 42, boost::ref(res)));
  op->post("s0");
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


int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
