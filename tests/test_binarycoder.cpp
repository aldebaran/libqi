/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <map>
#include <qi/buffer.hpp>
#include <qi/bufferreader.hpp>
#include <qimessaging/binarycodec.hpp>
#include <qimessaging/session.hpp>
#include <limits.h>

TEST(TestBind, serializeInt)
{
  qi::Buffer buf;
  qi::BufferReader bufr(buf);
  qi::encodeBinary(&buf, 12);

  int i;
  qi::decodeBinary(&bufr, &i);
  EXPECT_EQ(12, i);
}

TEST(TestBind, serializeInts)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  qi::encodeBinary(&buf, 12);
  qi::encodeBinary(&buf, 13);
  qi::encodeBinary(&buf, 14);

  int i1;
  qi::decodeBinary(&bufr, &i1);
  int i2;
  qi::decodeBinary(&bufr, &i2);
  int i3;
  qi::decodeBinary(&bufr, &i3);

  EXPECT_EQ(12, i1);
  EXPECT_EQ(13, i2);
  EXPECT_EQ(14, i3);
}

TEST(TestBind, serializeBool)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  qi::encodeBinary(&buf, true);

  bool b;
  qi::decodeBinary(&bufr, &b);

  EXPECT_EQ(true, b);
}

TEST(TestBind, serializeBools)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  qi::encodeBinary(&buf, true);
  qi::encodeBinary(&buf, false);
  qi::encodeBinary(&buf, false);

  bool b1;
  qi::decodeBinary(&bufr, &b1);
  bool b2;
  qi::decodeBinary(&bufr, &b2);
  bool b3;
  qi::decodeBinary(&bufr, &b3);

  EXPECT_TRUE(b1);
  EXPECT_FALSE(b2);
  EXPECT_FALSE(b3);
}

TEST(TestBind, serializeChar)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  char c = 'c';
  qi::encodeBinary(&buf, c);

  char b;
  qi::decodeBinary(&bufr, &b);

  EXPECT_EQ('c', b);
}

TEST(TestBind, serializeChars)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  char c1 = 'c';
  qi::encodeBinary(&buf, 'c');
  qi::encodeBinary(&buf, 'd');
  qi::encodeBinary(&buf, 'e');

  char b1;
  qi::decodeBinary(&bufr, &b1);
  char b2;
  qi::decodeBinary(&bufr, &b2);
  char b3;
  qi::decodeBinary(&bufr, &b3);

  EXPECT_EQ('c', b1);
  EXPECT_EQ('d', b2);
  EXPECT_EQ('e', b3);
}

TEST(TestBind, serializeUInt)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  unsigned int ui1 = UINT_MAX;
  qi::encodeBinary(&buf, ui1);

  unsigned int ui2;
  qi::decodeBinary(&bufr, &ui2);

  EXPECT_EQ(ui1, ui2);
}

TEST(TestBind, serializeUInts)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  unsigned int ui1 = UINT_MAX;
  unsigned int ui2 = 0;
  unsigned int ui3 = 456789;
  qi::encodeBinary(&buf, ui1);
  qi::encodeBinary(&buf, ui2);
  qi::encodeBinary(&buf, ui3);

  unsigned int ui4;
  qi::decodeBinary(&bufr, &ui4);
  unsigned int ui5;
  qi::decodeBinary(&bufr, &ui5);
  unsigned int ui6;
  qi::decodeBinary(&bufr, &ui6);

  EXPECT_EQ(ui1, ui4);
  EXPECT_EQ(ui2, ui5);
  EXPECT_EQ(ui3, ui6);
}

TEST(TestBind, serializeUChar)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  unsigned char ui1 = UCHAR_MAX;
  qi::encodeBinary(&buf, ui1);

  unsigned char ui2;
  qi::decodeBinary(&bufr, &ui2);

  EXPECT_EQ(ui1, ui2);
}

TEST(TestBind, serializeUChars)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  unsigned char ui1 = UCHAR_MAX;
  unsigned char ui2 = 0;
  unsigned char ui3 = 45;
  qi::encodeBinary(&buf, ui1);
  qi::encodeBinary(&buf, ui2);
  qi::encodeBinary(&buf, ui3);

  unsigned char ui4;
  qi::decodeBinary(&bufr, &ui4);
  unsigned char ui5;
  qi::decodeBinary(&bufr, &ui5);
  unsigned char ui6;
  qi::decodeBinary(&bufr, &ui6);

  EXPECT_EQ(ui1, ui4);
  EXPECT_EQ(ui2, ui5);
  EXPECT_EQ(ui3, ui6);
}

TEST(TestBind, serializeFloat)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  float f = 1.25f;
  qi::encodeBinary(&buf, f);

  float b;
  qi::decodeBinary(&bufr, &b);

  EXPECT_EQ(f, b);
}

