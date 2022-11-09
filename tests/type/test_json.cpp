/*
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <climits>
#include <float.h>
#include <cmath>
#include <gtest/gtest.h>
#include <map>
#include <qi/anyvalue.hpp>
#include <qi/application.hpp>
#include <qi/type/typeinterface.hpp>
#include <qi/jsoncodec.hpp>
#include <qi/log.hpp>

qiLogCategory("qi.test_json");

struct MPoint {
  MPoint(int x=0, int y=0)
    : x(x)
    , y(y)
  {}
  int x;
  int y;
};


QI_TYPE_STRUCT(MPoint, x, y);

TEST(EncodeJSON, MapIntStringArray)
{
  std::map<int, std::vector<std::string> > mps;

  std::vector<std::string> vs;
  vs.push_back("pif");
  vs.push_back("paf");
  vs.push_back("pof");

  mps[0] = vs;
  mps[2] = vs;

  qi::AnyReference gv = qi::AnyReference::from(mps);

  EXPECT_EQ("{0:[\"pif\",\"paf\",\"pof\"],2:[\"pif\",\"paf\",\"pof\"]}", qi::encodeJSON(gv));
}

TEST(EncodeJSON, MST) {
  std::map<std::string, std::vector<std::string> > mps;

  std::vector<std::string> vs;
  vs.push_back("pif");
  vs.push_back("paf");
  vs.push_back("pof");

  mps["zero"] = vs;
  mps["two"] = vs;

  qi::AnyReference gv = qi::AnyReference::from(mps);

  EXPECT_EQ("{\"two\":[\"pif\",\"paf\",\"pof\"],\"zero\":[\"pif\",\"paf\",\"pof\"]}", qi::encodeJSON(gv));
}

TEST(EncodeJSON, PrettyPrint) {
  std::map<std::string, std::vector<std::string> > mps;

  std::vector<std::string> vs;
  vs.push_back("pif");
  vs.push_back("paf");
  vs.push_back("pof");

  mps["0"] = vs;
  mps["2"] = vs;

  qi::AnyReference gv = qi::AnyReference::from(mps);

  EXPECT_EQ("{\n  \"0\": [\n    \"pif\",\n    \"paf\",\n    \"pof\"\n  ],\n  \"2\": [\n    \"pif\",\n    \"paf\",\n    \"pof\"\n  ]\n}", qi::encodeJSON(gv, qi::JsonOption_PrettyPrint));
}

TEST(EncodeJSON, Simple) {
  EXPECT_EQ("true", qi::encodeJSON(bool(true)));
  EXPECT_EQ("false", qi::encodeJSON(bool(false)));
  EXPECT_EQ("32", qi::encodeJSON(32));
  EXPECT_EQ("\"ttc:42\"", qi::encodeJSON("ttc:42"));
  EXPECT_EQ("32.4000015", qi::encodeJSON(32.4f));
  EXPECT_EQ("32.299999999999997", qi::encodeJSON((double)32.3));

  qi::AnyValue gv(qi::TypeInterface::fromSignature(qi::Signature("c")));
  gv.setInt(42);
  EXPECT_EQ("42", qi::encodeJSON(gv));
}

TEST(EncodeJSON, SimpleAutoGV) {
  EXPECT_EQ("true", qi::encodeJSON(true));
  EXPECT_EQ("false", qi::encodeJSON(false));
  EXPECT_EQ("32", qi::encodeJSON(32));
  EXPECT_EQ("\"ttc:42\"", qi::encodeJSON("ttc:42"));
  EXPECT_EQ("32.4000015", qi::encodeJSON(32.4f));
  EXPECT_EQ("32.299999999999997", qi::encodeJSON((double)32.3));
}

TEST(EncodeJSON, String) {
  EXPECT_EQ("\" \\\" \"", qi::encodeJSON(" \" "));
  EXPECT_EQ("\" ' \"", qi::encodeJSON(" ' "));
  EXPECT_EQ("\" \\u0000 \"", qi::encodeJSON(" \0 "));
  EXPECT_EQ("\" \\u00E9 \"", qi::encodeJSON(" é "));

  EXPECT_EQ("\" \\\" \\u0000 \\u00E9 \"", qi::encodeJSON(" \" \0 é "));
}

TEST(EncodeJSON, CharTuple) {
  qi::AnyValue gv(qi::TypeInterface::fromSignature(qi::Signature("(c)")));
  EXPECT_EQ("[0]", qi::encodeJSON(gv));
}

TEST(EncodeJSON, EmptyValue) {
  qi::AnyValue gv(qi::TypeInterface::fromSignature(qi::Signature("m")));
  EXPECT_EQ("", qi::encodeJSON(gv));
}

TEST(EncodeJSON, Dynamics) {
  qi::AnyValue gv(qi::TypeInterface::fromSignature(qi::Signature("m")));
  gv.setDynamic(qi::AnyReference::from("plouf"));
  EXPECT_EQ("\"plouf\"", qi::encodeJSON(gv));
}

TEST(EncodeJSON, NamedStruct) {
  MPoint mp(41, 42);
  qi::AnyValue gvr = qi::AnyValue::from(mp);
  EXPECT_EQ("{\"x\":41,\"y\":42}", qi::encodeJSON(gvr));
}

TEST(EncodeJSON, OptionalValue)
{
  EXPECT_EQ("null", qi::encodeJSON(boost::optional<std::string>()));
  EXPECT_EQ("null", qi::encodeJSON(boost::optional<int>()));
  EXPECT_EQ("\"foo\"", qi::encodeJSON(boost::optional<std::string>("foo")));
  EXPECT_EQ("642", qi::encodeJSON(boost::optional<int>(642)));
}

template<class T>
std::string itoa(T n)
{
  std::stringstream ss;

  ss << n;
  return ss.str();
}

bool pred(char const& c)
{
  bool res = ((c < '0' || c > '9')
              && c != '.'
      && c != '+' && c != '-'
      && c != 'e');
  return res;
}

std::string cleanStr(std::string const& numberStr)
{
  std::string copy = numberStr;
  size_t pos;

  if ((pos = copy.find("double")) != std::string::npos)
    copy.erase(pos, 6);
  std::string::iterator it = std::remove_if(copy.begin(), copy.end(), &pred);

  copy.erase(it, copy.end());
  return copy;
}

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

TEST(DecodeJSON, EmptyValue) {
  ASSERT_ANY_THROW(qi::decodeJSON(""));
}

TEST(DecodeJSON, String) {
  // Broken string
  EXPECT_ANY_THROW(qi::decodeJSON("\""));
  // Empty string
  EXPECT_NO_THROW(qi::decodeJSON("\"\""));
  // Normal string
  EXPECT_NO_THROW(qi::decodeJSON("\"test\""));

  // Test result
  EXPECT_STREQ("test", qi::decodeJSON("\"test\"").asString().c_str());

  // Test result with spaces before
  EXPECT_STREQ("   test", qi::decodeJSON("\"   test\"").asString().c_str());

  // Escape chars
  std::string testString =
      "\""
      "\\\""
      "\\\\"
      "\\/"
      "\\b"
      "\\f"
      "\\n"
      "\\r"
      "\\t"
      "\"";
  std::string resString =
      "\""
      "\\"
      "/"
      "\b"
      "\f"
      "\n"
      "\r"
      "\t";
  ASSERT_NO_THROW(qi::decodeJSON(testString));
  ASSERT_STREQ(resString.c_str(), qi::decodeJSON(testString).asString().c_str());
}

TEST(DecodeJSON, Integer) {
  // broken value
  EXPECT_ANY_THROW(qi::decodeJSON("--42"));

  ASSERT_NO_THROW(qi::decodeJSON("42"));
  ASSERT_NO_THROW(qi::decodeJSON("-42"));

  ASSERT_NO_THROW(qi::decodeJSON("42").asInt64());

  // positive value
  EXPECT_EQ(42, qi::decodeJSON("42").asInt64());

  // negative value
  EXPECT_EQ(-42, qi::decodeJSON("-42").asInt64());

  // max/min vals
  EXPECT_EQ(LONG_MAX, qi::decodeJSON(itoa(LONG_MAX)).asInt64());
  EXPECT_EQ(LONG_MIN, qi::decodeJSON(itoa(LONG_MIN)).asInt64());
}

// Helper to access to binary representation of the float
union Double_t
{
    Double_t(double num = 0.0f) : f(num) {}
    int64_t i;
    double f;
};

std::ostream &operator<<(std::ostream &os, const Double_t &f)
{
  os.precision(std::numeric_limits<double>::max_digits10+5);
  os << f.f << " / 0x"  << std::hex << std::setw(16) << std::setfill('0') << f.i << std::dec;
  return os;
}

::testing::AssertionResult AssertDoubleRoundTrip(const char* f_expr,
                                           double f) {
  std::string str = qi::encodeJSON(f);
  double ff = qi::decodeJSON(str).as<double>();
#if defined(_MSC_VER)
  // MSVC fails to roundtrip exactly. See here for the upstream report
  // forum:
  // https://social.msdn.microsoft.com/Forums/en-US/214a6e0c-2036-47ab-a6ff-c7737f5cf395/imprecise-deserializing-of-decimals-to-double?forum=vcgeneral
  // bug report:
  // connect.microsoft.com/VisualStudio/feedback/details/2245376/imprecise-deserialization-of-decimals-to-double/
  using FPdouble = testing::internal::FloatingPoint<double>;
  if (FPdouble(f).AlmostEquals(FPdouble(f)))
#else
  if (f == ff)
#endif
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure()
      << f_expr << " with value " << Double_t(f)
      << " is serialized to \"" << str << "\"  then deserialized to "
      << Double_t(ff);
}

#define EXPECT_DOUBLE_ROUNDTRIP(f) EXPECT_PRED_FORMAT1(AssertDoubleRoundTrip, f);

TEST(DecodeJSON, Float) {
  // broken value
  ASSERT_NO_THROW(qi::decodeJSON("42.43"));
  ASSERT_NO_THROW(qi::decodeJSON("-42.43"));
  ASSERT_NO_THROW(qi::decodeJSON("0.42"));
  ASSERT_NO_THROW(qi::decodeJSON("42.0"));
  ASSERT_NO_THROW(qi::decodeJSON("0.0000042"));
  ASSERT_NO_THROW(qi::decodeJSON("0.42").asDouble());
  ASSERT_NO_THROW(qi::decodeJSON("40.40e+10"));
  ASSERT_NO_THROW(qi::decodeJSON("42e+10"));
  ASSERT_NO_THROW(qi::decodeJSON("42e-10"));
  ASSERT_NO_THROW(qi::decodeJSON("42e10"));

  // comparing floating point values is usually done with an epsilon.
  // However here we do exact comparisons, on purpose, since (de)serialization
  // shall be lossless.
  EXPECT_EQ(0.42, qi::decodeJSON("0.42").as<double>());
  EXPECT_EQ(42.0, qi::decodeJSON("42.0").as<double>());
  EXPECT_EQ(0.0000042, qi::decodeJSON("0.0000042").as<double>());

  // positive value
  EXPECT_EQ(42.43, qi::decodeJSON("42.43").as<double>());

  // negative value
  EXPECT_EQ(-42.43, qi::decodeJSON("-42.43").as<double>());

  // with E
  EXPECT_EQ(42e+10, qi::decodeJSON("42e+10").as<double>());
  EXPECT_EQ(42e-10, qi::decodeJSON("42e-10").as<double>());
  EXPECT_EQ(42e10, qi::decodeJSON("42e10").as<double>());

  // roundtrip
  EXPECT_EQ(0, qi::decodeJSON(qi::encodeJSON(0)).to<int>());
  EXPECT_EQ(0., qi::decodeJSON(qi::encodeJSON(0.)).to<double>());
  EXPECT_EQ(0.f, qi::decodeJSON(qi::encodeJSON(0.f)).to<float>());
  EXPECT_EQ(1.6f, qi::decodeJSON(qi::encodeJSON(1.6f)).to<float>());

  EXPECT_DOUBLE_ROUNDTRIP(1.5);
  EXPECT_DOUBLE_ROUNDTRIP(-1.8364336390987788);
  EXPECT_DOUBLE_ROUNDTRIP(std::numeric_limits<double>::max());
  EXPECT_DOUBLE_ROUNDTRIP(std::numeric_limits<double>::min());
  EXPECT_DOUBLE_ROUNDTRIP(std::numeric_limits<double>::lowest());
  EXPECT_DOUBLE_ROUNDTRIP(std::numeric_limits<double>::denorm_min());
}

TEST(DecodeJSON, Array) {
  // good parse and type
  ASSERT_NO_THROW(qi::decodeJSON("[]"));
  ASSERT_NO_THROW(qi::decodeJSON("[42]"));
  ASSERT_NO_THROW(qi::decodeJSON("[1, 2]"));
  ASSERT_NO_THROW(qi::decodeJSON("[1, 2, [1, 2, 3, [42], 45], 1.2]"));
  ASSERT_EQ(qi::TypeKind_List, qi::decodeJSON("[42]").kind());

  // size 1
  ASSERT_NO_THROW(qi::decodeJSON("[42]").size());
  ASSERT_EQ(1U, qi::decodeJSON("[42]").size());

  // size > 1
  ASSERT_EQ(6U, qi::decodeJSON("[1, 2, 3, 4, 5, 6]").size());

  // complex array
  qi::AnyValue val = qi::decodeJSON("[1, [2, 3]]");
  ASSERT_EQ(qi::TypeKind_List, val.kind());
  ASSERT_EQ(qi::TypeKind_Int, val[0].content().kind());
  ASSERT_EQ(qi::TypeKind_List, val[1].content().kind());
  ASSERT_EQ(2U, qi::decodeJSON("[1, [2, 3]]")[1].content().size());
}

TEST(DecodeJSON, Object) {
  // good parse
  ASSERT_NO_THROW(qi::decodeJSON("{}"));
  ASSERT_NO_THROW(qi::decodeJSON("{\"a\":42}"));
  ASSERT_NO_THROW(qi::decodeJSON("{\"a\":42, \"b\":1.0}"));
  ASSERT_NO_THROW(qi::decodeJSON("{\"a\":42, \"b\":{\"c\":[1, 2]}}"));

  ASSERT_ANY_THROW(qi::decodeJSON("{"));
  ASSERT_ANY_THROW(qi::decodeJSON("{42:42}"));
  ASSERT_ANY_THROW(qi::decodeJSON("{\"42\":}"));

  ASSERT_EQ(qi::TypeKind_Map, qi::decodeJSON("{}").kind());
  ASSERT_EQ(1U, qi::decodeJSON("{\"a\":42}").size());
  ASSERT_EQ(qi::TypeKind_Int, qi::decodeJSON("{\"a\":42}")["a"].content().kind());

}

TEST(DecodeJSON, special) {
  // good parse
  ASSERT_NO_THROW(qi::decodeJSON("true"));
  ASSERT_NO_THROW(qi::decodeJSON("false"));
  ASSERT_NO_THROW(qi::decodeJSON("null"));
  ASSERT_ANY_THROW(qi::decodeJSON("tru"));

  ASSERT_EQ(qi::TypeKind_Int, qi::decodeJSON("true").kind());
  ASSERT_EQ(qi::TypeKind_Int, qi::decodeJSON("false").kind());
  ASSERT_EQ(qi::TypeKind_Void, qi::decodeJSON("null").kind());

  ASSERT_EQ(0U, static_cast<qi::IntTypeInterface*>(qi::decodeJSON("true").type())->size());
  ASSERT_EQ(0U, static_cast<qi::IntTypeInterface*>(qi::decodeJSON("false").type())->size());

  ASSERT_EQ(1, qi::decodeJSON("true").toInt());
  ASSERT_EQ(0, qi::decodeJSON("false").toInt());
}

TEST(DecodeJSON, itOverload) {
  std::string testString = "<jsonString=\"[\"a\", 42]\"/>";

  qi::AnyValue val;
  ASSERT_NO_THROW(qi::decodeJSON(testString.begin() + 13, testString.end(), val));
  ASSERT_EQ('\"', *qi::decodeJSON(testString.begin() + 13, testString.end(), val));
  qi::decodeJSON(testString.begin() + 13, testString.end(), val);
  ASSERT_EQ(qi::TypeKind_List, val.kind());

  std::string testString2 = "<jsonString=\"[\"a\", 42\"/>";
  ASSERT_ANY_THROW(qi::decodeJSON(testString2.begin() + 13, testString2.end(), val));

}

TEST(DecodeJSON, Strings)
{
  static const std::string s1 = "{ \"content\" : \"pone\" }";
  static const std::string s2 = "{ \"content\" : \"poné\" }";
  static const std::string s3 = "{ \"content\" : \"pon\\u00e9\" }";
  static const std::string s4 = "{ \"content\" : \"pon\\u00ex\" }";
  static const std::string s5 = "{ \"content\" : \"pon\\u\"}";

  qi::AnyValue v;
  v = qi::decodeJSON(s1);
  EXPECT_EQ("pone", v["content"].to<std::string>());

  v = qi::decodeJSON(s2);
  EXPECT_EQ("poné", v["content"].to<std::string>());

  ASSERT_NO_THROW(v = qi::decodeJSON(s3));
  EXPECT_EQ("pon\xc3\xa9", v["content"].to<std::string>());

  ASSERT_ANY_THROW(qi::decodeJSON(s4));
  ASSERT_ANY_THROW(qi::decodeJSON(s5));
}


TEST(DecodeJSON, ignoringWhiteSpace)
{
  static const std::string ohMinceAlors =
      "{\n"
      "  \"petit\" : \"poney\"\n"
      "}\n";
  std::map<std::string, std::string> ahBahZut = qi::decodeJSON(ohMinceAlors)
      .toMap<std::string, std::string>();
  ASSERT_EQ(std::string("poney"), ahBahZut["petit"]);
}

struct Qiqi
{
  float ffloat;
  double fdouble;
  int fint;
  bool operator==(const Qiqi &other) const
  {
      return (ffloat == other.ffloat) &&
             (fdouble == other.fdouble) &&
             (fint == other.fint);
  }
};
QI_TYPE_STRUCT_REGISTER(Qiqi, ffloat, fdouble, fint);

TEST(DecodeJSON, StructWithDifferentSizedFields)
{
  Qiqi val;
  val.ffloat = 3.14f;
  val.fdouble = 5.5555;
  val.fint = 44;

  std::cout << qi::AnyValue(val).signature().toString() << std::endl;

  std::string json = qi::encodeJSON(val);
  qiLogDebug() << json;
  qi::AnyValue res_any = qi::decodeJSON(json);
  qiLogDebug() << res_any.signature().toString();
  Qiqi res = res_any.to<Qiqi>();
  EXPECT_EQ(val,
            res) << qi::encodeJSON(val) << "\n" << qi::encodeJSON(res);
}
