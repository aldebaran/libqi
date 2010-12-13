/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/signature.hpp>
#include <qi/signature/signature_lexer.hpp>

#include <vector>
#include <map>

#include "alvalue.pb.h"

TEST(TestSignatureLexer, Simple) {
  qi::SignatureLexer::Element elt;

  qi::SignatureLexer lex1("i");
  elt = lex1.getNext();

  EXPECT_STREQ("i", elt.signature);
  EXPECT_FALSE(elt.child_1);
  EXPECT_FALSE(elt.child_2);
  EXPECT_FALSE(elt.pointer);
}

TEST(TestSignatureLexer, STL) {
  qi::SignatureLexer::Element elt;

  qi::SignatureLexer lex1("[iiss]");
  EXPECT_THROW(lex1.getNext(), qi::BadFormatException);

  qi::SignatureLexer lex2("[i]*");
  elt = lex2.getNext();
  EXPECT_STREQ("[i]*", elt.signature);
  EXPECT_STREQ("i]*", elt.child_1);
  EXPECT_FALSE(elt.child_2);
  EXPECT_TRUE(elt.pointer);

  qi::SignatureLexer lex3("{is}*");
  elt = lex3.getNext();
  EXPECT_STREQ("{is}*", elt.signature);
  EXPECT_STREQ("is}*", elt.child_1);
  EXPECT_STREQ("s}*", elt.child_2);
  EXPECT_TRUE(elt.pointer);
}

TEST(TestSignatureLexer, Protobuf) {
  qi::SignatureLexer::Element elt;

  qi::SignatureLexer lex1("@toto@*i");
  elt = lex1.getNext();

  EXPECT_STREQ("@toto@*i", elt.signature);
  EXPECT_FALSE(elt.child_1);
  EXPECT_FALSE(elt.child_2);
  EXPECT_TRUE(elt.pointer);

  elt = lex1.getNext();
  EXPECT_STREQ("i", elt.signature);
  EXPECT_FALSE(elt.child_1);
  EXPECT_FALSE(elt.child_2);
  EXPECT_FALSE(elt.pointer);
}