TEST(TestBind, serializeFloats)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  float f1 = 1.25f;
  float f2 = -1.25f;
  float f3 = 0.0f;
  qi::encodeBinary(&buf, f1);
  qi::encodeBinary(&buf, f2);
  qi::encodeBinary(&buf, f3);

  float b1;
  qi::decodeBinary(&bufr, &b1);
  float b2;
  qi::decodeBinary(&bufr, &b2);
  float b3;
  qi::decodeBinary(&bufr, &b3);

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}


TEST(TestBind, serializeDouble)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  double f = 1.25;
  qi::encodeBinary(&buf, f);

  double b;
  qi::decodeBinary(&bufr, &b);

  EXPECT_EQ(f, b);
}

TEST(TestBind, serializeDoubles)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  double f1 = 1.25;
  double f2 = -1.25;
  double f3 = 0.0;
  qi::encodeBinary(&buf, f1);
  qi::encodeBinary(&buf, f2);
  qi::encodeBinary(&buf, f3);

  double b1;
  qi::decodeBinary(&bufr, &b1);
  double b2;
  qi::decodeBinary(&bufr, &b2);
  double b3;
  qi::decodeBinary(&bufr, &b3);

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}

TEST(TestBind, serializeString)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  std::string s = "1.25";
  qi::encodeBinary(&buf, s);

  std::string s2;
  qi::decodeBinary(&bufr, &s2);

  EXPECT_EQ(s, s2);
}

TEST(TestBind, serializeStrings)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  std::string f1 = "1.25";
  std::string f2 = "-1.25";
  std::string f3 = "0.0";
  qi::encodeBinary(&buf, f1);
  qi::encodeBinary(&buf, f2);
  qi::encodeBinary(&buf, f3);

  std::string b1;
  qi::decodeBinary(&bufr, &b1);
  std::string b2;
  qi::decodeBinary(&bufr, &b2);
  std::string b3;
  qi::decodeBinary(&bufr, &b3);

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}

TEST(TestBind, serializeCChar)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  std::string s = "1.25";
  qi::encodeBinary(&buf, s.c_str());

  std::string s2;
  qi::decodeBinary(&bufr, &s2);

  EXPECT_EQ(s, s2);
}

TEST(TestBind, serializeCChars)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  std::string f1 = "1.25";
  std::string f2 = "-1.25";
  std::string f3 = "0.0";
  qi::encodeBinary(&buf, f1.c_str());
  qi::encodeBinary(&buf, f2.c_str());
  qi::encodeBinary(&buf, f3.c_str());

  std::string b1;
  qi::decodeBinary(&bufr, &b1);
  std::string b2;
  qi::decodeBinary(&bufr, &b2);
  std::string b3;
  qi::decodeBinary(&bufr, &b3);

  EXPECT_EQ(f1, b1);
  EXPECT_EQ(f2, b2);
  EXPECT_EQ(f3, b3);
}

TEST(TestBind, serializeVectorString)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  std::vector<std::string> vs;
  vs.push_back("toto");
  vs.push_back("tutu");
  vs.push_back("tata");
  qi::encodeBinary(&buf, vs);

  std::vector<std::string> vs1;
  qi::decodeBinary(&bufr, &vs1);

  EXPECT_EQ(vs[0], vs1[0]);
  EXPECT_EQ(vs[1], vs1[1]);
  EXPECT_EQ(vs[2], vs1[2]);
}

TEST(TestBind, serializeVectorStrings)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);
  std::vector<std::string> vs;
  vs.push_back("toto");
  vs.push_back("tutu");
  vs.push_back("tata");

  qi::encodeBinary(&buf, vs);
  qi::encodeBinary(&buf, vs);

  std::vector<std::string> vs1;
  qi::decodeBinary(&bufr, &vs1);
  std::vector<std::string> vs2;
  qi::decodeBinary(&bufr, &vs2);

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
  qi::Buffer buffer;
  qi::BufferReader bufr(buf);
  int a = 42;
  buffer.write(&a, sizeof(int));
  qi::encodeBinary(&buf, buffer);

  qi::Buffer buffer2;
  qi::decodeBinary(&bufr, &buffer2);
  int b;
  buffer2.read(&b, 0, sizeof(int));

  EXPECT_EQ(a, b);

}

