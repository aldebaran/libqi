/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/

#include <ka/macro.hpp>

// C4503 decorated name length exceeded, name was truncated.
// The only workaround is to make structs to hide the template complexity.
// We don't want to have to do that.
KA_WARNING_DISABLE(4503, )

#include <gtest/gtest.h>
#define __QI_SIGNATURE_UNKNOWN_INSTEAD_OF_ASSERT

#include <qi/signature.hpp>
#include <qi/anyvalue.hpp>
#include <qi/jsoncodec.hpp>

#include <vector>
#include <map>

TEST(TestSignature, SignatureSize)
{
  qi::Signature s("(iiii)");
  EXPECT_EQ(4u, s.children().size());
  EXPECT_EQ(0u, s.children().at(0).children().size());

  s = qi::Signature("[i]");
  EXPECT_EQ(1u, s.children().size());

  s = qi::Signature("{ii}");
  EXPECT_EQ(2u, s.children().size());
}

TEST(TestSignature, VArgs)
{
  //vargs
  EXPECT_TRUE(qi::Signature("#b").isValid());
  EXPECT_TRUE(qi::Signature("#b<titi,toto>").isValid());

  //kwargs
  EXPECT_TRUE(qi::Signature("~b").isValid());
  EXPECT_TRUE(qi::Signature("~b<titi,toto>").isValid());
}

struct MPoint
{
  MPoint(std::int32_t x=0, std::int32_t y=0)
    : x(x)
    , y(y)
  {}
  std::int32_t x;
  std::int32_t y;
};
QI_TYPE_STRUCT(MPoint, x, y);

TEST(TestSignature, Equal)
{
  EXPECT_TRUE(qi::Signature("[s]") == qi::Signature("[s]"));
  EXPECT_TRUE(qi::Signature("(ss)<Point,x,y>") == qi::Signature("(ss)")); // really?

  EXPECT_TRUE(qi::Signature("(mm)") != "(mmm)");
  EXPECT_TRUE(qi::Signature("(mm)") != "(m)");
}

TEST(TestSignature, InvalidSignature)
{
  //empty signature are invalid
  EXPECT_THROW(qi::Signature(""), std::runtime_error);
  EXPECT_THROW(qi::Signature("("), std::runtime_error);
  EXPECT_THROW(qi::Signature("{"), std::runtime_error);
  EXPECT_THROW(qi::Signature("["), std::runtime_error);
  EXPECT_THROW(qi::Signature("plafbim"), std::runtime_error);
  EXPECT_THROW(qi::Signature("(m)(sib)"), std::runtime_error);
  EXPECT_THROW(qi::Signature("ddd"), std::runtime_error);
  EXPECT_THROW(qi::Signature("(mm"), std::runtime_error);
}

TEST(TestSignature, InvalidNumberOfArgs)
{
  //empty signature are invalid
  EXPECT_THROW(qi::Signature("[iii]"), std::runtime_error);
  EXPECT_THROW(qi::Signature("{iii}"), std::runtime_error);
  EXPECT_THROW(qi::Signature("{i}"), std::runtime_error);
}

TEST(TestSignature, FromString)
{
  qi::Signature *sig;

  sig = new qi::Signature();
  EXPECT_FALSE(sig->isValid());
  delete sig;


  sig = new qi::Signature("(s)");
  EXPECT_TRUE(sig->isValid());
  delete sig;

  sig = new qi::Signature("()");
  EXPECT_TRUE(sig->isValid());
  delete sig;

  sig = new qi::Signature("()<>");
  EXPECT_TRUE(sig->isValid());
  delete sig;

  ASSERT_NO_THROW(sig = new qi::Signature("(((sIsI[(sssW)]s)))"));
  EXPECT_TRUE(sig->isValid());
  delete sig;

  ASSERT_NO_THROW(sig = new qi::Signature("({I(Isss[(ss)]s)}{I(Is)}{I(Iss)}s)"));
  EXPECT_TRUE(sig->isValid());
  delete sig;
}

