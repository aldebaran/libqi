/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
//#include <alcommon-ng/functor/functionsignature.hpp>
#include <alcommon-ng/functor/typesignature.hpp>
//#include <alcommon-ng/tools/dataperftimer.hpp>

#include <vector>
#include <map>


static const int gLoopCount   = 1000000;

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

TEST(TestSignature, BasicType) {
  Foo          foo;

  std::string s;
  EXPECT_EQ("b",    AL::typeSignatureWithCopy<bool>::value());
  EXPECT_EQ("i",    AL::typeSignatureWithCopy<int>::value());
  EXPECT_EQ("f",    AL::typeSignatureWithCopy<float>::value());
  EXPECT_EQ("d",    AL::typeSignatureWithCopy<double>::value());
  EXPECT_EQ("s",    AL::typeSignatureWithCopy<std::string>::value());
  EXPECT_EQ("[i]",  AL::typeSignatureWithCopy< std::vector<int> >::value());
  typedef std::map<int,int> MapInt;
  EXPECT_EQ("{ii}", AL::typeSignatureWithCopy< MapInt >::value() );

  EXPECT_EQ("b",    AL::typeSignatureWithCopy<const bool>::value());
  EXPECT_EQ("i",    AL::typeSignatureWithCopy<const int>::value());
  EXPECT_EQ("f",    AL::typeSignatureWithCopy<const float>::value());
  EXPECT_EQ("d",    AL::typeSignatureWithCopy<const double>::value());
  EXPECT_EQ("s",    AL::typeSignatureWithCopy<const std::string>::value());
  EXPECT_EQ("[i]",  AL::typeSignatureWithCopy<const std::vector< int > >::value());
  EXPECT_EQ("{ii}", AL::typeSignatureWithCopy<const MapInt >::value());

  EXPECT_EQ("*b",   AL::typeSignatureWithCopy<const bool*>::value());
  EXPECT_EQ("*i",   AL::typeSignatureWithCopy<const int*>::value());
  EXPECT_EQ("*f",   AL::typeSignatureWithCopy<const float*>::value());
  EXPECT_EQ("*d",   AL::typeSignatureWithCopy<const double*>::value());
  EXPECT_EQ("*s",   AL::typeSignatureWithCopy<const std::string*>::value());
  EXPECT_EQ("*[i]", AL::typeSignatureWithCopy<const std::vector< int >* >::value());
  EXPECT_EQ("*{ii}",AL::typeSignatureWithCopy<const MapInt* >::value());

  EXPECT_EQ("b",    AL::typeSignatureWithCopy<const bool&>::value());
  EXPECT_EQ("i",    AL::typeSignatureWithCopy<const int&>::value());
  EXPECT_EQ("f",    AL::typeSignatureWithCopy<const float&>::value());
  EXPECT_EQ("d",    AL::typeSignatureWithCopy<const double&>::value());
  EXPECT_EQ("s",    AL::typeSignatureWithCopy<const std::string&>::value());
  EXPECT_EQ("(i)",  AL::typeSignatureWithCopy<const std::vector< int >& >::value());
  EXPECT_EQ("{ii}", AL::typeSignatureWithCopy<const MapInt& >::value());


  //ERROR
  EXPECT_EQ("UNKNOWN", AL::typeSignatureWithCopy<short>::value());
  //  functor = AL::makeFunctor(&foo, &Foo::fun1);
  //  EXPECT_EQ(1 , AL::callFunctor<int>(functor, 1));
  //  functor = AL::makeFunctor(&foo, &Foo::fun2);
  //  EXPECT_EQ(3 , AL::callFunctor<int>(functor, 1, 2));
  //  functor = AL::makeFunctor(&foo, &Foo::fun3);
  //  EXPECT_EQ(6 , AL::callFunctor<int>(functor, 1, 2, 3));
  //  functor = AL::makeFunctor(&foo, &Foo::fun4);
  //  EXPECT_EQ(10, AL::callFunctor<int>(functor, 1, 2, 3, 4));
  //  functor = AL::makeFunctor(&foo, &Foo::fun5);
  //  EXPECT_EQ(15, AL::callFunctor<int>(functor, 1, 2, 3, 4, 5));
  //  functor = AL::makeFunctor(&foo, &Foo::fun6);
  //  EXPECT_EQ(21, AL::callFunctor<int>(functor, 1, 2, 3, 4, 5, 6));
}




TEST(TestSignature, BasicSignature) {
  Foo          foo;

  std::string val;
  //std::cout << AL::typeSignatureWithCopy<int>::value(val) << std::endl;

//  std::plus<int> add;
//  assert(invoke(add,make_vector(1,1)) == 2);

//  EXPECT_EQ("v:p0", AL::functionSignature(&vfun1));
//  functor = AL::makeFunctor(&foo, &Foo::fun1);
//  EXPECT_EQ(1 , AL::callFunctor<int>(functor, 1));
//  functor = AL::makeFunctor(&foo, &Foo::fun2);
//  EXPECT_EQ(3 , AL::callFunctor<int>(functor, 1, 2));
//  functor = AL::makeFunctor(&foo, &Foo::fun3);
//  EXPECT_EQ(6 , AL::callFunctor<int>(functor, 1, 2, 3));
//  functor = AL::makeFunctor(&foo, &Foo::fun4);
//  EXPECT_EQ(10, AL::callFunctor<int>(functor, 1, 2, 3, 4));
//  functor = AL::makeFunctor(&foo, &Foo::fun5);
//  EXPECT_EQ(15, AL::callFunctor<int>(functor, 1, 2, 3, 4, 5));
//  functor = AL::makeFunctor(&foo, &Foo::fun6);
//  EXPECT_EQ(21, AL::callFunctor<int>(functor, 1, 2, 3, 4, 5, 6));
}
