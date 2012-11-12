/*
 *  Author(s):
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <string>

#include <gtest/gtest.h>

#include <qimessaging/url.hpp>


TEST(TestURL, EmptyUrl)
{
  qi::Url url;

  EXPECT_EQ("", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());
}


TEST(TestURL, InvalidUrl)
{
  qi::Url url("slip");

  EXPECT_EQ("", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());

  url = "slip:";
  EXPECT_EQ("", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());

  url = qi::Url("slip//");
  EXPECT_EQ("", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());

  url = "slip::";
  EXPECT_EQ("", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());

  url = "slip:/:";
  EXPECT_EQ("", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());
}

TEST(TestURL, ValidUrl)
{
  qi::Url url("slip://pd:5");

  EXPECT_EQ("slip", url.protocol());
  EXPECT_EQ("pd", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url.isValid());

  url = "slip://:5";

  EXPECT_EQ("slip", url.protocol());
  EXPECT_EQ("", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url.isValid());
}
