/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/

#ifdef _MSC_VER
   // C4503 decorated name length exceeded, name was truncated
   // The only workaround is to make structs to hide the template complexity
   // We don't want to have to do that
#  pragma warning( disable: 4503 )
#endif

#include <gtest/gtest.h>
#define __QI_SIGNATURE_UNKNOWN_INSTEAD_OF_ASSERT

#include <qitype/signature.hpp>
#include <qitype/genericvalue.hpp>

#include <vector>
#include <map>

//#include "alvalue.pb.h"

static const int gLoopCount = 1000000;

namespace qi
{
  // OLD API compat layer for this test.
  template<typename T> struct signatureFromType
  {
    static std::string value()
    {
      return typeOf<T>()->signature();
    }
  };
  struct signatureFromObject
  {
    template<typename T> static std::string value(const T& ptr)
    {
      return typeOf(ptr)->signature();
    }
  };
}
class noSigForThis;

typedef std::map<int,int> MapInt;

TEST(TestSignature, BasicTypeSignature) {
  EXPECT_EQ("b",    qi::signatureFromType<bool>::value());
  EXPECT_EQ("c",    qi::signatureFromType<char>::value());
  EXPECT_EQ("C",    qi::signatureFromType<unsigned char>::value());
  EXPECT_EQ("w",    qi::signatureFromType<short>::value());
  EXPECT_EQ("W",    qi::signatureFromType<unsigned short>::value());
  EXPECT_EQ("i",    qi::signatureFromType<int>::value());
  EXPECT_EQ("I",    qi::signatureFromType<unsigned int>::value());
  EXPECT_EQ("l",    qi::signatureFromType<long>::value());
  EXPECT_EQ("L",    qi::signatureFromType<unsigned long>::value());

  EXPECT_EQ("c",    qi::signatureFromType<qi::int8_t>::value());
  EXPECT_EQ("C",    qi::signatureFromType<qi::uint8_t>::value());
  EXPECT_EQ("w",    qi::signatureFromType<qi::int16_t>::value());
  EXPECT_EQ("W",    qi::signatureFromType<qi::uint16_t>::value());
  EXPECT_EQ("i",    qi::signatureFromType<qi::int32_t>::value());
  EXPECT_EQ("I",    qi::signatureFromType<qi::uint32_t>::value());
  EXPECT_EQ("l",    qi::signatureFromType<qi::int64_t>::value());
  EXPECT_EQ("L",    qi::signatureFromType<qi::uint64_t>::value());

  EXPECT_EQ("c",    qi::signatureFromType<char>::value());
  EXPECT_EQ("C",    qi::signatureFromType<unsigned char>::value());
  EXPECT_EQ("w",    qi::signatureFromType<short>::value());
  EXPECT_EQ("W",    qi::signatureFromType<unsigned short>::value());
  EXPECT_EQ("i",    qi::signatureFromType<int>::value());
  EXPECT_EQ("I",    qi::signatureFromType<unsigned int>::value());
  EXPECT_EQ("l",    qi::signatureFromType<long>::value());
  EXPECT_EQ("L",    qi::signatureFromType<unsigned long>::value());
  EXPECT_EQ("l",    qi::signatureFromType<long long>::value());
  EXPECT_EQ("L",    qi::signatureFromType<unsigned long long>::value());

  EXPECT_EQ("f",    qi::signatureFromType<float>::value());
  EXPECT_EQ("d",    qi::signatureFromType<double>::value());
  EXPECT_EQ("s",    qi::signatureFromType<std::string>::value());
  EXPECT_EQ("[i]",  qi::signatureFromType< std::vector<int> >::value());
  EXPECT_EQ("{ii}", qi::signatureFromType< MapInt >::value() );
}

TEST(TestSignature, TypeConstRefPointerMix) {

  EXPECT_EQ("b",    qi::signatureFromType<bool>::value());
  EXPECT_EQ("c",    qi::signatureFromType<char>::value());
  EXPECT_EQ("i",    qi::signatureFromType<int>::value());
  EXPECT_EQ("f",    qi::signatureFromType<float>::value());
  EXPECT_EQ("d",    qi::signatureFromType<double>::value());
  EXPECT_EQ("s",    qi::signatureFromType<std::string>::value());
  EXPECT_EQ("[i]",  qi::signatureFromType< std::vector<int> >::value());
  EXPECT_EQ("{ii}", qi::signatureFromType< MapInt >::value() );

  EXPECT_EQ("b",    qi::signatureFromType<const bool>::value());
  EXPECT_EQ("c",    qi::signatureFromType<const char>::value());
  EXPECT_EQ("i",    qi::signatureFromType<const int>::value());
  EXPECT_EQ("f",    qi::signatureFromType<const float>::value());
  EXPECT_EQ("d",    qi::signatureFromType<const double>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const std::string>::value());
  EXPECT_EQ("[i]",  qi::signatureFromType<const std::vector< int > >::value());
  EXPECT_EQ("{ii}", qi::signatureFromType<const MapInt >::value());

  EXPECT_EQ("b",    qi::signatureFromType<const bool&>::value());
  EXPECT_EQ("c",    qi::signatureFromType<const char&>::value());
  EXPECT_EQ("i",    qi::signatureFromType<const int&>::value());
  EXPECT_EQ("f",    qi::signatureFromType<const float&>::value());
  EXPECT_EQ("d",    qi::signatureFromType<const double&>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const std::string&>::value());
  EXPECT_EQ("[i]",  qi::signatureFromType<const std::vector< int >& >::value());
  EXPECT_EQ("{ii}", qi::signatureFromType<const MapInt& >::value());
}