TEST(TestBind, serializeAllTypes)
{
  qi::Buffer      buf;
  qi::BufferReader bufr(buf);

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

  qi::encodeBinary(&buf, b);
  qi::encodeBinary(&buf, c);
  qi::encodeBinary(&buf, sh);
  qi::encodeBinary(&buf, i);
  qi::encodeBinary(&buf, l);
  //qi::encodeBinary(&buf, ll);
  qi::encodeBinary(&buf, uc);
  qi::encodeBinary(&buf, ush);
  qi::encodeBinary(&buf, ui);
  qi::encodeBinary(&buf, ul);
  //qi::encodeBinary(&buf, ull);
  qi::encodeBinary(&buf, f);
  qi::encodeBinary(&buf, d);
  qi::encodeBinary(&buf, s);
  qi::encodeBinary(&buf, s.c_str());

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


  qi::decodeBinary(&bufr, &b1);
  qi::decodeBinary(&bufr, &c1);
  qi::decodeBinary(&bufr, &sh1);
  qi::decodeBinary(&bufr, &i1);
  qi::decodeBinary(&bufr, &l1);
  //qi::decodeBinary(&bufr, &ll1);
  qi::decodeBinary(&bufr, &uc1);
  qi::decodeBinary(&bufr, &ush1);
  qi::decodeBinary(&bufr, &ui1);
  qi::decodeBinary(&bufr, &ul1);
  //qi::decodeBinary(&bufr, &ull1);
  qi::decodeBinary(&bufr, &f1);
  qi::decodeBinary(&bufr, &d1);
  qi::decodeBinary(&bufr, &s1);
  qi::decodeBinary(&bufr, &s2);

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
  qi::BufferReader bufr(buf);
  qi::encodeBinary(&buf, p);
  Point pout;
  qi::decodeBinary(&bufr, &pout);
  ASSERT_EQ(p, pout);
}

Point point(int x, int y)
{
  Point p;
  p.x = x;
  p.y = y;
  return p;
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
  qi::BufferReader bufr(buf);
  qi::encodeBinary(&buf, comp);
  Complex compout;
  qi::decodeBinary(&bufr, &compout);
  ASSERT_EQ(comp, compout);
}

//compilation of weird case. C++ typesystem Hell.
TEST(TestBind, TestShPtr) {
  boost::shared_ptr<int> sh1;
  boost::shared_ptr<int> sh2(new int);
  qi::Buffer buf;
  qi::BufferReader bufr(buf);
  bool b1;
  bool b2;

  qi::encodeBinary(&buf, sh1);
  qi::encodeBinary(&buf, sh2);

  qi::decodeBinary(&bufr, &b1);
  qi::decodeBinary(&bufr, &b2);

  typedef void(*titi_t)(void);
  titi_t titi = (titi_t)1;

  qi::encodeBinary(&buf, titi);
}


//(ii)
struct Point2D {
public:
  Point2D(int x = 0, int y = 0)
    : _x(x), _y(y)
  {}
  int x() { return _x; }
  int y() { return _y; }

  QI_TYPE_STRUCT_PRIVATE_ACCESS(Point2D);
private:
  int _x;
  int _y;
};
QI_TYPE_STRUCT(Point2D, _x, _y);

//(ii)
struct TimeStamp {
  TimeStamp(int i = 0, int j = 0)
    : i(i),
      j(j)
  {}

  int i;
  int j;
};
QI_TYPE_STRUCT(TimeStamp, i, j);


//((ii)(ii))
struct TimeStampedPoint2D {
  TimeStampedPoint2D(int x = 0, int y = 0, int i = 0, int j = 0)
    : p(x, y),
      t(i, j)
  {}

  Point2D   p;
  TimeStamp t;
};
QI_TYPE_STRUCT(TimeStampedPoint2D, p, t);


TEST(testSerializable, Struct) {
  Point2D   p2d1(4, 2), p2d2;
  TimeStamp ts1(3, 1) , ts2;
  TimeStampedPoint2D   tsp2d1(4, 2, 3, 1), tsp2d2;

  qi::Buffer buf;
  qi::BufferReader bufr(buf);
  qi::encodeBinary(&buf, p2d1);
  qi::encodeBinary(&buf, ts1);
  qi::encodeBinary(&buf, tsp2d1);

  qi::decodeBinary(&bufr, &p2d2);
  qi::decodeBinary(&bufr, &ts2);
  qi::decodeBinary(&bufr, &tsp2d2);

  EXPECT_EQ(p2d1.x(), p2d2.x());
  EXPECT_EQ(p2d1.y(), p2d2.y());
  EXPECT_EQ(ts1.i, ts2.i);
  EXPECT_EQ(ts1.j, ts2.j);
  EXPECT_EQ(tsp2d1.p.x(), tsp2d2.p.x());
  EXPECT_EQ(tsp2d1.p.y(), tsp2d2.p.y());
  EXPECT_EQ(tsp2d1.t.i, tsp2d2.t.i);
  EXPECT_EQ(tsp2d1.t.j, tsp2d2.t.j);
  EXPECT_EQ("(ii)", qi::typeOf(p2d1)->signature());
  EXPECT_EQ("(ii)", qi::typeOf(ts1)->signature());
  EXPECT_EQ("((ii)(ii))", qi::typeOf(tsp2d1)->signature());
}