TEST(TestSignature, SignatureSplitError)
{
  std::vector<std::string> sigInfo;

  sigInfo = qi::signatureSplit("reply");
  EXPECT_EQ("", sigInfo[0]);
  EXPECT_EQ("reply", sigInfo[1]);
  EXPECT_EQ("", sigInfo[2]);

  EXPECT_ANY_THROW(qi::signatureSplit("reply::"));
  EXPECT_ANY_THROW(qi::signatureSplit("reply::("));
  EXPECT_ANY_THROW(qi::signatureSplit("reply::e"));

  EXPECT_ANY_THROW(qi::signatureSplit("reply::(RRRR)"));
  EXPECT_NO_THROW(qi::signatureSplit("reply::(sssss)"));

  EXPECT_ANY_THROW(qi::signatureSplit("reply::([sss])"));
  EXPECT_NO_THROW(qi::signatureSplit("reply::([s])"));

  EXPECT_ANY_THROW(qi::signatureSplit("titi::s([[xxx]])"));
  EXPECT_ANY_THROW(qi::signatureSplit("titi::s([[sss]])"));
  EXPECT_NO_THROW(qi::signatureSplit("titi::s([[s]])"));

  EXPECT_ANY_THROW(qi::signatureSplit("titi::s([({ii}s[ss])])"));
  EXPECT_NO_THROW(qi::signatureSplit("titi::s([({ii}s[s])])"));

  EXPECT_NO_THROW(qi::signatureSplit("titi::s({i{is}})"));
}

TEST(TestSignature, IsCompatible)
{
  qi::Signature s("(s[m])");
  EXPECT_EQ(s.isConvertibleTo("(si)"), 0.);
  EXPECT_EQ(s.isConvertibleTo("(sf)"), 0.);
  EXPECT_EQ(s.isConvertibleTo("(ss)"), 0.);
  EXPECT_GT(s.isConvertibleTo("(sm)"), 0.);

  qi::Signature s2("(s)");
  EXPECT_EQ(s2.isConvertibleTo("(si)"), 0.);
  EXPECT_GT(s2.isConvertibleTo("(m)"), 0.);

  qi::Signature s3("([s])");
  EXPECT_GT(s3.isConvertibleTo("([m])"), s3.isConvertibleTo("(m)"));
}

TEST(TestSignature, SignatureSplit)
{
  std::vector<std::string> sigInfo;

  ASSERT_NO_THROW(sigInfo = qi::signatureSplit("reply::(s)"));
  EXPECT_EQ("", sigInfo[0]);
  EXPECT_EQ("reply", sigInfo[1]);
  EXPECT_EQ("(s)", sigInfo[2]);

  ASSERT_NO_THROW(sigInfo = qi::signatureSplit("reply::s(s)"));
  EXPECT_EQ("s", sigInfo[0]);
  EXPECT_EQ("reply", sigInfo[1]);
  EXPECT_EQ("(s)", sigInfo[2]);


  ASSERT_NO_THROW(sigInfo = qi::signatureSplit("info::(m)(sib)"));
  EXPECT_EQ("(m)", sigInfo[0]);
  EXPECT_EQ("info", sigInfo[1]);
  EXPECT_EQ("(sib)", sigInfo[2]);

  ASSERT_NO_THROW(sigInfo = qi::signatureSplit("toto::((([{ii}])))({[{{ii}{ii}}][[{{ii}{ii}}]]})"));
  EXPECT_EQ("((([{ii}])))", sigInfo[0]);
  EXPECT_EQ("toto", sigInfo[1]);
  EXPECT_EQ("({[{{ii}{ii}}][[{{ii}{ii}}]]})", sigInfo[2]);

  ASSERT_NO_THROW(sigInfo = qi::signatureSplit("toto"));
  EXPECT_EQ("", sigInfo[0]);
  EXPECT_EQ("toto", sigInfo[1]);
  EXPECT_EQ("", sigInfo[2]);
}

