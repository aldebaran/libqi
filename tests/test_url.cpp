/*
 *  Author(s):
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <string>

#include <gtest/gtest.h>

#include <qitype/type.hpp>
#include <qitype/functiontypefactory.hpp>
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
  qi::Type* t = qi::typeOf<qi::Url>();
  EXPECT_EQ(qi::Type::String, t->kind());
  qi::GenericFunction fout = qi::makeGenericFunction(&urlOut);
  qi::GenericFunction fin = qi::makeGenericFunction(&urlIn);
  std::vector<qi::GenericValuePtr> args;
  std::string s("tcp://canard:0");
  EXPECT_EQ(s, qi::GenericValueRef(s).to<qi::Url>().str());
  args.push_back(qi::GenericValueRef(s));
  qi::Url u = fout.call(args).to<qi::Url>();
  EXPECT_EQ(s, u.str());
  EXPECT_EQ(u, qi::Url(qi::GenericValueRef(u).to<std::string>()));
  args.clear();
  args.push_back(qi::GenericValueRef(u));
  std::string res = fin.call(args).to<std::string>();
  EXPECT_EQ(res, u.str());
}