TEST(TestSignature, Bools) {
  EXPECT_EQ("b",    qi::signatureFromType<bool>::value());
  EXPECT_EQ("b",    qi::signatureFromType<bool&>::value());
  EXPECT_EQ("b",    qi::signatureFromType<const bool>::value());
  EXPECT_EQ("b",    qi::signatureFromType<const bool&>::value());
}

TEST(TestSignature, Strings) {
  EXPECT_EQ("s",    qi::signatureFromType<std::string>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const std::string>::value());

  EXPECT_EQ("s",    qi::signatureFromType<char *>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const char *>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const char * const>::value());
  EXPECT_EQ("s",    qi::signatureFromType<char const * const>::value());
  EXPECT_EQ("s",    qi::signatureFromType<char *&>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const char *&>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const char * const&>::value());
  EXPECT_EQ("s",    qi::signatureFromType<char const * const&>::value());
}

TEST(TestSignature, ComplexTypeSignature) {
  //{ii}
  typedef std::map<int,int> MapInt;
  //{{ii}{ii}}
  typedef std::map<MapInt,MapInt>        MapInt2;
  typedef std::vector<MapInt2>           VectorMapInt2;

  // MapInt2& Does not works
  typedef MapInt2                        MapInt2Ref;

  typedef std::vector<MapInt2Ref>        VectMapInt2Ref;

  //const VectMapInt2Ref does not works
  typedef VectMapInt2Ref                 VectMapInt2RefConst;

  typedef std::vector< VectMapInt2RefConst > VectVectMapInt2ConstRef;

  typedef std::map<VectorMapInt2, VectVectMapInt2ConstRef> FuckinMap;

  //{[{{ii}{ii}}][[{{ii}{ii}}&]#]}
  //and obama said: Yes We Can!
  EXPECT_EQ("{ii}"                        , qi::typeOf<MapInt>()->signature());
  EXPECT_EQ("{{ii}{ii}}"                  , qi::typeOf<MapInt2>()->signature());
  EXPECT_EQ("[{{ii}{ii}}]"                , qi::typeOf<VectorMapInt2>()->signature());
  EXPECT_EQ("{{ii}{ii}}"                  , qi::typeOf<MapInt2Ref>()->signature());
  EXPECT_EQ("[{{ii}{ii}}]"                , qi::typeOf<VectMapInt2Ref>()->signature());
  EXPECT_EQ("[{{ii}{ii}}]"                , qi::typeOf<VectMapInt2RefConst>()->signature());
  EXPECT_EQ("[[{{ii}{ii}}]]"              , qi::typeOf<VectVectMapInt2ConstRef>()->signature());
  EXPECT_EQ("{[{{ii}{ii}}][[{{ii}{ii}}]]}", qi::typeOf<FuckinMap>()->signature());
}

TEST(TestSignature, FromObject) {
  int myint;
  EXPECT_EQ("i"     , qi::signatureFromObject::value(myint));
}

TEST(TestSignature, ComplexConstRefPtr) {
  EXPECT_EQ("f",    qi::signatureFromType<float>::value());
  EXPECT_EQ("f",    qi::signatureFromType<float&>::value());
  EXPECT_EQ("f",    qi::signatureFromType<const float &>::value());
  EXPECT_EQ("f",    qi::signatureFromType<const float>::value());
}




//expect that the following test to do not build. (static assert)
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



TEST(TestSignature, WeirdPointerTypes) {
  EXPECT_EQ("X", qi::signatureFromType<bool*>::value());
  EXPECT_EQ("X", qi::signatureFromType<bool*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const bool*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const bool*&>::value());

  EXPECT_EQ("X", qi::signatureFromType<float *>::value());
  EXPECT_EQ("X", qi::signatureFromType<float const *>::value());
  EXPECT_EQ("X", qi::signatureFromType<float * const>::value());
  EXPECT_EQ("X", qi::signatureFromType<float const * const>::value());
}
//not instanciable
//TEST(TestSignature, noSignature) {
//  EXPECT_EQ("X", qi::signatureFromType<noSigForThis>::value());
//  EXPECT_EQ("X", qi::signatureFromType<void (int, int)>::value());
//}

TEST(TestSignature, WeirdBasicFunctionSignature) {
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun6));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun6));
}

TEST(TestSignature, WeirdPointers) {
  EXPECT_EQ("X", qi::signatureFromType<const bool*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const int*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const float*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const double*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const std::string*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const std::vector< int >* >::value());
  EXPECT_EQ("X", qi::signatureFromType<const MapInt* >::value());

  EXPECT_EQ("X", qi::signatureFromType<const bool*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const int*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const float*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const double*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const std::string*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const std::vector< int >*& >::value());
  EXPECT_EQ("X", qi::signatureFromType<const MapInt*& >::value());
}


TEST(TestSignature, FunctionType) {
  EXPECT_EQ("X", qi::signatureFromType<void (*)(int, int)>::value());
}

TEST(TestSignature, BasicVoidFunctionSignature) {
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun6));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun6));

}

//dont ask me why, but member function evaluates to bool :(
TEST(TestSignature, WeirdBasicVoidMemberSignature) {
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun6));
}

TEST(TestSignature, WeirdBasicMemberSignature) {
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun6));
}

//yes shared_ptr evaluate to bool
TEST(TestSignature, WeirdSharedPtr) {
  EXPECT_EQ("X", qi::signatureFromType< boost::shared_ptr<int> >::value());
  EXPECT_EQ("X", qi::signatureFromType< boost::shared_ptr<int> &>::value());
}


//#endif