TEST(TestSignature, ItAnnotation)
{
  std::string orig("((m)<Plouf,x>(scf)<Point3d,x,y,z>)");
  qi::Signature sig(orig);
  qi::SignatureVector subsig = sig.children();

  EXPECT_EQ(orig, sig.toString());
  EXPECT_EQ("(m)<Plouf,x>", subsig[0].toString());
  EXPECT_EQ("(scf)<Point3d,x,y,z>", subsig[1].toString());
}

TEST(TestSignature, TestToSTLType)
{
  qi::Signature sig("(s)");
  EXPECT_EQ("(String)", sig.toPrettySignature());

  qi::Signature sig2("(slb)");
  EXPECT_EQ("(String,Int64,Bool)", sig2.toPrettySignature());

  qi::Signature sig3("{is}");
  EXPECT_EQ("Map<Int32,String>", sig3.toPrettySignature());

  qi::Signature sig4("((m)(scf))");
  EXPECT_EQ("((Value),(String,Int8,Float))", sig4.toPrettySignature());

  qi::Signature sig5("((m)<Plouf,x>(scf)<Point3d,x,y,z>)");
  EXPECT_EQ("(Plouf,Point3d)", sig5.toPrettySignature());
}

//expect that the following test to do not build. (static assert)
static int gGlobalResult = 0;

void vfun0() { gGlobalResult = 0; }
void vfun1(const int &p0) { gGlobalResult = p0; }
void vfun2(const int &p0,const int &p1) { gGlobalResult = p0 + p1; }
void vfun3(const int &p0,const int &p1,const int &p2) { gGlobalResult = p0 + p1 + p2; }
void vfun4(const int &p0,const int &p1,const int &p2,const int &p3) { gGlobalResult = p0 + p1 + p2 + p3; }
void vfun5(const int &p0,const int &p1,const int &p2,const int &p3,const int &p4)
{ gGlobalResult = p0 + p1 + p2 + p3 + p4; }
void vfun6(const int &p0,const int &p1,const int &p2,const int &p3,const int &p4,const int &p5)
{ gGlobalResult = p0 + p1 + p2 + p3 + p4 + p5; }

int fun0() { return 0; }
int fun1(const int &p0) { return p0; }
int fun2(const int &p0,const int &p1) { return p0 + p1; }
int fun3(const int &p0,const int &p1,const int &p2) { return p0 + p1 + p2; }
int fun4(const int &p0,const int &p1,const int &p2,const int &p3) { return p0 + p1 + p2 + p3; }
int fun5(const int &p0,const int &p1,const int &p2,const int &p3,const int &p4)
{ return p0 + p1 + p2 + p3 + p4; }
int fun6(const int &p0,const int &p1,const int &p2,const int &p3,const int &p4,const  int &p5)
{ return p0 + p1 + p2 + p3 + p4 + p5; }


struct Foo {
  void voidCall() { return; }
  int intStringCall(const std::string &plouf) { return plouf.size(); }

  int fun0() { return 0; }
  int fun1(const int &p0) { return p0; }
  int fun2(const int &p0,const int &p1) { return p0 + p1; }
  int fun3(const int &p0,const int &p1,const int &p2) { return p0 + p1 + p2; }
  int fun4(const int &p0,const int &p1,const int &p2,const int &p3) { return p0 + p1 + p2 + p3; }
  int fun5(const int &p0,const int &p1,const int &p2,const int &p3,const int &p4)
  { return p0 + p1 + p2 + p3 + p4; }
  int fun6(const int &p0,const int &p1,const int &p2,const int &p3,const int &p4,const int &p5)
  { return p0 + p1 + p2 + p3 + p4 + p5; }

