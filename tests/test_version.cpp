/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/version.hpp>

TEST(TestVersion, TestVersionCompare) {
  EXPECT_TRUE(qi::Version("1.2.3") == qi::Version("1.2.3"));
  EXPECT_TRUE(qi::Version("1.20.3") > qi::Version("1.2.3"));
  EXPECT_TRUE(qi::Version("0") > qi::Version(""));
  EXPECT_TRUE(qi::Version("1.2..3") > qi::Version("1.2.3"));
  EXPECT_TRUE(qi::Version("1.2.-.3") > qi::Version("1.2.3"));
  EXPECT_TRUE(qi::Version("1.2.3.0") > qi::Version("1.2.3"));
  EXPECT_TRUE(qi::Version("1.2.3.0") < qi::Version("1.2.a3"));
  EXPECT_TRUE(qi::Version("1.2.3.0") < qi::Version("1.2.3a"));
  EXPECT_TRUE(qi::Version("1.2.3.0") < qi::Version("1a1.2.3"));
  EXPECT_TRUE(qi::Version("10.2.3.0") > qi::Version("1a1.2.3"));

  EXPECT_TRUE(qi::Version("1&.2.3") < qi::Version("1&&&&.2.3"));

  EXPECT_TRUE(qi::Version("a") == qi::Version("a"));
  EXPECT_TRUE(qi::Version("aa") > qi::Version("a"));
  EXPECT_TRUE(qi::Version("aa") < qi::Version("b"));
  EXPECT_TRUE(qi::Version("01") < qi::Version("2"));
  EXPECT_TRUE(qi::Version("a0") > qi::Version("a"));

  EXPECT_TRUE(qi::Version("1a0") > qi::Version("1.0"));
  EXPECT_TRUE(qi::Version("a0a") > qi::Version("a.a"));
  EXPECT_TRUE(qi::Version("1a0") > qi::Version("1.1"));
  EXPECT_TRUE(qi::Version("a0a") > qi::Version("a.b"));

}

TEST(TestVersion, comparisonOperators)
{
  qi::Version p1("1.0.0"), p2("2.0.0");

  ASSERT_FALSE(p1 > p2);
  ASSERT_TRUE(p1 < p2);
  ASSERT_FALSE(p1 == p2);
  ASSERT_TRUE(p1 != p2);
  ASSERT_FALSE(p1 >= p2);
  ASSERT_TRUE(p1 <= p2);

  qi::Version p3 = p1;

  ASSERT_FALSE(p1 > p3);
  ASSERT_FALSE(p1 < p3);
  ASSERT_TRUE(p1 == p3);
  ASSERT_FALSE(p1 != p3);
  ASSERT_TRUE(p1 >= p3);
  ASSERT_TRUE(p1 <= p3);

  ASSERT_TRUE(p2 > p3);
  ASSERT_FALSE(p2 < p3);
  ASSERT_FALSE(p2 == p3);
  ASSERT_TRUE(p2 != p3);
  ASSERT_TRUE(p2 >= p3);
  ASSERT_FALSE(p2 <= p3);
}

TEST(TestVersion, assigmentAndCopy)
{
  qi::Version p1("1.0.0");
  qi::Version p2 = p1;
  qi::Version p3(p1);
  ASSERT_TRUE(p1 == p2);
  ASSERT_TRUE(p1 == p3);
  ASSERT_TRUE(p2 == p3);

  p1 = "1.0.0";
  ASSERT_TRUE(p1 == "1.0.0");
  ASSERT_TRUE(p1 != "2.0.0");
}
