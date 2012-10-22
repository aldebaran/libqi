/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <map>
#include <qimessaging/datastream.hpp>
#include <qi/buffer.hpp>

#include <limits.h>

TEST(TestBind, serializeInt)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  d << 12;

  qi::IDataStream  d2(buf);
  int i;
  d2 >> i;

  EXPECT_EQ(12, i);
}

TEST(TestBind, serializeInts)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  d << 12;
  d << 13;
  d << 14;

  qi::IDataStream  d2(buf);
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
  qi::ODataStream  d(buf);
  d << true;

  qi::IDataStream  d2(buf);
  bool b;
  d2 >> b;

  EXPECT_EQ(true, b);
}

TEST(TestBind, serializeBools)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  d << true;
  d << false;
  d << false;

  qi::IDataStream  d2(buf);
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
  qi::ODataStream  d(buf);
  d << 'c';

  qi::IDataStream  d2(buf);
  char b;
  d2 >> b;

  EXPECT_EQ('c', b);
}

TEST(TestBind, serializeChars)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  d << 'c';
  d << 'd';
  d << 'e';

  qi::IDataStream  d2(buf);
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
  qi::ODataStream  d(buf);
  unsigned int ui1 = UINT_MAX;
  d << ui1;

  qi::IDataStream  d2(buf);
  unsigned int ui2;
  d2 >> ui2;

  EXPECT_EQ(ui1, ui2);
}

TEST(TestBind, serializeUInts)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  unsigned int ui1 = UINT_MAX;
  unsigned int ui2 = 0;
  unsigned int ui3 = 456789;
  d << ui1;
  d << ui2;
  d << ui3;

  qi::IDataStream  d2(buf);
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
  qi::ODataStream  d(buf);
  unsigned char ui1 = UCHAR_MAX;
  d << ui1;

  qi::IDataStream  d2(buf);
  unsigned char ui2;
  d2 >> ui2;

  EXPECT_EQ(ui1, ui2);
}

TEST(TestBind, serializeUChars)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  unsigned char ui1 = UCHAR_MAX;
  unsigned char ui2 = 0;
  unsigned char ui3 = 45;
  d << ui1;
  d << ui2;
  d << ui3;

  qi::IDataStream  d2(buf);
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
  qi::ODataStream  d(buf);
  float f = 1.25f;
  d << f;

  qi::IDataStream  d2(buf);
  float b;
  d2 >> b;

  EXPECT_EQ(f, b);
}

TEST(TestBind, serializeFloats)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  float f1 = 1.25f;
  float f2 = -1.25f;
  float f3 = 0.0f;
  d << f1;
  d << f2;
  d << f3;

  qi::IDataStream  d2(buf);
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
  qi::ODataStream  d(buf);
  double f = 1.25;
  d << f;

  qi::IDataStream  d2(buf);
  double b;
  d2 >> b;

  EXPECT_EQ(f, b);
}

TEST(TestBind, serializeDoubles)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  double f1 = 1.25;
  double f2 = -1.25;
  double f3 = 0.0;
  d << f1;
  d << f2;
  d << f3;

  qi::IDataStream  d2(buf);
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
  qi::ODataStream  d(buf);
  std::string s = "1.25";
  d << s;

  qi::IDataStream  d2(buf);
  std::string s2;
  d2 >> s2;

  EXPECT_EQ(s, s2);
}

TEST(TestBind, serializeStrings)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  std::string f1 = "1.25";
  std::string f2 = "-1.25";
  std::string f3 = "0.0";
  d << f1;
  d << f2;
  d << f3;

  qi::IDataStream  d2(buf);
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
  qi::ODataStream  d(buf);
  std::string s = "1.25";
  d << s.c_str();

  qi::IDataStream  d2(buf);
  std::string s2;
  d2 >> s2;

  EXPECT_EQ(s, s2);
}

TEST(TestBind, serializeCChars)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  std::string f1 = "1.25";
  std::string f2 = "-1.25";
  std::string f3 = "0.0";
  d << f1.c_str();
  d << f2.c_str();
  d << f3.c_str();

  qi::IDataStream  d2(buf);
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
  qi::ODataStream  d(buf);
  std::vector<std::string> vs;
  vs.push_back("toto");
  vs.push_back("tutu");
  vs.push_back("tata");
  d << vs;

  qi::IDataStream  d2(buf);
  std::vector<std::string> vs1;
  d2 >> vs1;

  EXPECT_EQ(vs[0], vs1[0]);
  EXPECT_EQ(vs[1], vs1[1]);
  EXPECT_EQ(vs[2], vs1[2]);
}