  void vfun0() { gGlobalResult = 0; }
  void vfun1(const int &p0) { gGlobalResult = p0; }
  void vfun2(const int &p0,const int &p1) { gGlobalResult = p0 + p1; }
  void vfun3(const int &p0,const int &p1,const int &p2) { gGlobalResult = p0 + p1 + p2; }
  void vfun4(const int &p0,const int &p1,const int &p2,const int &p3) { gGlobalResult = p0 + p1 + p2 + p3; }
  void vfun5(const int &p0,const int &p1,const int &p2,const int &p3,const int &p4)
  { gGlobalResult = p0 + p1 + p2 + p3 + p4; }
  void vfun6(const int &p0,const int &p1,const int &p2,const int &p3,const int &p4,const int &p5)
  { gGlobalResult = p0 + p1 + p2 + p3 + p4 + p5; }
};

qiLogCategory("test.signature()");

void stackIt(std::vector<qi::Signature>& stack, const qi::Signature& sig)
{
  qiLogInfo() << "Pushing:" << sig.toString() << ", annot:" << sig.annotation();
  qi::SignatureVector child = sig.children();
  for (qi::SignatureVector::const_reverse_iterator it = child.rbegin(); it != child.rend(); ++it) {
    stackIt(stack, *it);
  }
  stack.push_back(sig);
}

class SignatureValidator
{
public:

  SignatureValidator(qi::Signature s)
  : good(true), count(0)
  {
    stackIt(stack, s);
  }

  SignatureValidator& operator()(char t, const char* annotation)
  {
    if (!good)
      return *this;
    if (stack.empty())
    {
      qiLogError() << "No more elements after " << count;
      good = false;
      return *this;
    }
    qi::Signature& schi = stack.back();
    if (schi.type() != t)
    {
      qiLogError() << "Type mismatch " << t << " " << schi.type();
      good = false;
      return *this;
    }
    if (schi.annotation() != annotation)
    {
      qiLogError() << "Annotation mismatch " << annotation << " vs " << schi.annotation();
      good = false;
      return *this;
    }
    stack.pop_back();
    return *this;
  }

  operator bool() const
  {
    if (!stack.empty())
    {
      qiLogError() << "Remaining elements at the end";
      good = false;
    }
    return good;
  }

  std::vector<qi::Signature> stack;
  mutable bool good;
  int count;
};


qi::Signature tuple(const std::string& str)
{
  return qi::Signature("(" + str + ")");
}

TEST(TestSignature, Annotation)
{
  using qi::Signature;
  Signature s("i<foo>");
  EXPECT_EQ("foo", s.annotation());
  EXPECT_EQ('i', s.type());

  s = Signature("i<foo<bar><<baz@5'\"[]><>a>>");
  EXPECT_EQ('i', s.type());
  EXPECT_EQ("foo<bar><<baz@5'\"[]><>a>", s.annotation());

  s = Signature("(ii<foo>i<bar>ii<bam<baz>>)");
  qiLogInfo() << "sig: " << s.toString();
  EXPECT_EQ(5u, s.children().size());
  EXPECT_TRUE(
    SignatureValidator(s)
    ('(', "")
    ('i', "")
    ('i', "foo")
    ('i', "bar")
    ('i', "")
    ('i', "bam<baz>")
    );
  EXPECT_TRUE(SignatureValidator(tuple("i<foo>[i<bar>]<baz>{i<bim>I<bam>}<boum>(i<pif>I<paf>)<pouf>"))
    ('(', "")
    ('i', "foo")
    ('[', "baz")
    ('i', "bar")
    ('{', "boum")
    ('i', "bim")
    ('I', "bam")
    ('(', "pouf")
    ('i', "pif")
    ('I', "paf")
    );
 EXPECT_TRUE(SignatureValidator("[[[[i<a>]<b>]<c>]<d>]<e>")
   ('[', "e")
   ('[', "d")
   ('[', "c")
   ('[', "b")
   ('i', "a")
 );
 // Test the test-suite tool
 EXPECT_FALSE(SignatureValidator("i")('I', ""));
 EXPECT_FALSE(SignatureValidator("i"));
 EXPECT_FALSE(SignatureValidator(tuple("ii"))('i', ""));
 EXPECT_FALSE(SignatureValidator("i")('i', "coin"));
 EXPECT_FALSE(SignatureValidator("i<coin>")('i', ""));
}

