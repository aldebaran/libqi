/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/signature.hpp>
#include <qi/signature/signature_iterator.hpp>

#include <vector>
#include <map>

#include "alvalue.pb.h"

TEST(TestSignatureIterator, Simple) {

  qi::Signature::iterator it;

  qi::Signature sig("i");
  it = sig.begin();

  EXPECT_STREQ("i", it.signature);
  EXPECT_FALSE(it.child_1);
  EXPECT_FALSE(it.child_2);
  EXPECT_FALSE(it.pointer);
}

TEST(TestSignatureIterator, STL) {
  qi::Signature::iterator it;

  qi::Signature sig1("[iiss]");
  EXPECT_THROW(sig1.begin(), qi::BadFormatException);

  qi::Signature sig2("[i]*");
  it = sig2.begin();
  EXPECT_STREQ("[i]*", it.signature);
  EXPECT_STREQ("i]*",  it.child_1);
  EXPECT_FALSE(it.child_2);
  EXPECT_TRUE(it.pointer);

  qi::Signature sig3("{is}*");
  it = sig3.begin();
  EXPECT_STREQ("{is}*", it.signature);
  EXPECT_STREQ("is}*",  it.child_1);
  EXPECT_STREQ("s}*",   it.child_2);
  EXPECT_TRUE(it.pointer);
}

TEST(TestSignatureIterator, Protobuf) {
  qi::Signature::iterator it;

  qi::Signature sig1("@toto@*i");
  it = sig1.begin();

  EXPECT_STREQ("@toto@*i", it.signature);
  EXPECT_FALSE(it.child_1);
  EXPECT_FALSE(it.child_2);
  EXPECT_TRUE(it.pointer);

  EXPECT_TRUE(it != sig1.end());
  ++it;
  EXPECT_STREQ("i", it.signature);
  EXPECT_FALSE(it.child_1);
  EXPECT_FALSE(it.child_2);
  EXPECT_FALSE(it.pointer);
  EXPECT_TRUE(it != sig1.end());
  ++it;
  EXPECT_TRUE(it == sig1.end());
}
