/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <map>
#include <qimessaging/datastream.hpp>
#include <qimessaging/buffer.hpp>

#include <limits.h>

TEST(TestBind, serializeInt)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  d << 12;

  qi::DataStream  d2(buf);
  int i;
  d2 >> i;

  EXPECT_EQ(12, i);
}

TEST(TestBind, serializeInts)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  d << 12;
  d << 13;
  d << 14;

  qi::DataStream  d2(buf);
  int i1;
  d2 >> i1;
  int i2;
  d2 >> i2;
  int i3;
  d2 >> i3;

  EXPECT_EQ(12, i1);
  EXPECT_EQ(13, i2);
  EXPECT_EQ(14, i3);
}

TEST(TestBind, serializeBool)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  d << true;

  qi::DataStream  d2(buf);
  bool b;
  d2 >> b;

  EXPECT_EQ(true, b);
}

TEST(TestBind, serializeBools)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  d << true;
  d << false;
  d << false;

  qi::DataStream  d2(buf);
  bool b1;
  d2 >> b1;
  bool b2;
  d2 >> b2;
  bool b3;
  d2 >> b3;

  EXPECT_TRUE(b1);
  EXPECT_FALSE(b2);
  EXPECT_FALSE(b3);
}

TEST(TestBind, serializeChar)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  d << 'c';

  qi::DataStream  d2(buf);
  char b;
  d2 >> b;

  EXPECT_EQ('c', b);
}

TEST(TestBind, serializeChars)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  d << 'c';
  d << 'd';
  d << 'e';

  qi::DataStream  d2(buf);
  char b1;
  d2 >> b1;
  char b2;
  d2 >> b2;
  char b3;
  d2 >> b3;

  EXPECT_EQ('c', b1);
  EXPECT_EQ('d', b2);
  EXPECT_EQ('e', b3);
}

TEST(TestBind, serializeUInt)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  unsigned int ui1 = UINT_MAX;
  d << ui1;

  qi::DataStream  d2(buf);
  unsigned int ui2;
  d2 >> ui2;

  EXPECT_EQ(ui1, ui2);
}

TEST(TestBind, serializeUInts)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  unsigned int ui1 = UINT_MAX;
  unsigned int ui2 = 0;
  unsigned int ui3 = 456789;
  d << ui1;
  d << ui2;
  d << ui3;

  qi::DataStream  d2(buf);
  unsigned int ui4;
  d2 >> ui4;
  unsigned int ui5;
  d2 >> ui5;
  unsigned int ui6;
  d2 >> ui6;

  EXPECT_EQ(ui1, ui4);
  EXPECT_EQ(ui2, ui5);
  EXPECT_EQ(ui3, ui6);
}

TEST(TestBind, serializeUChar)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  unsigned char ui1 = UCHAR_MAX;
  d << ui1;

  qi::DataStream  d2(buf);
  unsigned char ui2;
  d2 >> ui2;

  EXPECT_EQ(ui1, ui2);
}

TEST(TestBind, serializeUChars)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  unsigned char ui1 = UCHAR_MAX;
  unsigned char ui2 = 0;
  unsigned char ui3 = 45;
  d << ui1;
  d << ui2;
  d << ui3;

  qi::DataStream  d2(buf);
  unsigned char ui4;
  d2 >> ui4;
  unsigned char ui5;
  d2 >> ui5;
  unsigned char ui6;
  d2 >> ui6;

  EXPECT_EQ(ui1, ui4);
  EXPECT_EQ(ui2, ui5);
  EXPECT_EQ(ui3, ui6);
}

TEST(TestBind, serializeFloat)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  float f = 1.25f;
  d << f;

  qi::DataStream  d2(buf);
  float b;
  d2 >> b;

  EXPECT_EQ(f, b);
}

TEST(TestBind, serializeFloats)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  float f1 = 1.25f;
  float f2 = -1.25f;
  float f3 = 0.0f;
  d << f1;
  d << f2;
  d << f3;

  qi::DataStream  d2(buf);
  float b1;
  d2 >> b1;
  float b2;
  d2 >> b2;
  float b3;
  d2 >> b3;

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}


TEST(TestBind, serializeDouble)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  double f = 1.25;
  d << f;

  qi::DataStream  d2(buf);
  double b;
  d2 >> b;

  EXPECT_EQ(f, b);
}

