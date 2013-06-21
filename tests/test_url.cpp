/*
 *  Author(s):
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <string>

#include <gtest/gtest.h>

#include <qitype/typeinterface.hpp>
#include <qitype/anyfunction.hpp>
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
  qi::Url url("example.com");

  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());

  url = "example.com:";
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());

  url = qi::Url("tcp//");
  EXPECT_EQ("tcp//", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());

  url = "tcp::";
  EXPECT_EQ("tcp", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());

  url = "tcp:/:";
  EXPECT_EQ("tcp", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());

  url = "tcp://example.com";
  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_FALSE(url.isValid());
}

TEST(TestURL, ValidUrl)
{
  qi::Url url("tcp://example.com:5");

  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url.isValid());
  EXPECT_EQ("tcp://example.com:5", url.str());

  url = "tcp://:5";

  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url.isValid());
  EXPECT_EQ("tcp://:5", url.str());

  url = qi::Url("tcp://example.com", 5);

  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url.isValid());
  EXPECT_EQ("tcp://example.com:5", url.str());

  url = qi::Url("tcp://example.com:5", 10);

  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url.isValid());
  EXPECT_EQ("tcp://example.com:5", url.str());

  url = qi::Url("example.com:5", "tcp");

  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url.isValid());
  EXPECT_EQ("tcp://example.com:5", url.str());

  url = qi::Url("tcp://example.com:5", "http");

  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url.isValid());
  EXPECT_EQ("tcp://example.com:5", url.str());

  url = qi::Url("example.com", "tcp", 5);

  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url.isValid());
  EXPECT_EQ("tcp://example.com:5", url.str());

  url = qi::Url("tcp://example.com:5", "http", 10);

  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url.isValid());
  EXPECT_EQ("tcp://example.com:5", url.str());
}

TEST(TestURL, CopyUrl)
{
  qi::Url url("tcp://example.com:5");
  EXPECT_TRUE(url.isValid());
  qi::Url url2(url);
  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(5, url.port());
  EXPECT_TRUE(url2.isValid());
  EXPECT_EQ("tcp://example.com:5", url.str());
}

std::string urlOut(const qi::Url& u)
{
  return u.str();
}

qi::Url urlIn(const std::string& s)
{
  return qi::Url(s);
}

TEST(TestURL, TypeSystem)
{
  qi::TypeInterface* t = qi::typeOf<qi::Url>();
  EXPECT_EQ(qi::TypeKind_String, t->kind());
  qi::AnyFunction fout = qi::AnyFunction::from(&urlOut);
  qi::AnyFunction fin = qi::AnyFunction::from(&urlIn);
  std::vector<qi::AnyReference> args;
  std::string s("tcp://canard:0");
  EXPECT_EQ(s, qi::AnyReference(s).to<qi::Url>().str());
  args.push_back(qi::AnyReference(s));
  qi::Url u = fout.call(args).to<qi::Url>();
  EXPECT_EQ(s, u.str());
  EXPECT_EQ(u, qi::Url(qi::AnyReference(u).to<std::string>()));
  args.clear();
  args.push_back(qi::AnyReference(u));
  std::string res = fin.call(args).to<std::string>();
  EXPECT_EQ(res, u.str());
}
