/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <map>
#include <qimessaging/binaryencoder.hpp>
#include <qimessaging/binarydecoder.hpp>
#include <qi/buffer.hpp>

#include <limits.h>

TEST(TestBind, serializeInt)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  d.write(12);

  qi::BinaryDecoder  d2(buf);
  int i;
  d2.read(i);

  EXPECT_EQ(12, i);
}

TEST(TestBind, serializeInts)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  d.write(12);
  d.write(13);
  d.write(14);

  qi::BinaryDecoder  d2(buf);
  int i1;
  d2.read(i1);
  int i2;
  d2.read(i2);
  int i3;
  d2.read(i3);

  EXPECT_EQ(12, i1);
  EXPECT_EQ(13, i2);
  EXPECT_EQ(14, i3);
}

TEST(TestBind, serializeBool)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  d.write(true);

  qi::BinaryDecoder  d2(buf);
  bool b;
  d2.read(b);

  EXPECT_EQ(true, b);
}

TEST(TestBind, serializeBools)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  d.write(true);
  d.write(false);
  d.write(false);

  qi::BinaryDecoder  d2(buf);
  bool b1;
  d2.read(b1);
  bool b2;
  d2.read(b2);
  bool b3;
  d2.read(b3);

  EXPECT_TRUE(b1);
  EXPECT_FALSE(b2);
  EXPECT_FALSE(b3);
}

TEST(TestBind, serializeChar)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  d.write('c');

  qi::BinaryDecoder  d2(buf);
  char b;
  d2.read(b);

  EXPECT_EQ('c', b);
}

TEST(TestBind, serializeChars)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  d.write('c');
  d.write('d');
  d.write('e');

  qi::BinaryDecoder  d2(buf);
  char b1;
  d2.read(b1);
  char b2;
  d2.read(b2);
  char b3;
  d2.read(b3);

  EXPECT_EQ('c', b1);
  EXPECT_EQ('d', b2);
  EXPECT_EQ('e', b3);
}

TEST(TestBind, serializeUInt)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  unsigned int ui1 = UINT_MAX;
  d.write(ui1);

  qi::BinaryDecoder  d2(buf);
  unsigned int ui2;
  d2.read(ui2);

  EXPECT_EQ(ui1, ui2);
}

TEST(TestBind, serializeUInts)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  unsigned int ui1 = UINT_MAX;
  unsigned int ui2 = 0;
  unsigned int ui3 = 456789;
  d.write(ui1);
  d.write(ui2);
  d.write(ui3);

  qi::BinaryDecoder  d2(buf);
  unsigned int ui4;
  d2.read(ui4);
  unsigned int ui5;
  d2.read(ui5);
  unsigned int ui6;
  d2.read(ui6);

  EXPECT_EQ(ui1, ui4);
  EXPECT_EQ(ui2, ui5);
  EXPECT_EQ(ui3, ui6);
}

TEST(TestBind, serializeUChar)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  unsigned char ui1 = UCHAR_MAX;
  d.write(ui1);

  qi::BinaryDecoder  d2(buf);
  unsigned char ui2;
  d2.read(ui2);

  EXPECT_EQ(ui1, ui2);
}

TEST(TestBind, serializeUChars)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  unsigned char ui1 = UCHAR_MAX;
  unsigned char ui2 = 0;
  unsigned char ui3 = 45;
  d.write(ui1);
  d.write(ui2);
  d.write(ui3);

  qi::BinaryDecoder  d2(buf);
  unsigned char ui4;
  d2.read(ui4);
  unsigned char ui5;
  d2.read(ui5);
  unsigned char ui6;
  d2.read(ui6);

  EXPECT_EQ(ui1, ui4);
  EXPECT_EQ(ui2, ui5);
  EXPECT_EQ(ui3, ui6);
}

TEST(TestBind, serializeFloat)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  float f = 1.25f;
  d.write(f);

  qi::BinaryDecoder  d2(buf);
  float b;
  d2.read(b);

  EXPECT_EQ(f, b);
}

TEST(TestBind, serializeFloats)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  float f1 = 1.25f;
  float f2 = -1.25f;
  float f3 = 0.0f;
  d.write(f1);
  d.write(f2);
  d.write(f3);

  qi::BinaryDecoder  d2(buf);
  float b1;
  d2.read(b1);
  float b2;
  d2.read(b2);
  float b3;
  d2.read(b3);

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}


TEST(TestBind, serializeDouble)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  double f = 1.25;
  d.write(f);

  qi::BinaryDecoder  d2(buf);
  double b;
  d2.read(b);

  EXPECT_EQ(f, b);
}

TEST(TestBind, serializeDoubles)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  double f1 = 1.25;
  double f2 = -1.25;
  double f3 = 0.0;
  d.write(f1);
  d.write(f2);
  d.write(f3);

  qi::BinaryDecoder  d2(buf);
  double b1;
  d2.read(b1);
  double b2;
  d2.read(b2);
  double b3;
  d2.read(b3);

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}