TEST(TestSignature, AnnotationInvalid)
{
  using qi::Signature;
  EXPECT_THROW(Signature("i<foo")  , std::runtime_error);
  EXPECT_THROW(Signature("ifoo>")  , std::runtime_error);
  EXPECT_THROW(Signature("[ifoo>]"), std::runtime_error);
  EXPECT_THROW(Signature("[i]>")   , std::runtime_error);
  EXPECT_THROW(Signature("<>")     , std::runtime_error);
  EXPECT_THROW(Signature(">")      , std::runtime_error);
  EXPECT_THROW(Signature("<")      , std::runtime_error);
}

struct Point
{
  double x;
  double y;
  std::string name;
};
QI_TYPE_STRUCT(Point, x, y, name);

TEST(TestSignature, AnnotationStruct)
{
  EXPECT_EQ("(dds)<Point,x,y,name>", qi::typeOf<Point>()->signature().toString());
}

TEST(TestSignature, AnnotatedStructCompatibleWithStringStringMap)
{
  qi::Signature s("(s)<Phrase,text>");
  EXPECT_EQ(0., s.isConvertibleTo("{ss}"));
}

TEST(TestSignature, AnnotatedStructCompatibleWithStringIntMap)
{
  qi::Signature s("(ii)<Locale,language,region>");
  EXPECT_EQ(0., s.isConvertibleTo("{si}"));
}

TEST(TestSignature, SeveralAnnotatedStructsCompatibleWithSeveralMaps)
{
  qi::Signature s("((s)<Phrase,text>(ii)<Locale,language,region>)");
  EXPECT_EQ(0., s.isConvertibleTo("({ss}{si})"));
}

TEST(TestSignature, ObjectAndSeveralAnnotatedStructsCompatibleWithObjectAndSeveralMaps)
{
  qi::Signature s("(o(s)<Phrase,text>(ii)<Locale,language,region>)");
  EXPECT_EQ(0., s.isConvertibleTo("(o{ss}{si})"));
}

std::string trimall(const std::string& s)
{
  std::string res;
  for (unsigned i=0; i<s.size(); ++i)
    if (s[i] != ' ')
      res += s[i];
  return res;
}

std::string unarmor(const std::string& s)
{
  return trimall(s.substr(1, s.size()-2));
}

TEST(TestSignature, ToData)
{
  #define j(a) trimall(qi::encodeJSON(qi::Signature(a).toData()))
  #define S(a) unarmor(#a)
  EXPECT_EQ(j("i"),             S(([ "i", [], "" ])));
  EXPECT_EQ(j("i<foo>"),        S(([ "i", [], "foo" ])));
  EXPECT_EQ(j("[i<foo>]<bar>"), S((["[", [[ "i", [], "foo" ]], "bar"])));
  EXPECT_EQ(j("(i[f]{if})"),
    S(( ["(", [["i", [], ""],["[", [["f", [], ""]], ""],["{", [["i", [], ""], ["f", [], ""]], "" ]], "" ])));
}


#define verif_iter(_it, _sig, _type, _hasChildren) \
{\
  EXPECT_STREQ(_sig, (_it).toString().c_str());\
  EXPECT_TRUE(_hasChildren == (_it).hasChildren()); \
  EXPECT_EQ(qi::Signature::_type, (_it).type());\
}


TEST(TestSignatureIterator, Simple)
{
  qi::SignatureVector::iterator it;

  qi::Signature signature("(is)");
  qi::SignatureVector sig = signature.children();

  EXPECT_TRUE(signature.isValid());
  EXPECT_TRUE(sig.size() == 2);
  it = sig.begin();
  verif_iter(*it, "i", Type_Int32, false);
  ++it;
  verif_iter(*it, "s", Type_String, false);
}

