/*
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <map>
#include <qitype/genericvalue.hpp>

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

  qi::GenericValue gv(qi::Type::fromSignature("c"));
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
  qi::GenericValue gv(qi::Type::fromSignature("(c)"));
  EXPECT_EQ("[ 0 ]", qi::encodeJSON(gv));
}


TEST(TestJSON, EmptyValue) {
  qi::GenericValue gv(qi::Type::fromSignature("m"));
  EXPECT_EQ("", qi::encodeJSON(gv));
}


TEST(TestJSON, Dynamics) {
  qi::GenericValuePtr gv(qi::Type::fromSignature("m"));
  qi::GenericValue gvr = qi::GenericValue::from("plouf");
  gv.setDynamic(gvr);
  EXPECT_EQ("\"plouf\"", qi::encodeJSON(gv));
}
