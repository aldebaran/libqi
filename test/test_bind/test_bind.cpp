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
#include <alcommon-ng/functor/callfunctor.hpp>
#include <alcommon-ng/tools/dataperftimer.hpp>
#include <cmath>

static const int gLoopCount   = 1000000;

using AL::Messaging::ReturnValue;
using AL::Messaging::ArgumentList;


int fun0()                                                                                      { return 0; }
int fun1(const int &p0)                                                                         { return p0; }
int fun2(const int &p0,const int &p1)                                                           { return p0 + p1; }
int fun3(const int &p0,const int &p1,const int &p2)                                             { return p0 + p1 + p2; }
int fun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { return p0 + p1 + p2 + p3; }
int fun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { return p0 + p1 + p2 + p3 + p4; }
int fun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { return p0 + p1 + p2 + p3 + p4 + p5; }

struct Foo {
  void voidCall()                                          { return; }
  int intStringCall(const std::string &plouf)              { return plouf.size(); }

  int fun0()                                                                                      { return 0; }
  int fun1(const int &p0)                                                                         { return p0; }
  int fun2(const int &p0,const int &p1)                                                           { return p0 + p1; }
  int fun3(const int &p0,const int &p1,const int &p2)                                             { return p0 + p1 + p2; }
  int fun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { return p0 + p1 + p2 + p3; }
  int fun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { return p0 + p1 + p2 + p3 + p4; }
  int fun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { return p0 + p1 + p2 + p3 + p4 + p5; }
};


TEST(TestBind, MultiArgMember) {
  Foo          foo;
  AL::Functor *functor;

  functor = AL::makeFunctor(&foo, &Foo::fun0);
  EXPECT_EQ(0 , AL::callFunctor<int>(functor));
  functor = AL::makeFunctor(&foo, &Foo::fun1);
  EXPECT_EQ(1 , AL::callFunctor<int>(functor, 1));
  functor = AL::makeFunctor(&foo, &Foo::fun2);
  EXPECT_EQ(3 , AL::callFunctor<int>(functor, 1, 2));
  functor = AL::makeFunctor(&foo, &Foo::fun3);
  EXPECT_EQ(6 , AL::callFunctor<int>(functor, 1, 2, 3));
  functor = AL::makeFunctor(&foo, &Foo::fun4);
  EXPECT_EQ(10, AL::callFunctor<int>(functor, 1, 2, 3, 4));
  functor = AL::makeFunctor(&foo, &Foo::fun5);
  EXPECT_EQ(15, AL::callFunctor<int>(functor, 1, 2, 3, 4, 5));
  functor = AL::makeFunctor(&foo, &Foo::fun6);
  EXPECT_EQ(21, AL::callFunctor<int>(functor, 1, 2, 3, 4, 5, 6));
}

TEST(TestBind, MultiArgFun) {
  AL::Functor *functor;

  functor = AL::makeFunctor(&fun0);
  EXPECT_EQ(0 , AL::callFunctor<int>(functor));
  functor = AL::makeFunctor(&fun1);
  EXPECT_EQ(1 , AL::callFunctor<int>(functor, 1));
  functor = AL::makeFunctor(&fun2);
  EXPECT_EQ(3 , AL::callFunctor<int>(functor, 1, 2));
  functor = AL::makeFunctor(&fun3);
  EXPECT_EQ(6 , AL::callFunctor<int>(functor, 1, 2, 3));
  functor = AL::makeFunctor(&fun4);
  EXPECT_EQ(10, AL::callFunctor<int>(functor, 1, 2, 3, 4));
  functor = AL::makeFunctor(&fun5);
  EXPECT_EQ(15, AL::callFunctor<int>(functor, 1, 2, 3, 4, 5));
  functor = AL::makeFunctor(&fun6);
  EXPECT_EQ(21, AL::callFunctor<int>(functor, 1, 2, 3, 4, 5, 6));
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