TEST(TestBind, serializeString)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  std::string s = "1.25";
  d.write(s);

  qi::BinaryDecoder  d2(buf);
  std::string s2;
  d2.read(s2);

  EXPECT_EQ(s, s2);
}

TEST(TestBind, serializeStrings)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  std::string f1 = "1.25";
  std::string f2 = "-1.25";
  std::string f3 = "0.0";
  d.write(f1);
  d.write(f2);
  d.write(f3);

  qi::BinaryDecoder  d2(buf);
  std::string b1;
  d2.read(b1);
  std::string b2;
  d2.read(b2);
  std::string b3;
  d2.read(b3);

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}

TEST(TestBind, serializeCChar)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  std::string s = "1.25";
  d.write(s.c_str());

  qi::BinaryDecoder  d2(buf);
  std::string s2;
  d2.read(s2);

  EXPECT_EQ(s, s2);
}

TEST(TestBind, serializeCChars)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  std::string f1 = "1.25";
  std::string f2 = "-1.25";
  std::string f3 = "0.0";
  d.write(f1.c_str());
  d.write(f2.c_str());
  d.write(f3.c_str());

  qi::BinaryDecoder  d2(buf);
  std::string b1;
  d2.read(b1);
  std::string b2;
  d2.read(b2);
  std::string b3;
  d2.read(b3);

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}

TEST(TestBind, serializeVectorString)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  std::vector<std::string> vs;
  vs.push_back("toto");
  vs.push_back("tutu");
  vs.push_back("tata");
  d.write(vs);

  qi::BinaryDecoder  d2(buf);
  std::vector<std::string> vs1;
  d2.read(vs1);

  EXPECT_EQ(vs[0], vs1[0]);
  EXPECT_EQ(vs[1], vs1[1]);
  EXPECT_EQ(vs[2], vs1[2]);
}

TEST(TestBind, serializeVectorStrings)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  d(buf);
  std::vector<std::string> vs;
  vs.push_back("toto");
  vs.push_back("tutu");
  vs.push_back("tata");

  d.write(vs);
  d.write(vs);

  qi::BinaryDecoder  d2(buf);
  std::vector<std::string> vs1;
  d2.read(vs1);
  std::vector<std::string> vs2;
  d2.read(vs2);

  EXPECT_EQ(vs[0], vs1[0]);
  EXPECT_EQ(vs[1], vs1[1]);
  EXPECT_EQ(vs[2], vs1[2]);
  EXPECT_EQ(vs[0], vs2[0]);
  EXPECT_EQ(vs[1], vs2[1]);
  EXPECT_EQ(vs[2], vs2[2]);
}

TEST(TestBind, serializeBuffer)
{
  qi::Buffer buf;
  qi::BinaryEncoder e(buf);
  qi::Buffer buffer;
  int a = 42;
  buffer.write(&a, sizeof(int));
  e.write(buffer);

  qi::BinaryDecoder d(buf);
  qi::Buffer buffer2;
  d.read(buffer2);
  int b;
  buffer2.read(&b, 0, sizeof(int));

  EXPECT_EQ(a, b);

}

TEST(TestBind, serializeAllTypes)
{
  qi::Buffer      buf;
  qi::BinaryEncoder  ds(buf);

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

  ds.write(b);
  ds.write(c);
  ds.write(sh);
  ds.write(i);
  ds.write(l);
  //ds.write(ll);
  ds.write(uc);
  ds.write(ush);
  ds.write(ui);
  ds.write(ul);
  //ds.write(ull);
  ds.write(f);
  ds.write(d);
  ds.write(s);
  ds.write(s.c_str());

  qi::BinaryDecoder  d2(buf);
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


  d2.read(b1);
  d2.read(c1);
  d2.read(sh1);
  d2.read(i1);
  d2.read(l1);
  //d2.read(ll1);
  d2.read(uc1);
  d2.read(ush1);
  d2.read(ui1);
  d2.read(ul1);
  //d2.read(ull1);
  d2.read(f1);
  d2.read(d1);
  d2.read(s1);
  d2.read(s2);

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
  qi::BinaryEncoder dout(buf);
  dout.write(p);
  qi::BinaryDecoder din(buf);
  Point pout;
  din.read(pout);
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
  qi::BinaryEncoder dout(buf);
  dout.write(comp);
  qi::BinaryDecoder din(buf);
  Complex compout;
  din.read(compout);
  ASSERT_EQ(comp, compout);
}

//compilation of weird case. C++ typesystem Hell.
TEST(TestBind, TestShPtr) {
  boost::shared_ptr<int> sh1;
  boost::shared_ptr<int> sh2(new int);
  qi::Buffer bif;
  qi::BinaryEncoder dou(bif);
  bool b1;
  bool b2;

  dou.write(sh1);
  dou.write(sh2);
  qi::BinaryDecoder din(bif);
  din.read(b1);
  din.read(b2);

  typedef void(*titi_t)(void);
  titi_t titi = (titi_t)1;

  dou.write(titi);
}
