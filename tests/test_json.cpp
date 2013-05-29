/*
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <climits>
#include <float.h>
#include <cmath>
#include <gtest/gtest.h>
#include <map>
#include <qitype/genericvalue.hpp>
#include <qi/application.hpp>
#include <qitype/type.hpp>

struct MPoint {
  MPoint(int x=0, int y=0)
    : x(x)
    , y(y)
  {}
  int x;
  int y;
};


QI_TYPE_STRUCT(MPoint, x, y);

TEST(TestJSON, MapIntTableString) {
  std::map<int, std::vector<std::string> > mps;

  std::vector<std::string> vs;
  vs.push_back("pif");
  vs.push_back("paf");
  vs.push_back("pof");

  mps[0] = vs;
  mps[2] = vs;

  qi::GenericValuePtr gv(&mps);

  EXPECT_EQ("{ 0 : [ \"pif\", \"paf\", \"pof\" ], 2 : [ \"pif\", \"paf\", \"pof\" ] }", qi::encodeJSON(gv));
}

TEST(TestJSON, MST) {
  std::map<std::string, std::vector<std::string> > mps;

  std::vector<std::string> vs;
  vs.push_back("pif");
  vs.push_back("paf");
  vs.push_back("pof");

  mps["0"] = vs;
  mps["2"] = vs;

  qi::GenericValuePtr gv(&mps);

  EXPECT_EQ("{ \"0\" : [ \"pif\", \"paf\", \"pof\" ], \"2\" : [ \"pif\", \"paf\", \"pof\" ] }", qi::encodeJSON(gv));

}

TEST(TestJSON, Simple) {
  EXPECT_EQ("true", qi::encodeJSON(qi::GenericValueRef(bool(true))));
  EXPECT_EQ("false", qi::encodeJSON(qi::GenericValueRef(bool(false))));
  EXPECT_EQ("32", qi::encodeJSON(qi::GenericValueRef(32)));
  EXPECT_EQ("\"ttc:42\"", qi::encodeJSON(qi::GenericValueRef("ttc:42")));
  EXPECT_EQ("32.4", qi::encodeJSON(qi::GenericValueRef(32.4f)));
  EXPECT_EQ("32.3", qi::encodeJSON(qi::GenericValueRef((double)32.3)));

  qi::GenericValue gv(qi::Type::fromSignature(qi::Signature("c")));
  gv.setInt(42);
  EXPECT_EQ("42", qi::encodeJSON(gv));
}

TEST(TestJSON, String) {
  EXPECT_EQ("\" \\\" \"", qi::encodeJSON(qi::GenericValueRef(" \" ")));
  EXPECT_EQ("\" \\u0000 \"", qi::encodeJSON(qi::GenericValueRef(" \0 ")));
  EXPECT_EQ("\" \\u00C3\\u00A9 \"", qi::encodeJSON(qi::GenericValueRef(" é ")));

  EXPECT_EQ("\" \\\" \\u0000 \\u00C3\\u00A9 \"", qi::encodeJSON(qi::GenericValueRef(" \" \0 é ")));
}

TEST(TestJSON, CharTuple) {
  qi::GenericValue gv(qi::Type::fromSignature(qi::Signature("(c)")));
  EXPECT_EQ("[ 0 ]", qi::encodeJSON(gv));
}

TEST(TestJSON, EmptyValue) {
  qi::GenericValue gv(qi::Type::fromSignature(qi::Signature("m")));
  EXPECT_EQ("", qi::encodeJSON(gv));
}

TEST(TestJSON, Dynamics) {
  qi::GenericValuePtr gv(qi::Type::fromSignature(qi::Signature("m")));
  qi::GenericValue gvr = qi::GenericValue::from("plouf");
  gv.setDynamic(gvr);
  EXPECT_EQ("\"plouf\"", qi::encodeJSON(gv));
}

TEST(TestJSON, NamedStruct) {
  MPoint mp(41, 42);
  qi::GenericValue gvr = qi::GenericValue::from(mp);
  EXPECT_EQ("{ \"x\" : 41, \"y\" : 42 }", qi::encodeJSON(gvr));
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

TEST(TestJSONDecoder, EmptyValue) {
  ASSERT_ANY_THROW(qi::decodeJSON(""));
}

TEST(TestJSONDecoder, String) {
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

TEST(TestJSONDecoder, Integer) {
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

TEST(TestJSONDecoder, Float) {
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

  EXPECT_EQ(0.42, qi::decodeJSON("0.42").asDouble());
  EXPECT_EQ(42.0, qi::decodeJSON("42.0").asDouble());
  EXPECT_EQ(0.0000042, qi::decodeJSON("0.0000042").asDouble());

  // positive value
  EXPECT_EQ(42.43, qi::decodeJSON("42.43").asDouble());

  // negative value
  EXPECT_EQ(-42.43, qi::decodeJSON("-42.43").asDouble());

  // with E
  EXPECT_EQ(42e+10, qi::decodeJSON("42e+10"));
  EXPECT_EQ(42e-10, qi::decodeJSON("42e-10"));
  EXPECT_EQ(42e10, qi::decodeJSON("42e10"));

  // max/min vals
  EXPECT_EQ(DBL_MAX, qi::decodeJSON(cleanStr(STR(DBL_MAX))).asDouble());
  EXPECT_EQ(DBL_MIN, qi::decodeJSON(cleanStr(STR(DBL_MIN))).asDouble());
}

TEST(TestJSONDecoder, Array) {
  // good parse and type
  ASSERT_NO_THROW(qi::decodeJSON("[]"));
  ASSERT_NO_THROW(qi::decodeJSON("[42]"));
  ASSERT_NO_THROW(qi::decodeJSON("[1, 2]"));
  ASSERT_NO_THROW(qi::decodeJSON("[1, 2, [1, 2, 3, [42], 45], 1.2]"));
  ASSERT_EQ(qi::Type::List, qi::decodeJSON("[42]").kind());

  // size 1
  ASSERT_NO_THROW(qi::decodeJSON("[42]").size());
  ASSERT_EQ(1U, qi::decodeJSON("[42]").size());

  // size > 1
  ASSERT_EQ(6U, qi::decodeJSON("[1, 2, 3, 4, 5, 6]").size());

  // complex array
  qi::GenericValue val = qi::decodeJSON("[1, [2, 3]]");
  ASSERT_EQ(qi::Type::List, val.kind());
  ASSERT_EQ(qi::Type::Int, val[0].asDynamic().kind());
  ASSERT_EQ(qi::Type::List, val[1].asDynamic().kind());
  ASSERT_EQ(2U, qi::decodeJSON("[1, [2, 3]]")[1].asDynamic().size());
}

TEST(TestJSONDecoder, Object) {
  // good parse
  ASSERT_NO_THROW(qi::decodeJSON("{}"));
  ASSERT_NO_THROW(qi::decodeJSON("{\"a\":42}"));
  ASSERT_NO_THROW(qi::decodeJSON("{\"a\":42, \"b\":1.0}"));
  ASSERT_NO_THROW(qi::decodeJSON("{\"a\":42, \"b\":{\"c\":[1, 2]}}"));

  ASSERT_ANY_THROW(qi::decodeJSON("{"));
  ASSERT_ANY_THROW(qi::decodeJSON("{42:42}"));
  ASSERT_ANY_THROW(qi::decodeJSON("{\"42\":}"));

  ASSERT_EQ(qi::Type::Map, qi::decodeJSON("{}").kind());
  ASSERT_EQ(1U, qi::decodeJSON("{\"a\":42}").size());
  ASSERT_EQ(qi::Type::Int, qi::decodeJSON("{\"a\":42}")["a"].asDynamic().kind());

}

TEST(TestJSONDecoder, special) {
  // good parse
  ASSERT_NO_THROW(qi::decodeJSON("true"));
  ASSERT_NO_THROW(qi::decodeJSON("false"));
  ASSERT_NO_THROW(qi::decodeJSON("null"));
  ASSERT_ANY_THROW(qi::decodeJSON("tru"));

  ASSERT_EQ(qi::Type::Int, qi::decodeJSON("true").kind());
  ASSERT_EQ(qi::Type::Int, qi::decodeJSON("false").kind());
  ASSERT_EQ(qi::Type::Void, qi::decodeJSON("null").kind());

  ASSERT_EQ(0U, static_cast<qi::TypeInt*>(qi::decodeJSON("true").type)->size());
  ASSERT_EQ(0U, static_cast<qi::TypeInt*>(qi::decodeJSON("false").type)->size());

  ASSERT_EQ(1, qi::decodeJSON("true").toInt());
  ASSERT_EQ(0, qi::decodeJSON("false").toInt());
}

TEST(TestJSONDecoder, itOverload) {
  std::string testString = "<jsonString=\"[\"a\", 42]\"/>";

  qi::GenericValue val;
  ASSERT_NO_THROW(qi::decodeJSON(testString.begin() + 13, testString.end(), val));
  ASSERT_EQ('\"', *qi::decodeJSON(testString.begin() + 13, testString.end(), val));
  qi::decodeJSON(testString.begin() + 13, testString.end(), val);
  ASSERT_EQ(qi::Type::List, val.kind());

  std::string testString2 = "<jsonString=\"[\"a\", 42\"/>";
  ASSERT_ANY_THROW(qi::decodeJSON(testString2.begin() + 13, testString2.end(), val));

}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
