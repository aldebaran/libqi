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
void foolast(int, qi::Promise<void> prom) { prom.setValue(0); }


TEST(TestSignal, TestCompilation)
{
  int                    res = 0;
  qi::Signal<void (int)> s;
  Foo*                   f = 0;
  qi::Promise<void>      prom;

  //do not count
  s.connect(qi::makeGenericFunction(f, &Foo::func));
  s.connect(boost::bind<void>(&Foo::func, f, _1));

  s.connect(boost::bind(&foo, &res, 12, _1));
  s.connect(boost::bind(&foo2, &res, 12, _1));
  s.connect(boost::bind<void>(&Foo::func1, f, &res, _1));
  s.connect(boost::bind<void>(&Foo::func2, f, &res, 5, _1));
  s.connect(boost::bind(&foo3, &res, f));
  s.connect(boost::bind(&foolast, _1, prom));

  s(42);
  int timeout = 1000;
  while (timeout > 0) {
    qi::os::msleep(1);
    if (res == 5)
      break;
    timeout -= 1;
  }
  ASSERT_EQ(5, res);
  ASSERT_TRUE(prom.future().isReady());
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
  qi::Signal<void (boost::shared_ptr<int>)> sig;
  sig.connect(qi::makeGenericFunction(&write42), qi::getDefaultObjectEventLoop());
  {
    boost::shared_ptr<int> ptr(new int(12));
    sig(ptr);
  };
}

void byRef(int& i, bool* done)
{
  qiLogDebug("test") <<"byRef " << &i;
  i = 12;
  *done = true;
}

TEST(TestSignal, Copy)
{
  // Check that reference argument type are copied when an async call is made
  qi::Signal<void (int&, bool*)> sig;
  qiLogDebug("test") << "sync";
  sig.connect(qi::makeGenericFunction(byRef), 0);
  bool done = false;
  int i = 0;
  qiLogDebug("test") << "iref is " << &i;
  sig(i, &done);
  ASSERT_TRUE(done); //synchronous
  ASSERT_EQ(0, i); // byref, but still copies for small types
  qiLogDebug("test") << "async";
  sig =  qi::Signal<void (int&, bool*)>();
  sig.connect(qi::makeGenericFunction(byRef), qi::getDefaultObjectEventLoop());
  i = 0;
  done = false;
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
  qi::Signal<void (int*, int)> sig;
  sig.connect(foo, &Foo::func1, 0);
  sig(&r, 0);
  ASSERT_EQ(1, r);
  foo.reset();
  sig(&r, 0);
  ASSERT_EQ(1, r);
}

class ManageableFoo: public Foo, public qi::Manageable
{};

TEST(TestSignal, Manageable)
{
  // Test manageable detection
  int r = 0;
  qi::Signal<void (int*, int)> sig;
  { // just a scope to ensure later test does not use mf
    ManageableFoo* mf = new ManageableFoo();
    mf->moveToEventLoop(0);
    sig.connect(mf, &ManageableFoo::func1, 0);
    sig(&r, 0); // synchronous
    ASSERT_EQ(1, r);
    mf->moveToEventLoop(qi::getDefaultObjectEventLoop());
    sig(&r, 0); // asynchronous
    ASSERT_EQ(1, r);
    for(unsigned i=0; i<10 && r==1; ++i) qi::os::msleep(50)
      ;
    ASSERT_EQ(2, r);
    mf->moveToEventLoop(0);
    sig(&r, 0); // synchronous
    ASSERT_EQ(3, r);
    sig.disconnectAll();
    delete mf;
  }
  // Check that manageable detection works on shared_ptr
  boost::shared_ptr<ManageableFoo> shared(new ManageableFoo());
  sig.connect(shared, &ManageableFoo::func1, 0);
  r = 0;
  shared->moveToEventLoop(0);
  sig(&r, 0); // synchronous
  ASSERT_EQ(1, r);
  shared->moveToEventLoop(qi::getDefaultObjectEventLoop());
  sig(&r, 0); // asynchronous
  ASSERT_EQ(1, r);
  for(unsigned i=0; i<10 && r==1; ++i) qi::os::msleep(50)
    ;
  ASSERT_EQ(2, r);
  shared->moveToEventLoop(0);
  sig(&r, 0); // synchronous
  ASSERT_EQ(3, r);
  shared.reset();
  sig(&r, 0); // disconnected
  ASSERT_EQ(3, r);
}

TEST(TestSignal, BadArity)
{
  qi::Signal<void()> s;
  // caught at compile-time by signal<T>
  ASSERT_EQ(qi::SignalBase::invalidLink, ((qi::SignalBase&)s).connect(&foo));
  //idem
  ASSERT_EQ(qi::SignalBase::invalidLink, ((qi::SignalBase&)s).connect(&foo2));
  ASSERT_EQ(qi::SignalBase::invalidLink, s.connect((Foo*)0, &Foo::func1));
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