TEST(TestBind, serializeDoubles)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  double f1 = 1.25;
  double f2 = -1.25;
  double f3 = 0.0;
  d << f1;
  d << f2;
  d << f3;

  qi::DataStream  d2(buf);
  double b1;
  d2 >> b1;
  double b2;
  d2 >> b2;
  double b3;
  d2 >> b3;

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}

TEST(TestBind, serializeString)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  std::string s = "1.25";
  d << s;

  qi::DataStream  d2(buf);
  std::string s2;
  d2 >> s2;

  EXPECT_EQ(s, s2);
}

TEST(TestBind, serializeStrings)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  std::string f1 = "1.25";
  std::string f2 = "-1.25";
  std::string f3 = "0.0";
  d << f1;
  d << f2;
  d << f3;

  qi::DataStream  d2(buf);
  std::string b1;
  d2 >> b1;
  std::string b2;
  d2 >> b2;
  std::string b3;
  d2 >> b3;

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}

TEST(TestBind, serializeCChar)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  std::string s = "1.25";
  d << s.c_str();

  qi::DataStream  d2(buf);
  std::string s2;
  d2 >> s2;

  EXPECT_EQ(s, s2);
}

TEST(TestBind, serializeCChars)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  std::string f1 = "1.25";
  std::string f2 = "-1.25";
  std::string f3 = "0.0";
  d << f1.c_str();
  d << f2.c_str();
  d << f3.c_str();

  qi::DataStream  d2(buf);
  std::string b1;
  d2 >> b1;
  std::string b2;
  d2 >> b2;
  std::string b3;
  d2 >> b3;

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}

TEST(TestBind, serializeVectorString)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  std::vector<std::string> vs;
  vs.push_back("toto");
  vs.push_back("tutu");
  vs.push_back("tata");
  d << vs;

  qi::DataStream  d2(buf);
  std::vector<std::string> vs1;
  d2 >> vs1;

  EXPECT_EQ(vs[0], vs1[0]);
  EXPECT_EQ(vs[1], vs1[1]);
  EXPECT_EQ(vs[2], vs1[2]);
}

TEST(TestBind, serializeVectorStrings)
{
  qi::Buffer      buf;
  qi::DataStream  d(buf);
  std::vector<std::string> vs;
  vs.push_back("toto");
  vs.push_back("tutu");
  vs.push_back("tata");

  d << vs;
  d << vs;

  qi::DataStream  d2(buf);
  std::vector<std::string> vs1;
  d2 >> vs1;
  std::vector<std::string> vs2;
  d2 >> vs2;

  EXPECT_EQ(vs[0], vs1[0]);
  EXPECT_EQ(vs[1], vs1[1]);
  EXPECT_EQ(vs[2], vs1[2]);
  EXPECT_EQ(vs[0], vs2[0]);
  EXPECT_EQ(vs[1], vs2[1]);
  EXPECT_EQ(vs[2], vs2[2]);
}

TEST(TestBind, serializeAllTypes)
{
  qi::Buffer      buf;
  qi::DataStream  ds(buf);

  bool b = true;
  char c = 'c';
  int i = 42;
  unsigned char uc = UCHAR_MAX;
  unsigned int ui = UINT_MAX;
  float f = 23.5f;
  double d = 42.42;
  std::string s("test::string");

  ds << b;
  ds << c;
  ds << i;
  ds << uc;
  ds << ui;
  ds << f;
  ds << d;
  ds << s;
  ds << s.c_str();

  qi::DataStream  d2(buf);
  bool b1;
  char c1;
  int i1;
  unsigned char uc1;
  unsigned int ui1;
  float f1;
  double d1;
  std::string s1;
  std::string s2;
  d2 >> b1;
  d2 >> c1;
  d2 >> i1;
  d2 >> uc1;
  d2 >> ui1;
  d2 >> f1;
  d2 >> d1;
  d2 >> s1;
  d2 >> s2;

  EXPECT_EQ(b, b1);
  EXPECT_EQ(c, c1);
  EXPECT_EQ(i, i1);
  EXPECT_EQ(uc, uc1);
  EXPECT_EQ(ui, ui1);
  EXPECT_EQ(f, f1);
  EXPECT_EQ(d, d1);
  EXPECT_EQ(s, s1);
  EXPECT_EQ(s, s2);
}

