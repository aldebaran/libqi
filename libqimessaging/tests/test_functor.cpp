/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <map>
#include <qimessaging/functor.hpp>
#include <qimessaging/details/makefunctor.hpp>
#include <qimessaging/details/callfunctor.hpp>
#include <cmath>

static int gGlobalResult = 0;

void vfun0()                                                                                      { gGlobalResult = 0; }
void vfun1(const int &p0)                                                                         { gGlobalResult = p0; }
void vfun2(const int &p0,const int &p1)                                                           { gGlobalResult = p0 + p1; }
void vfun3(const int &p0,const int &p1,const int &p2)                                             { gGlobalResult = p0 + p1 + p2; }
void vfun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { gGlobalResult = p0 + p1 + p2 + p3; }
void vfun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { gGlobalResult = p0 + p1 + p2 + p3 + p4; }
void vfun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { gGlobalResult = p0 + p1 + p2 + p3 + p4 + p5; }

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

  void vfun0()                                                                                      { gGlobalResult = 0; }
  void vfun1(const int &p0)                                                                         { gGlobalResult = p0; }
  void vfun2(const int &p0,const int &p1)                                                           { gGlobalResult = p0 + p1; }
  void vfun3(const int &p0,const int &p1,const int &p2)                                             { gGlobalResult = p0 + p1 + p2; }
  void vfun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { gGlobalResult = p0 + p1 + p2 + p3; }
  void vfun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { gGlobalResult = p0 + p1 + p2 + p3 + p4; }
  void vfun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { gGlobalResult = p0 + p1 + p2 + p3 + p4 + p5; }
};


TEST(TestBind, MultiArgMember) {
  Foo          foo;
  qi::Functor *functor;

  functor = qi::makeFunctor(&foo, &Foo::fun0);
  EXPECT_EQ(0 , qi::callFunctor<int>(functor));
  functor = qi::makeFunctor(&foo, &Foo::fun1);
  EXPECT_EQ(1 , qi::callFunctor<int>(functor, 1));
  functor = qi::makeFunctor(&foo, &Foo::fun2);
  EXPECT_EQ(3 , qi::callFunctor<int>(functor, 1, 2));
  functor = qi::makeFunctor(&foo, &Foo::fun3);
  EXPECT_EQ(6 , qi::callFunctor<int>(functor, 1, 2, 3));
  functor = qi::makeFunctor(&foo, &Foo::fun4);
  EXPECT_EQ(10, qi::callFunctor<int>(functor, 1, 2, 3, 4));
  functor = qi::makeFunctor(&foo, &Foo::fun5);
  EXPECT_EQ(15, qi::callFunctor<int>(functor, 1, 2, 3, 4, 5));
  functor = qi::makeFunctor(&foo, &Foo::fun6);
  EXPECT_EQ(21, qi::callFunctor<int>(functor, 1, 2, 3, 4, 5, 6));
}

TEST(TestBind, MultiArgVoidMember) {
  Foo          foo;
  qi::Functor *functor;

  functor = qi::makeFunctor(&foo, &Foo::vfun0);
  qi::callFunctor<void>(functor);
  EXPECT_EQ(0, gGlobalResult);

  functor = qi::makeFunctor(&foo, &Foo::vfun1);
  qi::callFunctor<void>(functor, 1);
  EXPECT_EQ(1, gGlobalResult);

  functor = qi::makeFunctor(&foo, &Foo::vfun2);
  qi::callFunctor<void>(functor, 1, 2);
  EXPECT_EQ(3, gGlobalResult);

  functor = qi::makeFunctor(&foo, &Foo::vfun3);
  qi::callFunctor<void>(functor, 1, 2, 3);
  EXPECT_EQ(6, gGlobalResult);

  functor = qi::makeFunctor(&foo, &Foo::vfun4);
  qi::callFunctor<void>(functor, 1, 2, 3, 4);
  EXPECT_EQ(10, gGlobalResult);

  functor = qi::makeFunctor(&foo, &Foo::vfun5);
  qi::callFunctor<void>(functor, 1, 2, 3, 4, 5);
  EXPECT_EQ(15, gGlobalResult);

  functor = qi::makeFunctor(&foo, &Foo::vfun6);
  qi::callFunctor<void>(functor, 1, 2, 3, 4, 5, 6);
  EXPECT_EQ(21, gGlobalResult);
}


TEST(TestBind, MultiArgFun) {
  qi::Functor *functor;

  functor = qi::makeFunctor(&fun0);
  EXPECT_EQ(0 , qi::callFunctor<int>(functor));
  functor = qi::makeFunctor(&fun1);
  EXPECT_EQ(1 , qi::callFunctor<int>(functor, 1));
  functor = qi::makeFunctor(&fun2);
  EXPECT_EQ(3 , qi::callFunctor<int>(functor, 1, 2));
  functor = qi::makeFunctor(&fun3);
  EXPECT_EQ(6 , qi::callFunctor<int>(functor, 1, 2, 3));
  functor = qi::makeFunctor(&fun4);
  EXPECT_EQ(10, qi::callFunctor<int>(functor, 1, 2, 3, 4));
  functor = qi::makeFunctor(&fun5);
  EXPECT_EQ(15, qi::callFunctor<int>(functor, 1, 2, 3, 4, 5));
  functor = qi::makeFunctor(&fun6);
  EXPECT_EQ(21, qi::callFunctor<int>(functor, 1, 2, 3, 4, 5, 6));
}

TEST(TestBind, MultiArgVoidFun) {
  qi::Functor *functor;

  functor = qi::makeFunctor(&vfun0);
  qi::callFunctor<void>(functor);
  EXPECT_EQ(0, gGlobalResult);

  functor = qi::makeFunctor(&vfun1);
  qi::callFunctor<void>(functor, 1);
  EXPECT_EQ(1, gGlobalResult);

  functor = qi::makeFunctor(&vfun2);
  qi::callFunctor<void>(functor, 1, 2);
  EXPECT_EQ(3, gGlobalResult);

  functor = qi::makeFunctor(&vfun3);
  qi::callFunctor<void>(functor, 1, 2, 3);
  EXPECT_EQ(6, gGlobalResult);

  functor = qi::makeFunctor(&vfun4);
  qi::callFunctor<void>(functor, 1, 2, 3, 4);
  EXPECT_EQ(10, gGlobalResult);

  functor = qi::makeFunctor(&vfun5);
  qi::callFunctor<void>(functor, 1, 2, 3, 4, 5);
  EXPECT_EQ(15, gGlobalResult);

  functor = qi::makeFunctor(&vfun6);
  qi::callFunctor<void>(functor, 1, 2, 3, 4, 5, 6);
  EXPECT_EQ(21, gGlobalResult);
}