TEST(TestBind, serializeVectorStrings)
{
  qi::Buffer      buf;
  qi::ODataStream  d(buf);
  std::vector<std::string> vs;
  vs.push_back("toto");
  vs.push_back("tutu");
  vs.push_back("tata");

  d << vs;
  d << vs;

  qi::IDataStream  d2(buf);
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
  qi::ODataStream  ds(buf);

  bool b = true;
  char c = 'c';
  unsigned char uc = UCHAR_MAX;
  short sh = SHRT_MAX;
  unsigned short ush = USHRT_MAX;
  int i = 42;
  unsigned int ui = UINT_MAX;
  long l = LONG_MAX;
  unsigned long ul = ULONG_MAX;
//  long long ll = LONG_LONG_MAX;
//  unsigned long long ull = ULONG_LONG_MAX;
  float f = 23.5f;
  double d = 42.42;
  std::string s("test::string");

  ds << b;
  ds << c;
  ds << sh;
  ds << i;
  ds << l;
  //ds << ll;
  ds << uc;
  ds << ush;
  ds << ui;
  ds << ul;
  //ds << ull;
  ds << f;
  ds << d;
  ds << s;
  ds << s.c_str();

  qi::IDataStream  d2(buf);
  bool b1;
  char c1;
  unsigned char uc1;
  short sh1;
  unsigned short ush1;
  int i1;
  unsigned int ui1;
  long l1;
  unsigned long ul1;
//  long long ll1;
//  unsigned long long ull1;
  float f1;
  double d1;
  std::string s1;
  std::string s2;


  d2 >> b1;
  d2 >> c1;
  d2 >> sh1;
  d2 >> i1;
  d2 >> l1;
  //d2 >> ll1;
  d2 >> uc1;
  d2 >> ush1;
  d2 >> ui1;
  d2 >> ul1;
  //d2 >> ull1;
  d2 >> f1;
  d2 >> d1;
  d2 >> s1;
  d2 >> s2;

  EXPECT_EQ(b, b1);
  EXPECT_EQ(c, c1);
  EXPECT_EQ(sh, sh1);
  EXPECT_EQ(i, i1);
  EXPECT_EQ(l, l1);
  //EXPECT_EQ(ll, ll1);

  EXPECT_EQ(uc, uc1);
  EXPECT_EQ(ush, ush1);
  EXPECT_EQ(ui, ui1);
  EXPECT_EQ(ul, ul1);
  //EXPECT_EQ(ull, ull1);
  EXPECT_EQ(f, f1);
  EXPECT_EQ(d, d1);
  EXPECT_EQ(s, s1);
  EXPECT_EQ(s, s2);
}

struct Point
{
  bool operator == (const Point& b) const { return x==b.x && y==b.y;}
  int x, y;
};

QI_TYPE_STRUCT(Point, x, y);

struct Complex
{
  bool operator == (const Complex& b) const {
    return points == b.points
    && foo == b.foo
    && baz == b.baz
    && stuff == b.stuff;
  }
  std::vector<Point> points;
  float foo;
  std::string baz;
  std::list<std::vector<int> > stuff;
};

QI_TYPE_STRUCT(Complex, points, foo, baz, stuff)

TEST(TestBind, SerializeCustomSimple)
{
  Point p;
  p.x = 12; p.y = 13;
  qi::Buffer buf;
  qi::ODataStream dout(buf);
  dout << p;
  qi::IDataStream din(buf);
  Point pout;
  din >> pout;
  ASSERT_EQ(p, pout);
}
Point point(int x, int y)
{
  Point p; p.x = x; p.y = y; return p;
}
TEST(TestBind, SerializeCustomComplex)
{
  Complex comp;
  comp.foo = 1.5;
  comp.points.push_back(point(1, 2));
  comp.points.push_back(point(3, 4));
  comp.baz = "testbaz";
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  comp.stuff.push_back(v);
  v.push_back(3);
  comp.stuff.push_back(v);
  qi::Buffer buf;
  qi::ODataStream dout(buf);
  dout << comp;
  qi::IDataStream din(buf);
  Complex compout;
  din >> compout;
  ASSERT_EQ(comp, compout);
}

//compilation of weird case. C++ typesystem Hell.
TEST(TestBind, TestShPtr) {
  boost::shared_ptr<int> sh1;
  boost::shared_ptr<int> sh2(new int);
  qi::Buffer bif;
  qi::ODataStream dou(bif);
  bool b1;
  bool b2;

  dou << sh1;
  dou << sh2;
  qi::IDataStream din(bif);
  din >> b1;
  din >> b2;

  typedef void(*titi_t)(void);
  titi_t titi = (titi_t)1;

  dou << titi;
}
