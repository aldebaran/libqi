/*
 *  Author(s):
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <string>

#include <gtest/gtest.h>

#include <qi/type/typeinterface.hpp>
#include <qi/anyfunction.hpp>
#include <qi/url.hpp>


TEST(TestURL, EmptyUrl)
{
  qi::Url url;

  EXPECT_EQ("", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.protocol());
  EXPECT_FALSE(url.isValid());

  EXPECT_FALSE(url.hasProtocol());
  EXPECT_FALSE(url.hasHost());
  EXPECT_FALSE(url.hasPort());
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

  url = "tcp://example.com:abcdef";
  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_FALSE(url.isValid());

  url = "tcp://example.com:-12";
  EXPECT_EQ("tcp", url.protocol());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_FALSE(url.isValid());

  url = "tcp://example.com:424242";
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
  EXPECT_FALSE(url.isValid());
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
  EXPECT_EQ(s, qi::AnyReference::from(s).to<qi::Url>().str());
  args.push_back(qi::AnyReference::from(s));
  qi::Url u = fout.call(args).to<qi::Url>();
  EXPECT_EQ(s, u.str());
  EXPECT_EQ(u, qi::Url(qi::AnyReference::from(u).to<std::string>()));
  args.clear();
  args.push_back(qi::AnyReference::from(u));
  std::string res = fin.call(args).to<std::string>();
  EXPECT_EQ(res, u.str());
}

TEST(TestURL, SetProtocol)
{
  qi::Url url;
  url.setProtocol("tcps");
  EXPECT_TRUE(url.hasProtocol());
  EXPECT_FALSE(url.hasHost());
  EXPECT_FALSE(url.hasPort());
  EXPECT_FALSE(url.isValid());
}

TEST(TestURL, SetHost)
{
  qi::Url url;
  url.setHost("0.0.0.0");
  EXPECT_FALSE(url.hasProtocol());
  EXPECT_TRUE(url.hasHost());
  EXPECT_FALSE(url.hasPort());
  EXPECT_FALSE(url.isValid());
}

TEST(TestURL, SetPort)
{
  qi::Url url;
  url.setPort(0u);
  EXPECT_FALSE(url.hasProtocol());
  EXPECT_FALSE(url.hasHost());
  EXPECT_TRUE(url.hasPort());
  EXPECT_FALSE(url.isValid());
}

TEST(TestURL, SetAll)
{
  qi::Url url;
  url.setProtocol("tcps");
  url.setHost("0.0.0.0");
  url.setPort(0u);
  EXPECT_TRUE(url.hasProtocol());
  EXPECT_TRUE(url.hasHost());
  EXPECT_TRUE(url.hasPort());
  EXPECT_TRUE(url.isValid());
}

TEST(TestURL, SpecifyUrl)
{
  const qi::Url specific("0.0.0.0:0");
  const qi::Url base("tcp://127.0.0.1:9559");
  const qi::Url result = qi::specifyUrl(specific, base);
  EXPECT_TRUE(result.hasProtocol());
  EXPECT_TRUE(result.hasHost());
  EXPECT_TRUE(result.hasPort());
  EXPECT_TRUE(result.isValid());
  EXPECT_EQ(base.protocol(), result.protocol());
  EXPECT_EQ(specific.host(), result.host());
  EXPECT_EQ(specific.port(), result.port());
}

TEST(TestURL, SpecifyUrlWithIncompleteBase)
{
  const qi::Url specific("0.0.0.0:0");
  const qi::Url base("127.0.0.1:9559");
  const qi::Url result = qi::specifyUrl(specific, base);
  EXPECT_FALSE(result.hasProtocol());
  EXPECT_TRUE(result.hasHost());
  EXPECT_TRUE(result.hasPort());
  EXPECT_FALSE(result.isValid());
  EXPECT_EQ(base.protocol(), result.protocol());
  EXPECT_EQ(specific.host(), result.host());
  EXPECT_EQ(specific.port(), result.port());
}

TEST(TestURL, SpecifyUrlWithEmptyDefault)
{
  const qi::Url specific("tcp://127.0.0.1:9559");
  const qi::Url base;
  const qi::Url result = qi::specifyUrl(specific, base);
  EXPECT_TRUE(result.hasProtocol());
  EXPECT_TRUE(result.hasHost());
  EXPECT_TRUE(result.hasPort());
  EXPECT_TRUE(result.isValid());
  EXPECT_EQ(specific.protocol(), result.protocol());
  EXPECT_EQ(specific.host(), result.host());
  EXPECT_EQ(specific.port(), result.port());
}
