/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <qi/log.hpp>
#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/genericobjectbuilder.hpp>
#include <qimessaging/objecttypebuilder.hpp>

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

std::vector<qi::GenericValue> convert(qi::AutoGenericValue v1 = qi::AutoGenericValue(),
  qi::AutoGenericValue v2 = qi::AutoGenericValue(),
  qi::AutoGenericValue v3 = qi::AutoGenericValue(),
  qi::AutoGenericValue v4 = qi::AutoGenericValue(),
  qi::AutoGenericValue v5 = qi::AutoGenericValue())
{
  std::vector<qi::GenericValue> res;
  if (v1.value)
    res.push_back(v1);
  if (v2.value)
    res.push_back(v2);
  if (v3.value)
    res.push_back(v3);
  return res;
}

template<int I>
class EvilPadder
{
public:
  char padding[I];
};

class Incrementer
{
public:
  Incrementer() : inc(1) {}
  // Ensure we use this
  int increment(int v) { return v+inc;}
  int inc;
};

class Adder:
  public EvilPadder<1>, public Incrementer, public EvilPadder<2>
  //public Incrementer
{
public:
  Adder() {
    qiLogDebug("Adder") << "constructor(default) " << this;
  v = -1;
  }
  Adder(const Adder& b)
  {
    qiLogDebug("Adder") << "constructor(copy) " << this;
    v = b.v+1;
  }
  Adder(int v)
  : v(v) {
    qiLogDebug("Adder") << "constructor(int) " << this;
  }
  int add(int v2)
  {
    qiLogDebug("adder") << this <<' ' << v << ' ' << v2;
    return v+v2;
  }
  static int addTwo(int v1, int v2)
  {
    return v1+v2;
  }
  int addAdderByRef(Adder& b) { return v + b.v;}
  int addAdderByConstRef(const Adder& b) { return v + b.v;}
  int addAdderByPtr(Adder* b) { return v + b->v;}
  int addAdderByConstPtr(const Adder* b) { return v + b->v;}
  int v;
};

template<typename T> bool checkValue(qi::GenericValue v, const T& val)
{
  std::pair<const T*, bool> r = v.as<T>();
  if (!r.first)
    return false;
  bool ok = *r.first == val;
  if (!ok)
    qiLogError("checkValue") << "expected: " << val <<"  actual: " << *r.first;
  if (r.second)
    delete r.first;
  return ok;
}

TEST(TestObject, Typing)
{
  qi::GenericFunction fv = qi::makeGenericFunction(&vfun);
  qi::GenericFunction fv2 = qi::makeGenericFunction(&fun);
  qi::GenericMethod mv = qi::makeGenericMethod(&Foo::fun);
  std::vector<qi::GenericValue> args1 = convert(1, 2);
  qi::GenericValue res = fv2.call(args1);
  ASSERT_TRUE(checkValue(res, 3));

  qi::GenericMethod adderAdd = qi::makeGenericMethod(&Adder::add);
  Adder add1(1);
  std::vector<qi::GenericValue> argsAdd = convert(41);
  res = adderAdd.call(qi::makeObjectValue(&add1), argsAdd);
  ASSERT_TRUE(checkValue(res, 42));
}

TEST(TestObject, Simple) {
  Foo                   foo;
  qi::GenericObjectBuilder ob;

  ob.advertiseMethod("test", &fun);
  ob.advertiseMethod("vtest", &vfun);
  ob.advertiseMethod("objtest", &foo, &Foo::fun);
  ob.advertiseMethod("objvtest", &foo, &Foo::vfun);
  ob.advertiseMethod("testBind", (boost::function<int(const int&)>)boost::bind(&fun, 21, _1));
  ob.advertiseMethod("testBind2", (boost::function<int(int)>)boost::bind(&fun, 21, _1));
  ob.advertiseMethod("ptrtest", &ptrfun);
  ob.advertiseMethod("reftest", &reffun);
  ob.advertiseMethod("valuetest", &valuefun);
  qi::GenericObject obj(ob.object());


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

QI_TYPE_SERIALIZABLE(Point)
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
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("swapPoint", &swapPoint);
  qi::GenericObject obj(ob.object());
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

QI_TYPE_SERIALIZABLE(::Complex)
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

  qi::GenericObjectBuilder ob;
  unsigned id = ob.advertiseMethod("echo", &echoBack);
  qi::GenericObject obj(ob.object());
  std::cerr << obj.metaObject().methodMap()[id].signature() << std::endl;
  Complex res = obj.call<Complex>("echo", comp);
  ASSERT_EQ(res, comp);
}

TEST(TestObject, ObjectTypeBuilder)
{
  qi::ObjectTypeBuilder<Adder> builder;
  builder.advertiseMethod("add", &Adder::add);
  builder.advertiseMethod("addTwo", &Adder::addTwo);
  builder.advertiseMethod("addAdderByRef", &Adder::addAdderByRef);
  builder.advertiseMethod("addAdderByConstRef", &Adder::addAdderByConstRef);
  builder.advertiseMethod("addAdderByPtr", &Adder::addAdderByPtr);
  builder.advertiseMethod("addAdderByConstPtr", &Adder::addAdderByConstPtr);
  builder.advertiseMethod("increment", &Adder::increment);
  builder.advertiseMethod("increment2", &Incrementer::increment);
  Adder a1(1);
  Adder a2(2);
  qi::GenericObject oa1 = builder.object(&a1);
  qi::GenericObject oa2 = builder.object(&a2);
  ASSERT_TRUE(!oa1.eventLoop()); // object not manageable
  ASSERT_EQ(2, oa1.call<int>("add", 1));
  ASSERT_EQ(3, oa2.call<int>("add", 1));
  ASSERT_EQ(5, oa1.call<int>("addTwo", 3, 2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByPtr", &a2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByPtr", oa2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByRef", a2));
  //go is T*, not T ASSERT_EQ(3, oa1.call<int>("addAdderByRef", oa2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByConstPtr", &a2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByConstPtr", oa2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByConstRef", a2));
  // same as above ASSERT_EQ(3, oa1.call<int>("addAdderByConstRef", oa2));

  ASSERT_EQ(4, oa1.call<int>("increment", 3));
  ASSERT_EQ(4, oa1.call<int>("increment2", 3));
}


int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
