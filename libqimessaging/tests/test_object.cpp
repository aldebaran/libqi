/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <qimessaging/object.hpp>

static int gGlobalResult = 0;

void vfun(const int &p0,const int &p1)   { gGlobalResult = p0 + p1; }
int fun(const int &p0,const int &p1)     { return p0 + p1; }

struct Foo {
  int fun(const int &p0,const int &p1)   { return p0 + p1; }
  void vfun(const int &p0,const int &p1) { gGlobalResult = p0 + p1; }
};

class C
{
public:
  bool operator == (const C& b) const { return baz == b.baz;}
  C() : baz(0) {}
  C(int v) : baz(v) {}
  int baz;
};

C* ptrfun(C* ptr) { return ptr;}
C& reffun(const C& ref) { return const_cast<C&>(ref);}
C valuefun(C val) { return val;}

TEST(TestObject, Simple) {
  qi::Object obj;
  Foo        foo;

  obj.advertiseMethod("test", &fun);
  obj.advertiseMethod("vtest", &vfun);
  obj.advertiseMethod("objtest", &foo, &Foo::fun);
  obj.advertiseMethod("objvtest", &foo, &Foo::vfun);
  obj.advertiseMethod("testBind", (boost::function<int(const int&)>)boost::bind(&fun, 21, _1));
  obj.advertiseMethod("testBind2", (boost::function<int(int)>)boost::bind(&fun, 21, _1));
  obj.advertiseMethod("ptrtest", &ptrfun);
  obj.advertiseMethod("reftest", &reffun);
  obj.advertiseMethod("valuetest", &valuefun);

  EXPECT_EQ(42, obj.call<int>("test", 21, 21));
  EXPECT_EQ(42, obj.call<int>("objtest", 21, 21));
  EXPECT_EQ(42, obj.call<int>("testBind", 21));
  EXPECT_EQ(42, obj.call<int>("testBind2", 21));
  EXPECT_EQ(static_cast<unsigned int>(42), obj.call<unsigned int>("test", 21, 21));
  EXPECT_EQ(42, obj.call<char>("test", 21, 21));
  EXPECT_EQ(42, obj.call<double>("test", 21, 21));
  EXPECT_EQ(42, obj.call<long>("test", 21, 21));
  EXPECT_EQ(42UL, obj.call<unsigned long>("test", 21, 21));

#ifndef QI_REQUIRE_SIGNATURE_EXACT_MATCH
  EXPECT_EQ(42, obj.call<int>("test", (char)21, 21));
  EXPECT_EQ(42, obj.call<int>("test", (unsigned char)21, 21));
  EXPECT_EQ(42, obj.call<int>("test", (short)21, 21));
  EXPECT_EQ(42, obj.call<int>("test", (unsigned short)21, 21));
  EXPECT_EQ(42, obj.call<int>("test", (float)21, 21));
  EXPECT_EQ(42, obj.call<int>("test", (double)21, 21));
    EXPECT_EQ(42, obj.call<int>("test", (long)21, 21));
#endif

  gGlobalResult = 0;
  obj.call<void>("vtest", 21, 21).wait();
  EXPECT_EQ(42, gGlobalResult);
  gGlobalResult = 0;
  obj.call<void>("objvtest", 21, 21).wait();
  EXPECT_EQ(42, gGlobalResult);

  C f(42);
  EXPECT_EQ(&f, obj.call<C*>("ptrtest", &f).value());
  EXPECT_EQ(f, obj.call<C>("valuetest", f).value());
}

struct Point
{
  bool operator == (const Point& b) const { return x==b.x && y==b.y;}
  int x, y;
};

Point point(int x, int y)
{
  Point p; p.x = x; p.y = y; return p;
}

QI_METATYPE_SERIALIZABLE(Point)
QI_REGISTER_STRUCT(Point, x, y);

struct Test
{
  float x;
};

/// Test the split form of the macro.
QI_REGISTER_STRUCT_DECLARE(Test)
QI_REGISTER_STRUCT_IMPLEMENT(Test, x)



Point swapPoint(const Point& b)
{
  return point(b.y, b.x);
}

TEST(TestObject, SerializeSimple)
{
  qi::Object obj;
  obj.advertiseMethod("swapPoint", &swapPoint);
  Point p;
  p.x = 1; p.y = 2;
  Point res = obj.call<Point>("swapPoint", p);
  ASSERT_EQ(2, res.x);
  ASSERT_EQ(1, res.y);
}


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

QI_METATYPE_SERIALIZABLE(::Complex)
// Test the sub macros
QI_DATASTREAM_STRUCT(Complex, points, foo, baz, stuff)
QI_SIGNATURE_STRUCT(Complex, points, foo, baz, stuff)


Complex echoBack(const Complex& c)
{
  return c;
}

TEST(TestObject, SerializeComplex)
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

  qi::Object obj;
  unsigned id = obj.advertiseMethod("echo", &echoBack);
  std::cerr << obj.metaObject().methodMap()[id].signature() << std::endl;
  Complex res = obj.call<Complex>("echo", comp);
  ASSERT_EQ(res, comp);
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
