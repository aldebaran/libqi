/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qimessaging/signal.hpp>
#include <qimessaging/future.hpp>
#include <qi/application.hpp>

class Foo
{
public:
  void func(int)               { }
  void func1(int *r, int)      { *r += 1; }
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
  sig(i, &done);
  ASSERT_TRUE(done); //synchronous
  ASSERT_EQ(12, i); // byref
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

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
