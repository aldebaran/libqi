/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <map>
#include <alcommon-ng/functor/functor.hpp>
#include <alcommon-ng/functor/makefunctor.hpp>
#include <alcommon-ng/tools/dataperftimer.hpp>


static const int gLoopCount   = 1000000;

using AL::Messaging::ReturnValue;
using AL::Messaging::ArgumentList;


int fun0()                                               { return 0; }
int fun1(int p0)                                         { return p0; }
int fun2(int p0, int p1)                                 { return p0 + p1; }
int fun3(int p0, int p1, int p2)                         { return p0 + p1 + p2; }
int fun4(int p0, int p1, int p2, int p3)                 { return p0 + p1 + p2 + p3; }
int fun5(int p0, int p1, int p2, int p3, int p4)         { return p0 + p1 + p2 + p3 + p4; }
int fun6(int p0, int p1, int p2, int p3, int p4, int p5) { return p0 + p1 + p2 + p3 + p4 + p5; }

struct Foo {
  void voidCall()                                          { return; }
  int intStringCall(const std::string &plouf)              { return plouf.size(); }

  int fun0()                                               { return 0; }
  int fun1(int p0)                                         { return p0; }
  int fun2(int p0, int p1)                                 { return p0 + p1; }
  int fun3(int p0, int p1, int p2)                         { return p0 + p1 + p2; }
  int fun4(int p0, int p1, int p2, int p3)                 { return p0 + p1 + p2 + p3; }
  int fun5(int p0, int p1, int p2, int p3, int p4)         { return p0 + p1 + p2 + p3 + p4; }
  int fun6(int p0, int p1, int p2, int p3, int p4, int p5) { return p0 + p1 + p2 + p3 + p4 + p5; }
};


TEST(TestBind, ArgumentNumber) {
  Foo     chiche;

  //AL::Functor *functor = AL::makeFunctor(&Foo, &Foo::fun0);
  //EXPECT_EQ(0, functor->call());

}

TEST(TestBind, VoidCallPerf) {
  Foo           chiche;
  Foo          *p = &chiche;
  ReturnValue   res;
  ArgumentList  cd;

  AL::Test::DataPerfTimer dp;
  AL::Functor    *functor = AL::makeFunctor(&chiche, &Foo::voidCall);
  std::cout << "AL::Functor call" << std::endl;
  dp.start(gLoopCount);
  for (int i = 0; i < gLoopCount; ++i)
  {
    functor->call(cd, res);
  }
  dp.stop();

  std::cout << "pointer call" << std::endl;
  dp.start(gLoopCount);
  for (int i = 0; i < gLoopCount; ++i)
  {
    p->voidCall();
  }
  dp.stop();
}

TEST(TestBind, IntStringCallPerf) {
  Foo           chiche;
  Foo          *p = &chiche;
  ReturnValue res;

  AL::Test::DataPerfTimer dp;

  std::cout << "AL::Functor call (string with a growing size)" << std::endl;

  for (int i = 0; i < 12; ++i)
  {
    unsigned int    numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string     request = std::string(numBytes, 'B');
    ArgumentList  cd;
    AL::Functor    *functor = AL::makeFunctor(&chiche, &Foo::intStringCall);

    cd.push_back(request);
    dp.start(gLoopCount, numBytes);
    for (int j = 0; j < gLoopCount; ++j) {
      functor->call(cd, res);
    }
    dp.stop();
  }

  std::cout << "pointer call (string with a growing size)" << std::endl;
  for (int i = 0; i < 12; ++i)
  {
    unsigned int    numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string     request = std::string(numBytes, 'B');

    dp.start(gLoopCount, numBytes);
    for (int j = 0; j < gLoopCount; ++j) {
      p->intStringCall(request);
    }
    dp.stop();
  }

}
