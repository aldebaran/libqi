/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <alcommon-ng/functor/functionsignature.hpp>
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

TEST(TestSignature, BasicTypeSignature) {
  EXPECT_EQ("b",    AL::typeSignatureWithCopy<bool>::value());
  EXPECT_EQ("i",    AL::typeSignatureWithCopy<int>::value());
  EXPECT_EQ("f",    AL::typeSignatureWithCopy<float>::value());
  EXPECT_EQ("d",    AL::typeSignatureWithCopy<double>::value());
  EXPECT_EQ("s",    AL::typeSignatureWithCopy<std::string>::value());
  EXPECT_EQ("[i]",  AL::typeSignatureWithCopy< std::vector<int> >::value());
  typedef std::map<int,int> MapInt;
  EXPECT_EQ("{ii}", AL::typeSignatureWithCopy< MapInt >::value() );

  EXPECT_EQ("b#",    AL::typeSignatureWithCopy<const bool>::value());
  EXPECT_EQ("i#",    AL::typeSignatureWithCopy<const int>::value());
  EXPECT_EQ("f#",    AL::typeSignatureWithCopy<const float>::value());
  EXPECT_EQ("d#",    AL::typeSignatureWithCopy<const double>::value());
  EXPECT_EQ("s#",    AL::typeSignatureWithCopy<const std::string>::value());
  EXPECT_EQ("[i]#",  AL::typeSignatureWithCopy<const std::vector< int > >::value());
  EXPECT_EQ("{ii}#", AL::typeSignatureWithCopy<const MapInt >::value());

  EXPECT_EQ("b#*",   AL::typeSignatureWithCopy<const bool*>::value());
  EXPECT_EQ("i#*",   AL::typeSignatureWithCopy<const int*>::value());
  EXPECT_EQ("f#*",   AL::typeSignatureWithCopy<const float*>::value());
  EXPECT_EQ("d#*",   AL::typeSignatureWithCopy<const double*>::value());
  EXPECT_EQ("s#*",   AL::typeSignatureWithCopy<const std::string*>::value());
  EXPECT_EQ("[i]#*", AL::typeSignatureWithCopy<const std::vector< int >* >::value());
  EXPECT_EQ("{ii}#*",AL::typeSignatureWithCopy<const MapInt* >::value());

  EXPECT_EQ("b#&",    AL::typeSignatureWithCopy<const bool&>::value());
  EXPECT_EQ("i#&",    AL::typeSignatureWithCopy<const int&>::value());
  EXPECT_EQ("f#&",    AL::typeSignatureWithCopy<const float&>::value());
  EXPECT_EQ("d#&",    AL::typeSignatureWithCopy<const double&>::value());
  EXPECT_EQ("s#&",    AL::typeSignatureWithCopy<const std::string&>::value());
  EXPECT_EQ("[i]#&",  AL::typeSignatureWithCopy<const std::vector< int >& >::value());
  EXPECT_EQ("{ii}#&", AL::typeSignatureWithCopy<const MapInt& >::value());

  //ERROR
  EXPECT_EQ("UNKNOWN", AL::typeSignatureWithCopy<short>::value());
}
TEST(TestSignature, ComplexTypeSignature) {
  typedef std::map<int,int> MapInt;
  //{ii}
  typedef std::map<MapInt,MapInt> MapInt2;
  //{{ii}{ii}}
  typedef std::map<std::vector<MapInt2>, std::vector<const std::vector<MapInt2&> > > FuckinMap;
  //{[{{ii}{ii}}][[{{ii}{ii}}&]#]}
  //and obama said: Yes We Can!

  EXPECT_EQ("{[{{ii}{ii}}][[{{ii}{ii}}&]#]}"      , AL::typeSignatureWithCopy<FuckinMap>::value());

}

TEST(TestSignature, BasicVoidFunctionSignature) {
  EXPECT_EQ("v:"      , AL::functionSignature(&vfun0));
  EXPECT_EQ("v:i"     , AL::functionSignature(&vfun1));
  EXPECT_EQ("v:ii"    , AL::functionSignature(&vfun2));
  EXPECT_EQ("v:iii"   , AL::functionSignature(&vfun3));
  EXPECT_EQ("v:iiii"  , AL::functionSignature(&vfun4));
  EXPECT_EQ("v:iiiii" , AL::functionSignature(&vfun5));
  EXPECT_EQ("v:iiiiii", AL::functionSignature(&vfun6));
}

TEST(TestSignature, BasicFunctionSignature) {
  EXPECT_EQ("i:"      , AL::functionSignature(&fun0));
  EXPECT_EQ("i:i"     , AL::functionSignature(&fun1));
  EXPECT_EQ("i:ii"    , AL::functionSignature(&fun2));
  EXPECT_EQ("i:iii"   , AL::functionSignature(&fun3));
  EXPECT_EQ("i:iiii"  , AL::functionSignature(&fun4));
  EXPECT_EQ("i:iiiii" , AL::functionSignature(&fun5));
  EXPECT_EQ("i:iiiiii", AL::functionSignature(&fun6));
}

TEST(TestSignature, BasicVoidMemberSignature) {
  Foo foo;
  EXPECT_EQ("v:"      , AL::functionSignature(&foo, &Foo::vfun0));
  EXPECT_EQ("v:i"     , AL::functionSignature(&foo, &Foo::vfun1));
  EXPECT_EQ("v:ii"    , AL::functionSignature(&foo, &Foo::vfun2));
  EXPECT_EQ("v:iii"   , AL::functionSignature(&foo, &Foo::vfun3));
  EXPECT_EQ("v:iiii"  , AL::functionSignature(&foo, &Foo::vfun4));
  EXPECT_EQ("v:iiiii" , AL::functionSignature(&foo, &Foo::vfun5));
  EXPECT_EQ("v:iiiiii", AL::functionSignature(&foo, &Foo::vfun6));
}

TEST(TestSignature, BasicMemberSignature) {
  Foo foo;
  EXPECT_EQ("i:"      , AL::functionSignature(&foo, &Foo::fun0));
  EXPECT_EQ("i:i"     , AL::functionSignature(&foo, &Foo::fun1));
  EXPECT_EQ("i:ii"    , AL::functionSignature(&foo, &Foo::fun2));
  EXPECT_EQ("i:iii"   , AL::functionSignature(&foo, &Foo::fun3));
  EXPECT_EQ("i:iiii"  , AL::functionSignature(&foo, &Foo::fun4));
  EXPECT_EQ("i:iiiii" , AL::functionSignature(&foo, &Foo::fun5));
  EXPECT_EQ("i:iiiiii", AL::functionSignature(&foo, &Foo::fun6));
}