TEST(TestSignatureIterator, Empty)
{
  qi::Signature sig3;

  EXPECT_TRUE(sig3.children().size() == 0);
  verif_iter(sig3, "", Type_None, false);
  EXPECT_STREQ("", sig3.toString().c_str());
}

//{ii}
using MapInt = std::map<std::int32_t, std::int32_t>;

//{{ii}{ii}}
using MapInt2 = std::map<MapInt, MapInt>;

using VectorMapInt2 = std::vector<MapInt2>;
using VectMapInt2 = std::vector<MapInt2>;
using VectVectMapInt2 = std::vector<VectMapInt2>;

//{[{{ii}{ii}}][[{{ii}{ii}}&]#]}
using ComplexMap = std::map<VectorMapInt2, VectVectMapInt2>;

class TestSignatureWithTypeToString
  : public ::testing::TestWithParam<std::pair<qi::TypeInterface* const, const char*>>
{};

// Equivalent types alternatives.
#define EQ_TYPES_ALTS(T, S) \
  { qi::typeOf<T>(),        S }, \
  { qi::typeOf<T&>(),       S }, \
  { qi::typeOf<const T>(),  S }, \
  { qi::typeOf<const T&>(), S }

// Equivalent pointers alternatives.
#define EQ_PTRS_ALTS(P, S) \
  EQ_TYPES_ALTS(P*, S),      \
  EQ_TYPES_ALTS(P* const, S) \

