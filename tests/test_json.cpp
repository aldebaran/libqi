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

  qi::GenericValue gv = qi::GenericValue::from(mps);

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

  qi::GenericValue gv = qi::GenericValue::from(mps);

  EXPECT_EQ("{ \"0\" : [ \"pif\", \"paf\", \"pof\" ], \"2\" : [ \"pif\", \"paf\", \"pof\" ] }", qi::encodeJSON(gv));

}

TEST(TestJSON, Simple) {
  EXPECT_EQ("true", qi::encodeJSON(qi::GenericValue::from(bool(true))));
  EXPECT_EQ("false", qi::encodeJSON(qi::GenericValue::from(bool(false))));
  EXPECT_EQ("32", qi::encodeJSON(qi::GenericValue::from(32)));
  EXPECT_EQ("\"ttc:42\"", qi::encodeJSON(qi::GenericValue::from("ttc:42")));
  EXPECT_EQ("32.4", qi::encodeJSON(qi::GenericValue::from(32.4f)));
  EXPECT_EQ("32.3", qi::encodeJSON(qi::GenericValue::from((double)32.3)));
}

TEST(TestJSON, String) {
  EXPECT_EQ("\" \\\" \"", qi::encodeJSON(qi::GenericValue::from(" \" ")));
  EXPECT_EQ("\" \\u0000 \"", qi::encodeJSON(qi::GenericValue::from(" \0 ")));
  EXPECT_EQ("\" \\u00C3\\u00A9 \"", qi::encodeJSON(qi::GenericValue::from(" é ")));

  EXPECT_EQ("\" \\\" \\u0000 \\u00C3\\u00A9 \"", qi::encodeJSON(qi::GenericValue::from(" \" \0 é ")));
}