INSTANTIATE_TEST_SUITE_P(
  MostTypes,
  TestSignatureWithTypeToString,
  ::testing::ValuesIn(
    std::map<qi::TypeInterface*, const char*> {
      // Integral types
      EQ_TYPES_ALTS(bool,                      "b"),
      EQ_TYPES_ALTS(std::int8_t,               "c"),
      EQ_TYPES_ALTS(std::uint8_t,              "C"),
      EQ_TYPES_ALTS(std::int16_t,              "w"),
      EQ_TYPES_ALTS(std::uint16_t,             "W"),
      EQ_TYPES_ALTS(std::int32_t,              "i"),
      EQ_TYPES_ALTS(std::uint32_t,             "I"),
      EQ_TYPES_ALTS(std::int64_t,              "l"),
      EQ_TYPES_ALTS(std::uint64_t,             "L"),

      // Float types
      EQ_TYPES_ALTS(float,                     "f"),
      EQ_TYPES_ALTS(double,                    "d"),

      // Strings
      EQ_TYPES_ALTS(std::string,               "s"),
      EQ_PTRS_ALTS(char,                       "s"),

      // Containers
      EQ_TYPES_ALTS(std::vector<std::int32_t>, "[i]"),
      EQ_TYPES_ALTS(qi::AnyVarArguments,       "#m"),
      EQ_TYPES_ALTS(MPoint,                    "(ii)<MPoint,x,y>"),
      EQ_TYPES_ALTS(MapInt,                    "{ii}"),
      EQ_TYPES_ALTS(MapInt2,                   "{{ii}{ii}}"),
      EQ_TYPES_ALTS(VectorMapInt2,             "[{{ii}{ii}}]"),
      EQ_TYPES_ALTS(VectMapInt2,               "[{{ii}{ii}}]"),
      EQ_TYPES_ALTS(VectMapInt2,               "[{{ii}{ii}}]"),
      EQ_TYPES_ALTS(VectVectMapInt2,           "[[{{ii}{ii}}]]"),
      EQ_TYPES_ALTS(ComplexMap,                "{[{{ii}{ii}}][[{{ii}{ii}}]]}"),

      // Pointers
      EQ_PTRS_ALTS(bool,                       "X"),
      EQ_PTRS_ALTS(float,                      "X"),
      EQ_PTRS_ALTS(int,                        "X"),
      EQ_PTRS_ALTS(double,                     "X"),
      EQ_PTRS_ALTS(std::string,                "X"),
      EQ_PTRS_ALTS(std::vector<int>,           "X"),
      EQ_PTRS_ALTS(MapInt,                     "X"),
      EQ_PTRS_ALTS(bool,                       "X"),
      EQ_PTRS_ALTS(int,                        "X"),
      EQ_TYPES_ALTS(boost::shared_ptr<int>,    "X"),
      { qi::typeOf<void (*)(int, int)>(),      "X" },
      { qi::typeOf<decltype(&fun0)>(),         "X" },
      { qi::typeOf<decltype(&fun1)>(),         "X" },
      { qi::typeOf<decltype(&fun2)>(),         "X" },
      { qi::typeOf<decltype(&fun3)>(),         "X" },
      { qi::typeOf<decltype(&fun4)>(),         "X" },
      { qi::typeOf<decltype(&fun5)>(),         "X" },
      { qi::typeOf<decltype(&fun6)>(),         "X" },
      { qi::typeOf<decltype(fun0)>(),          "X" },
      { qi::typeOf<decltype(fun1)>(),          "X" },
      { qi::typeOf<decltype(fun2)>(),          "X" },
      { qi::typeOf<decltype(fun3)>(),          "X" },
      { qi::typeOf<decltype(fun4)>(),          "X" },
      { qi::typeOf<decltype(fun5)>(),          "X" },
      { qi::typeOf<decltype(fun6)>(),          "X" },
      { qi::typeOf<decltype(&vfun0)>(),        "X" },
      { qi::typeOf<decltype(&vfun1)>(),        "X" },
      { qi::typeOf<decltype(&vfun2)>(),        "X" },
      { qi::typeOf<decltype(&vfun3)>(),        "X" },
      { qi::typeOf<decltype(&vfun4)>(),        "X" },
      { qi::typeOf<decltype(&vfun5)>(),        "X" },
      { qi::typeOf<decltype(&vfun6)>(),        "X" },
      { qi::typeOf<decltype(vfun0)>(),         "X" },
      { qi::typeOf<decltype(vfun1)>(),         "X" },
      { qi::typeOf<decltype(vfun2)>(),         "X" },
      { qi::typeOf<decltype(vfun3)>(),         "X" },
      { qi::typeOf<decltype(vfun4)>(),         "X" },
      { qi::typeOf<decltype(vfun5)>(),         "X" },
      { qi::typeOf<decltype(vfun6)>(),         "X" },
      { qi::typeOf<decltype(&Foo::vfun0)>(),   "X" },
      { qi::typeOf<decltype(&Foo::vfun1)>(),   "X" },
      { qi::typeOf<decltype(&Foo::vfun2)>(),   "X" },
      { qi::typeOf<decltype(&Foo::vfun3)>(),   "X" },
      { qi::typeOf<decltype(&Foo::vfun4)>(),   "X" },
      { qi::typeOf<decltype(&Foo::vfun5)>(),   "X" },
      { qi::typeOf<decltype(&Foo::vfun6)>(),   "X" },
      { qi::typeOf<decltype(&Foo::fun0)>(),    "X" },
      { qi::typeOf<decltype(&Foo::fun1)>(),    "X" },
      { qi::typeOf<decltype(&Foo::fun2)>(),    "X" },
      { qi::typeOf<decltype(&Foo::fun3)>(),    "X" },
      { qi::typeOf<decltype(&Foo::fun4)>(),    "X" },
      { qi::typeOf<decltype(&Foo::fun5)>(),    "X" },
      { qi::typeOf<decltype(&Foo::fun6)>(),    "X" },
    }
  )
);

#undef EQ_PTRS_ALTS
#undef EQ_TYPES_ALTS

TEST_P(TestSignatureWithTypeToString, toString)
{
  const auto param = GetParam();
  const auto type = param.first;
  const auto expected = param.second;
  EXPECT_EQ(expected, type->signature().toString());
}

TEST_P(TestSignatureWithTypeToString, OuputStreamOperator)
{
  const auto param = GetParam();
  const auto type = param.first;
  const auto expected = param.second;
  std::ostringstream oss;
  oss << type->signature();
  EXPECT_EQ(expected, oss.str());
}
