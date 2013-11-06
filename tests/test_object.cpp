/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/assign/list_of.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qitype/objecttypebuilder.hpp>
#include <qitype/objectfactory.hpp>

#if defined(_MSC_VER) && _MSC_VER <= 1500
// vs2008 32 bits does not have std::abs() on int64
namespace std
{
  qi::int64_t abs(qi::int64_t x)
  {
    if (x < 0)
      return -x;
    else
      return x;
  }
}
#endif

qiLogCategory("test");

static int gGlobalResult = 0;

void vfun(const int &p0,const int &p1)   { gGlobalResult = p0 + p1; }
int fun(const int &p0,const int &p1)     { return p0 + p1; }
int funVal(int p0, int p1) { return p0 + p1;} //by-value

qi::int8_t ping8(qi::int8_t v) { return v;}
qi::int16_t ping16(qi::int16_t v) { return v;}
qi::int32_t ping32(qi::int32_t v) { return v;}
qi::int64_t ping64(qi::int64_t v) { return v;}
float pingFloat(float v) { return v;}
double pingDouble(double v) { return v;}

struct Padding
{
  Padding() : pad(1) {}
  int pad;
};
struct Padding2
{
  Padding2() : pad(-1) {}
  int pad;
};

struct Parent
{
  qi::int32_t pping32(qi::int32_t v) { return v+m;}
  virtual  qi::int32_t vping32(qi::int32_t v) { return v+m;}
  qi::int32_t m;
  Parent() : m(2) {}
  virtual ~Parent() {}
};

// Insert padding to be able to check that class pointers are correctly
// converted when calling member functions
struct Foo: public Padding, public Parent, public Padding2 {
  Foo() : f(3) {}
  Foo(const Foo& b)
  : f(b.f+1) {}
  virtual ~Foo() {}
  void operator = (const Foo& b)
  {
    f = b.f + 1;
  }
  int fun(const int &p0,const int &p1)   { return p0 + p1; }
  void vfun(const int &p0,const int &p1) { gGlobalResult = p0 + p1; }
  qi::int8_t ping8(qi::int8_t v) { return v+f;}
  qi::int16_t ping16(qi::int16_t v) { return v+f;}
  qi::int32_t ping32(qi::int32_t v) { return v+f;}
  qi::int64_t ping64(qi::int64_t v) { return v+f;}
  bool pingB(bool v) { return v;}
  float pingFloat(float v) { return v+(float)f;}
  double pingDouble(double v) { return v+(double)f;}
  virtual  qi::int32_t vping32(qi::int32_t v) { return v+f;}
  void pingV(qi::int32_t v) { r = v+f;}
  int & getRefF() { return f;}
  std::string pingString(std::string s) { return s + (char)('0' + f);}
  const std::string& pingConstString(const std::string& s)
  {
    this->s = s;
    return this->s;
  }
  int f;
  int r;
  std::string s;
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

std::vector<qi::AnyReference> convert(qi::AutoAnyReference v1 = qi::AutoAnyReference(),
  qi::AutoAnyReference v2 = qi::AutoAnyReference(),
  qi::AutoAnyReference v3 = qi::AutoAnyReference(),
  qi::AutoAnyReference v4 = qi::AutoAnyReference(),
  qi::AutoAnyReference v5 = qi::AutoAnyReference())
{
  std::vector<qi::AnyReference> res;
  if (v1.rawValue())
    res.push_back(v1);
  if (v2.rawValue())
    res.push_back(v2);
  if (v3.rawValue())
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
    qiLogDebug() << this << " constructor(default) " << this;
  v = -1;
  }
  Adder(const Adder& b)
  {
    qiLogDebug() << this << " constructor(copy) " << this;
    v = b.v+1;
  }
  Adder(int v)
  : v(v) {
    qiLogDebug() << this << " constructor(int) " << this;
  }
  void operator = (const Adder& b)
  {
    qiLogDebug() << this << " = operator " << &b;
    v = b.v+1;
  }
  int add(int v2)
  {
    qiLogDebug() << this <<' ' << v << ' ' << v2;
    return v+v2;
  }
  static int addTwo(int v1, int v2)
  {
    return v1+v2;
  }
  // NO_SEGV will return a dummy value when 0 ptr instead of segv
  #define CHECK(ptr) if ((!this || !ptr)&&getenv("NO_SEGV")) return -1
  int addAdderByRef(Adder& b) { CHECK(&b); return v + b.v;}
  int addAdderByConstRef(const Adder& b) {  CHECK(&b); return v + b.v;}
  int addAdderByPtr(Adder* b) { CHECK(b); return v + b->v;}
  int addAdderByConstPtr(const Adder* b) { CHECK(b); return v + b->v;}
  #undef CHECK
  int v;
};

template<typename T> bool checkValue(qi::AnyReference v, const T& val)
{
  T actual = v.as<T>();
  bool ok = actual == val;
  if (!ok)
    qiLogError() << "expected: " << val <<"  actual: " << actual;
  return ok;
}

TEST(TestObject, Typing)
{
  qiLogDebug() << "vfun";
  qi::AnyFunction fv = qi::AnyFunction::from(&vfun);
  qiLogDebug() << "fun";
  qi::AnyFunction fv2 = qi::AnyFunction::from(&fun);
  qiLogDebug() << "Foo::fun";
  qi::AnyFunction mv = qi::AnyFunction::from(&Foo::fun);
  std::vector<qi::AnyReference> args1 = convert(1, 2);
  qi::AnyReference res = fv2.call(args1);
  ASSERT_TRUE(checkValue(res, 3));

  qi::AnyFunction adderAdd = qi::AnyFunction::from(&Adder::add);
  Adder add1(1);
  std::vector<qi::AnyReference> argsAdd = convert(41);
  res = adderAdd.call(qi::AnyReference::from(add1), argsAdd);
  ASSERT_TRUE(checkValue(res, 42));
}

TEST(TestObject, ABI)
{
  using namespace qi;
  // We must declare inheritance between Foo and Parent
  ObjectTypeBuilder<Foo> b;
  b.inherits<Parent>();
  b.registerType();
  AnyFunction f;
  f = AnyFunction::from(&ping8);
  EXPECT_EQ(42, f.call(convert(42)).toInt());
  f = AnyFunction::from(&ping16);
  EXPECT_EQ(42, f.call(convert(42)).toInt());
  f = AnyFunction::from(&ping32);
  EXPECT_EQ(42, f.call(convert(42)).toInt());
  f = AnyFunction::from(&ping64);
  EXPECT_EQ(42, f.call(convert(42)).toInt());
  f = AnyFunction::from(&pingFloat);
  EXPECT_EQ(42.42f, f.call(convert(42.42f)).toFloat());
  f = AnyFunction::from(&pingDouble);
  EXPECT_EQ(42.42, f.call(convert(42.42)).toDouble());

  Foo foo;
  f = AnyFunction::from(&Foo::ping8);
  EXPECT_EQ(45, f.call(convert(&foo, 42)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, 42)).toInt());
  f = AnyFunction::from(&Foo::ping8, &foo);
  EXPECT_EQ(45, f.call(convert(42)).toInt());

  f = AnyFunction::from(&Foo::ping16);
  EXPECT_EQ(45, f.call(convert(&foo, 42)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, 42)).toInt());
  f = AnyFunction::from(&Foo::ping16, &foo);
  EXPECT_EQ(45, f.call(convert(42)).toInt());

  f = AnyFunction::from(&Foo::ping32);
  EXPECT_EQ(45, f.call(convert(&foo, 42)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, 42)).toInt());
  f = AnyFunction::from(&Foo::ping32, &foo);
  EXPECT_EQ(45, f.call(convert(42)).toInt());

  f = AnyFunction::from(&Foo::ping64);
  EXPECT_EQ(45, f.call(convert(&foo, 42)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, 42)).toInt());
  f = AnyFunction::from(&Foo::ping64, &foo);
  EXPECT_EQ(45, f.call(convert(42)).toInt());

  f = AnyFunction::from(&Foo::pingFloat);
  EXPECT_EQ(45.42f, f.call(convert(&foo, 42.42f)).toFloat());
  EXPECT_EQ(45.42f, f.call(convert(foo, 42.42f)).toFloat());
  f = AnyFunction::from(&Foo::pingFloat, &foo);
  EXPECT_EQ(45.42f, f.call(convert(42.42f)).toFloat());

  f = AnyFunction::from(&Foo::pingDouble);
  EXPECT_EQ(45.42, f.call(convert(&foo, 42.42)).toDouble());
  EXPECT_EQ(45.42, f.call(convert(foo, 42.42)).toDouble());
  f = AnyFunction::from(&Foo::pingDouble, &foo);
  EXPECT_EQ(45.42, f.call(convert(42.42)).toDouble());

  f = AnyFunction::from(&Parent::pping32);
  EXPECT_EQ(44, f.call(convert(&foo, 42)).toInt());
  EXPECT_EQ(44, f.call(convert(foo, 42)).toInt());
  EXPECT_EQ(44, f.call(convert(static_cast<Parent*>(&foo), 42)).toInt());
  EXPECT_EQ(44, f.call(convert(static_cast<Parent&>(foo), 42)).toInt());
  f = AnyFunction::from(&Parent::pping32, &foo);
  EXPECT_EQ(44, f.call(convert(42)).toInt());
  f = AnyFunction::from(&Parent::pping32, static_cast<Parent*>(&foo));
  EXPECT_EQ(44, f.call(convert(42)).toInt());

  f = AnyFunction::from(&Foo::pingV);
  f.call(convert(&foo, 42));
  EXPECT_EQ(45, foo.r);

  f = AnyFunction::from(&Foo::pingB);
  EXPECT_EQ(true, f.call(convert(&foo, true)).toInt() != 0);
  EXPECT_EQ(true, f.call(convert(&foo, false)).toInt() == 0);

  f = AnyFunction::from(&Foo::vping32);
  EXPECT_EQ(45, f.call(convert(&foo, 42)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, 42)).toInt());
  f = AnyFunction::from(&Foo::vping32, &foo);
  EXPECT_EQ(45, f.call(convert(42)).toInt());
  EXPECT_EQ(45, f.call(convert(42)).toInt());
  f = AnyFunction::from(&Parent::vping32);
  EXPECT_EQ(45, f.call(convert(&foo, 42)).toInt());
  EXPECT_EQ(45, f.call(convert(static_cast<Parent*>(&foo), 42)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, 42)).toInt());

  f = AnyFunction::from(&Foo::getRefF);
  EXPECT_EQ(3, f.call(convert(&foo)).toInt());

  f = AnyFunction::from(&Foo::pingString);
  EXPECT_EQ("foo3", f.call(convert(&foo, "foo")).toString());
  f = AnyFunction::from(&Foo::pingConstString);
  EXPECT_EQ("bar", f.call(convert(&foo, "bar")).toString());
}

TEST(TestObject, Simple) {
  Foo                   foo;
  qi::DynamicObjectBuilder ob;

  ob.advertiseMethod("test", &fun);
  ob.advertiseMethod("testVal", &funVal);
  ob.advertiseMethod("vtest", &vfun);
  ob.advertiseMethod("objtest", &foo, &Foo::fun);
  ob.advertiseMethod("objvtest", &foo, &Foo::vfun);
  ob.advertiseMethod("testBind", (boost::function<int(const int&)>)boost::bind(&fun, 21, _1));
  ob.advertiseMethod("testBind2", (boost::function<int(int)>)boost::bind(&fun, 21, _1));
  ob.advertiseMethod("testBindVal", (boost::function<int(const int&)>)boost::bind(&funVal, 21, _1));
  ob.advertiseMethod("testBind2Val", (boost::function<int(int)>)boost::bind(&funVal, 21, _1));
  ob.advertiseMethod("ptrtest", &ptrfun);
  ob.advertiseMethod("reftest", &reffun);
  ob.advertiseMethod("valuetest", &valuefun);
  qi::AnyObject obj(ob.object());


  EXPECT_EQ(42, obj.call<int>("test", 21, 21));
  EXPECT_EQ(42, obj.call<int>("testVal", 21, 21));
  EXPECT_EQ(42, obj.call<int>("objtest", 21, 21));
  EXPECT_EQ(42, obj.call<int>("testBind", 21));
  EXPECT_EQ(42, obj.call<int>("testBind2", 21));
  EXPECT_EQ(42, obj.call<int>("testBindVal", 21));
  EXPECT_EQ(42, obj.call<int>("testBind2Val", 21));
  EXPECT_EQ(42U,obj.call<unsigned int>("test", 21, 21));
  EXPECT_EQ(42, obj.call<char>("test", 21, 21));
  double d = obj.call<double>("test", 21, 21);
  EXPECT_EQ(42.0, d);
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

//QI_TYPE_SERIALIZABLE(Point)
QI_TYPE_STRUCT(Point, x, y);

struct FPoint { float x; float y;};
QI_TYPE_STRUCT(FPoint, x, y);

struct Test
{
  float x;
};

QI_TYPE_STRUCT_DECLARE(Test);
QI_TYPE_STRUCT_IMPLEMENT(Test, x);



Point swapPoint(const Point& b)
{
  return point(b.y, b.x);
}

TEST(TestObject, SerializeSimple)
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("swapPoint", &swapPoint);
  qi::AnyObject obj(ob.object());
  Point p;
  p.x = 1; p.y = 2;
  Point res = obj.call<Point>("swapPoint", p);
  ASSERT_EQ(2, res.x);
  ASSERT_EQ(1, res.y);
}

TEST(TestObject, ConvertSimple)
{
  Point p; p.x = 1; p.y = 2;
  FPoint p2 = qi::AnyReference::from(p).to<FPoint>();
  ASSERT_EQ(p2.x, p.x);
  ASSERT_EQ(p2.y, p.y);
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

QI_TYPE_STRUCT_DECLARE(Complex)
QI_TYPE_STRUCT_IMPLEMENT(Complex , points, foo, baz, stuff);


struct Complex2
{
  std::list<FPoint> points;
  int foo;
  std::string baz;
  std::vector<std::list<float> > stuff;
};

QI_TYPE_STRUCT(Complex2, points, foo, baz, stuff);

//QI_TYPE_SERIALIZABLE(::Complex)



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

  qi::DynamicObjectBuilder ob;
  unsigned id = ob.advertiseMethod("echo", &echoBack);
  qi::AnyObject obj(ob.object());
  std::cerr << obj.metaObject().methodMap()[id].toString() << std::endl;
  Complex res = obj.call<Complex>("echo", comp);
  ASSERT_EQ(res, comp);
}

TEST(TestObject, convertComplex)
{
  Complex comp;
  comp.foo = 1;
  comp.points.push_back(point(1, 2));
  comp.points.push_back(point(3, 4));
  comp.baz = "testbaz";
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  comp.stuff.push_back(v);
  v.push_back(3);
  comp.stuff.push_back(v);
  Complex2 comp2 = qi::AnyReference::from(comp).to<Complex2>();
  ASSERT_EQ(comp2.foo, comp.foo);
  ASSERT_EQ(comp.points.size(), comp2.points.size());
  ASSERT_EQ(comp.points.front().x, comp2.points.front().x);
  ASSERT_EQ(comp.baz, comp2.baz);
  ASSERT_EQ(comp.stuff.size(), comp2.stuff.size());
  ASSERT_EQ(comp.stuff.front().size(), comp2.stuff.front().size());
  ASSERT_EQ(comp.stuff.front().front(), comp2.stuff.front().front());
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
  qi::AnyObject oa1 = builder.object(&a1, &qi::AnyObject::deleteGenericObjectOnly);
  qi::AnyObject oa2 = builder.object(&a2, &qi::AnyObject::deleteGenericObjectOnly);
  ASSERT_TRUE(!oa1.eventLoop()); // no eventloop override
  ASSERT_EQ(2, oa1.call<int>("add", 1));
  ASSERT_EQ(3, oa2.call<int>("add", 1));
  ASSERT_EQ(5, oa1.call<int>("addTwo", 3, 2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByPtr", &a2));
  //GenericObject is T not T*
  ASSERT_EQ(3, oa1.call<int>("addAdderByPtr", oa2));
  qiLogDebug() << "NEXT";
  ASSERT_EQ(3, oa1.call<int>("addAdderByRef", a2));
  qiLogDebug() << "NEXT";
  ASSERT_EQ(3, oa1.call<int>("addAdderByRef", oa2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByConstPtr", &a2));
  // GenericObject is T not T*
  ASSERT_EQ(3, oa1.call<int>("addAdderByConstPtr", oa2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByConstRef", a2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByConstRef", oa2));

  ASSERT_EQ(4, oa1.call<int>("increment", 3));
  ASSERT_EQ(4, oa1.call<int>("increment2", 3));
}

class MAdder: public Adder, public qi::Manageable
{
public:
  MAdder() {}
  MAdder(int i ) : Adder(i) {}
};

TEST(TestObject, ObjectTypeBuilderAsync)
{
  // We test both async calls, and calling methods with inherited argument type
  qi::ObjectTypeBuilder<MAdder> builder;
  builder.inherits<Adder>();
  builder.advertiseMethod("add", &Adder::add, qi::MetaCallType_Queued);
  builder.advertiseMethod("addTwo", &Adder::addTwo, qi::MetaCallType_Queued);
  builder.advertiseMethod("addAdderByRef", &Adder::addAdderByRef, qi::MetaCallType_Queued);
  builder.advertiseMethod("addAdderByConstRef", &Adder::addAdderByConstRef, qi::MetaCallType_Queued);
  builder.advertiseMethod("addAdderByPtr", &Adder::addAdderByPtr, qi::MetaCallType_Queued);
  builder.advertiseMethod("addAdderByConstPtr", &Adder::addAdderByConstPtr, qi::MetaCallType_Queued);
  builder.advertiseMethod("increment", &Adder::increment, qi::MetaCallType_Queued);
  builder.advertiseMethod("increment2", &Incrementer::increment, qi::MetaCallType_Queued);
  MAdder a1(1);
  MAdder a2(2);
  qi::AnyObject oa1 = builder.object(&a1, &qi::AnyObject::deleteGenericObjectOnly);
  qi::AnyObject oa2 = builder.object(&a2, &qi::AnyObject::deleteGenericObjectOnly);
  ASSERT_EQ(2, oa1.call<int>("add", 1));
  ASSERT_EQ(3, oa2.call<int>("add", 1));
  ASSERT_EQ(5, oa1.call<int>("addTwo", 3, 2));

  // ptr cast not supported, cast ourselve
  ASSERT_EQ(3, oa1.call<int>("addAdderByPtr", (Adder*)&a2));
  // Generic object is T not T*
  // ASSERT_EQ(3, oa1->call<int>("addAdderByPtr", oa2));
  // Copies a2
  ASSERT_EQ(4, oa1.call<int>("addAdderByRef", a2));
  // AnyObject: no copy
  ASSERT_EQ(3, oa1.call<int>("addAdderByRef", oa2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByConstPtr", &a2));
  // GenericObject is T not T*
  //ASSERT_EQ(3, oa1->call<int>("addAdderByConstPtr", oa2));

  ASSERT_EQ(4, oa1.call<int>("addAdderByConstRef", a2));
  ASSERT_EQ(3, oa1.call<int>("addAdderByConstRef", oa2));

  ASSERT_EQ(4, oa1.call<int>("increment", 3));
  ASSERT_EQ(4, oa1.call<int>("increment2", 3));
}


TEST(TestObject, TypeType)
{
  using namespace qi;
  int i = 12;
  std::vector<AnyReference> vals = convert(i);
  AnyReference val = vals[0];
  qiLogDebug() << "type ptr " << val.type()->infoString() << " "
  <<(void*)val.type();
  ASSERT_EQ(TypeKind_Int, val.kind());
  ASSERT_EQ(12, val.toInt());
  float onePointFive = 1.5f;
  vals = convert(onePointFive);
  val = vals[0];
  ASSERT_EQ(TypeKind_Float, val.kind());
  ASSERT_EQ(1.5, val.toDouble());

  /* We go through a named variable.
   * A constant has no reason to have its address survive
   * the convert() function call.
   * (Problem arises under vcxx release).
   * It works above because tested types are using byvalue Type.
   */
  double dv = 1.5;
  vals = convert(dv);
  val = vals[0];
  ASSERT_EQ(TypeKind_Float, val.kind());
  ASSERT_EQ(1.5, val.toDouble());

  vals = convert("foo");
  val = vals[0];
  ASSERT_EQ(TypeKind_String, val.kind());
  ASSERT_EQ(std::string("foo"), val.toString());
}

void ccb() {
};

class CPPCB {
public:
  void cb() {
  }
};

TEST(TestObject, CallBackRegistration)
{
  qi::DynamicObjectBuilder gob;

  gob.advertiseSignal("testcb");
  qi::AnyObject obj = gob.object();
  CPPCB c;
  obj.connect("testcb", boost::bind(&CPPCB::cb, &c));
  obj.connect("testcb", &ccb);
 // obj->connect("testcb", boost::bind<void>(&ccb));
}

void _delaySet(qi::Promise<int> p, unsigned long msDelay, int value)
{
  qiLogVerbose() << "Entering delaySet";
  qi::os::msleep(msDelay);
  if (value == -1)
    p.setError("-1");
  else
    p.setValue(value);
  qiLogVerbose() << "Leaving delaySet";
}

qi::Future<int> delaySet(unsigned long msDelay, int value)
{
  qi::Promise<int> p;
  boost::thread(_delaySet, p, msDelay, value);
  return p.future();
}

qi::FutureSync<int> delaySetSync(unsigned long msDelay, int value)
{
  qi::Promise<int> p;
  boost::thread(_delaySet, p, msDelay, value);
  return p.future();
}

TEST(TestObject, Future)
{
  qi::DynamicObjectBuilder gob;
  gob.advertiseMethod("delaySet", &delaySet);
  qi::AnyObject obj = gob.object();
  qi::Future<int> f = obj.call<int>("delaySet", 500, 41);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_EQ(41, f.value());
  f =  obj.call<int>("delaySet", 500, -1);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_TRUE(f.hasError());
  std::cerr << "ERR " << f.error() << std::endl;
}

void forward(qi::Future<int> f, qi::Promise<void> p) {
  if (f.hasError()) {
    p.setError(f.error());
    return;
  }
  p.setValue(0);
}

qi::Future<void> delaySetV(unsigned long msDelay, int value, qi::Promise<int>prom)
{
  qi::Promise<void> p;
  prom.future().connect(boost::bind<void>(&forward, _1, p));
  boost::thread(_delaySet, prom, msDelay, value);
  return p.future();
}

TEST(TestObject, FutureVoid)
{
  qi::DynamicObjectBuilder gob;
  qi::Promise<int> prom;
  gob.advertiseMethod("delaySet", boost::bind(&delaySetV, _1, _2, prom));
  qi::AnyObject obj = gob.object();
  qi::Future<void> f = obj.call<void>("delaySet", 500, 41);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_EQ(41, prom.future().value());
  prom.reset();
  f =  obj.call<void>("delaySet", 500, -1);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_TRUE(f.hasError());
  std::cerr << "ERR " << f.error() << std::endl;
}

TEST(TestObject, FutureSync)
{
  qi::DynamicObjectBuilder gob;
  gob.advertiseMethod("delaySetSync", &delaySetSync);
  qi::AnyObject obj = gob.object();
  qi::Future<int> f = obj.call<int>("delaySetSync", 500, 41);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_EQ(41, f.value());
  f =  obj.call<int>("delaySetSync", 500, -1);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_TRUE(f.hasError());
  std::cerr << "ERR " << f.error() << std::endl;
}

TEST(TestObject, statisticsGeneric)
{
  qi::DynamicObjectBuilder gob;
  int mid = gob.advertiseMethod("sleep", &qi::os::msleep);
  qi::AnyObject obj = gob.object();
  obj.call<void>("sleep", 10);
  EXPECT_TRUE(obj.stats().empty());
  obj.enableStats(true);
  obj.call<void>("sleep", 10);
  obj.call<void>("sleep", 100);
  qi::ObjectStatistics stats = obj.stats();
  EXPECT_EQ(1u, stats.size());
  qi::MethodStatistics& m = stats[mid];
  EXPECT_EQ(2u, m.count());
  // Don't expect too much sleep precision
  EXPECT_GT(0.01, std::abs(m.wall().minValue() - 0.010));
  EXPECT_GT(0.03, std::abs(m.wall().maxValue() - 0.100));
  EXPECT_GT(0.01, m.system().maxValue());
  EXPECT_GT(0.01, m.user().maxValue());
  obj.clearStats();
  obj.call<void>("sleep", 0);
  stats = obj.stats();
  m = stats[mid];
  EXPECT_EQ(1u, m.count());
  obj.clearStats();
  obj.enableStats(false);
  obj.call<void>("sleep", 0);
  EXPECT_TRUE(obj.stats().empty());

  obj.clearStats();
  obj.enableStats(true);
  obj.call<void>("sleep", 0);
  stats = obj.call<qi::ObjectStatistics>("stats");
  m = stats[mid];
  EXPECT_EQ(1u, m.count());
}

TEST(TestObject, statisticsType)
{
  qi::ObjectTypeBuilder<Adder> builder;
  int mid = builder.advertiseMethod("add", &Adder::add);
  Adder a1(1);
  qi::AnyObject oa1 = builder.object(&a1, &qi::AnyObject::deleteGenericObjectOnly);

  EXPECT_EQ(3, oa1.call<int>("add", 2));

  qi::ObjectStatistics stats;

  oa1.enableStats(true);
  oa1.call<int>("add", 2);
  stats = oa1.stats();
  EXPECT_EQ(1u, stats[mid].count());

  oa1.call<int>("add", 2);
  stats = oa1.call<qi::ObjectStatistics>("stats");
  EXPECT_EQ(2u, stats[mid].count());
}

void pushTrace(std::vector<qi::EventTrace>& target,
  const qi::EventTrace& trace)
{
  target.push_back(trace);
}

int throw_exception(const std::string& content)
{
  throw std::runtime_error(content);
}

TEST(TestObject, traceGeneric)
{
  qi::DynamicObjectBuilder gob;
  int mid = gob.advertiseMethod("sleep", &qi::os::msleep);
  int mid2 = gob.advertiseMethod("boom", &throw_exception);
  qi::AnyObject obj = gob.object();
  std::vector<qi::EventTrace> traces;
  qi::SignalLink id = obj.connect("traceObject",
    (boost::function<void(qi::EventTrace)>)
    boost::bind(&pushTrace, boost::ref(traces), _1));
  obj.call<void>("sleep", 100);
  for (unsigned i=0; i<20 && traces.size()<2; ++i) qi::os::msleep(50);
  qi::os::msleep(50);
  ASSERT_EQ(2u, traces.size());
  EXPECT_EQ(qi::EventTrace::Event_Call, traces[0].kind());
  EXPECT_EQ(qi::EventTrace::Event_Result, traces[1].kind());
  EXPECT_EQ(mid, traces[0].slotId());
  EXPECT_EQ(traces[0].id(), traces[1].id());
  qi::int64_t delta =
    traces[1].timestamp().tv_sec*1000
    + traces[1].timestamp().tv_usec/1000
    - traces[0].timestamp().tv_sec*1000
    - traces[0].timestamp().tv_usec/1000;
  EXPECT_LT(std::abs(delta - 100LL), 20LL); // be leniant
  ASSERT_TRUE(obj.call<bool>("isTraceEnabled").value());
  qi::os::msleep(50);
  traces.clear();
  obj.call<void>("boom", "o<").wait();
  for (unsigned i=0; i<20 && traces.size()<2; ++i) qi::os::msleep(50);
  qi::os::msleep(50);
  ASSERT_EQ(2u, traces.size());
  EXPECT_EQ(qi::EventTrace::Event_Call, traces[0].kind());
  EXPECT_EQ(qi::EventTrace::Event_Error, traces[1].kind());
  EXPECT_EQ(mid2, traces[0].slotId());
  EXPECT_EQ(traces[0].id(), traces[1].id());
  obj.disconnect(id);
  qi::os::msleep(50);
  ASSERT_TRUE(!obj.call<bool>("isTraceEnabled").value());
}

TEST(TestObject, traceType)
{
  qi::ObjectTypeBuilder<Adder> builder;
  int mid = builder.advertiseMethod("add", &Adder::add);
  Adder a1(1);
  qi::AnyObject oa1 = builder.object(&a1, &qi::AnyObject::deleteGenericObjectOnly);

  EXPECT_EQ(3, oa1.call<int>("add", 2));

  std::vector<qi::EventTrace> traces;
  qi::SignalLink id = oa1.connect("traceObject",
    (boost::function<void(qi::EventTrace)>)
    boost::bind(&pushTrace, boost::ref(traces), _1));

  EXPECT_EQ(3, oa1.call<int>("add", 2));
  for (unsigned i=0; i<20 && traces.size()<2; ++i) qi::os::msleep(50);
  qi::os::msleep(50);
  ASSERT_EQ(2u, traces.size());
  EXPECT_EQ(qi::EventTrace::Event_Call, traces[0].kind());
  EXPECT_EQ(qi::EventTrace::Event_Result, traces[1].kind());
  EXPECT_EQ(mid, traces[0].slotId());
  ASSERT_TRUE(oa1.call<bool>("isTraceEnabled").value());
  oa1.disconnect(id);
  qi::os::msleep(50);
  ASSERT_TRUE(!oa1.call<bool>("isTraceEnabled").value());
}

static void bim(int i, qi::Promise<void> p, const std::string &name) {
  qiLogInfo() << "Bim le callback:" << name << " ,i:" << i;
  if (i == 42)
    p.setValue(0);
  else
    p.setError(0);
}

//test local and remote callback.
//trigger local or remote each time
TEST(TestObject, AdvertiseRealSignal)
{
  qi::Promise<void> plocal;
  qi::Promise<void> premote;

  EXPECT_ANY_THROW(plocal.future().hasValue(0));
  EXPECT_ANY_THROW(premote.future().hasValue(0));

  qi::Signal<int> sig;
  sig.connect(boost::bind<void>(&bim, _1, plocal, "local"));
  qi::DynamicObjectBuilder gob;
  unsigned int id = gob.advertiseSignal("sig", &sig);
  ASSERT_LT(0, id);
  qi::AnyObject obj = gob.object();
  obj.connect("sig", boost::bind<void>(&bim, _1, premote, "remote"));

  //test remote trigger
  qiLogInfo() << "remote trigger.";
  obj.post("sig", 42);
  plocal.future().wait(1000);
  premote.future().wait(1000);
  ASSERT_TRUE(plocal.future().hasValue(0));
  ASSERT_TRUE(premote.future().hasValue(0));

  plocal.reset();
  premote.reset();

  //test local trigger
  qiLogInfo() << "local trigger.";
  sig(42);
  plocal.future().wait(1000);
  premote.future().wait(1000);
  ASSERT_TRUE(plocal.future().hasValue(0));
  ASSERT_TRUE(premote.future().hasValue(0));
}

std::string add(const std::string &str) {
  return str + "ahah";
}

TEST(TestObject, AdvertiseBadType)
{
  qi::DynamicObjectBuilder gob;

  //missing ::
  EXPECT_THROW(gob.xAdvertiseMethod("s", "addbadsignature", "s)", qi::AnyFunction::from(&add)), std::runtime_error);
  //missing param sig
  EXPECT_THROW(gob.xAdvertiseMethod("s", "addbadsignature", "", qi::AnyFunction::from(&add)), std::runtime_error);
  //missing ()
  EXPECT_THROW(gob.xAdvertiseMethod("s", "addbadsignature", "::", qi::AnyFunction::from(&add)), std::runtime_error);
  //missing ()
  EXPECT_THROW(gob.xAdvertiseMethod("s", "addbadsignature", "ss", qi::AnyFunction::from(&add)), std::runtime_error);
  //G do not exists
  EXPECT_THROW(gob.xAdvertiseMethod("s", "addbadsignature", "(G)", qi::AnyFunction::from(&add)), std::runtime_error);

  //bad return type
  EXPECT_THROW(gob.xAdvertiseMethod("TOOBADDDDD", "addbadsignature", "(s)", qi::AnyFunction::from(&add)), std::runtime_error);
  //two return type
  EXPECT_THROW(gob.xAdvertiseMethod("si", "addbadsignature", "(s)", qi::AnyFunction::from(&add)), std::runtime_error);

  EXPECT_THROW(gob.xAdvertiseSignal("ploufffffffPlifffff", ""), std::runtime_error);

  EXPECT_THROW(gob.xAdvertiseSignal("", "si"), std::runtime_error);
  EXPECT_THROW(gob.xAdvertiseProperty("plouf", "ss"), std::runtime_error);
  EXPECT_THROW(gob.xAdvertiseProperty("plouf", "("), std::runtime_error);
  //signature mismatch
  //TODO: EXPECT_EQ(-1, gob.xAdvertiseMethod("i", "add", "(i)", qi::AnyFunction::from(&add)));
}


class ArgPack
{
public:
  qi::Property<qi::AnyValue> onCall;
  void callMe(const qi::AnyArguments& pack)
  {
    onCall.set(qi::AnyValue::from(pack.args()));
  }
};

QI_REGISTER_OBJECT(ArgPack, onCall, callMe);

TEST(TestObject, AnyArguments)
{
  boost::shared_ptr<ArgPack> ap(new ArgPack);
  qi::AnyObject o = qi::AnyValue::from(ap).to<qi::AnyObject>();
  qi::details::printMetaObject(std::cerr, o.metaObject());
  o.call<void>("callMe", 1, 2, 3);
  qi::AnyValue args = o.property<qi::AnyValue>("onCall");
  std::vector<int> expect = boost::assign::list_of(1)(2)(3);
  EXPECT_EQ(expect, args.to<std::vector<int> >());
}

class Sleeper
{
public:
  int msleep(int duration)
  {
    qi::os::msleep(duration);
    return duration;
  }
  Sleeper()
  {
    qiLogDebug() << this <<" create";
  }
  ~Sleeper()
  {
    qiLogDebug() << this <<" destroy";
    ++dtorCount;
  }
  static qi::Atomic<int> dtorCount;
};

qi::Atomic<int> Sleeper::dtorCount;

QI_REGISTER_OBJECT(Sleeper, msleep);
QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR(Sleeper);

TEST(TestObject, async)
{
  {
    Sleeper rf;
    qi::Future<int> f = qi::async<int>(&rf, "msleep", 100);
    EXPECT_EQ(qi::FutureState_Running, f.wait(0));
    EXPECT_EQ(qi::FutureState_FinishedWithValue, f.wait());
    EXPECT_EQ(100, f.value());

    boost::shared_ptr<Sleeper> rfptr(new Sleeper);
    f = qi::async<int>(rfptr, "msleep", 100);
    EXPECT_EQ(qi::FutureState_Running, f.wait(0));
    EXPECT_EQ(qi::FutureState_FinishedWithValue, f.wait());
    EXPECT_EQ(100, f.value());

    qiLogDebug() << "converting to AnyObject";
    qi::AnyObject o = qi::AnyReference::from(rfptr).toObject();
    qiLogDebug() << "convesion done";
    f = qi::async<int>(o, "msleep", 100);
    EXPECT_EQ(qi::FutureState_Running, f.wait(0));
    EXPECT_EQ(qi::FutureState_FinishedWithValue, f.wait());
    EXPECT_EQ(100, f.value());
  }

  for (unsigned i=0; i<50 && *Sleeper::dtorCount <2; ++i)
    qi::os::msleep(10);
  EXPECT_EQ(2, *Sleeper::dtorCount);

  // Factory leaks, so can't test no-leak of async...
  qiLogDebug() << "Factory";
  qi::AnyObject o = qi::createObject("Sleeper");
  qi::Future<int> f = qi::async<int>(o, "msleep", 100);
  EXPECT_EQ(qi::FutureState_Running, f.wait(0));
  EXPECT_EQ(qi::FutureState_FinishedWithValue, f.wait());
  EXPECT_EQ(100, f.value());
}

qi::GenericFunctionParameters args(
  qi::AutoAnyReference p1=qi::AutoAnyReference(),
  qi::AutoAnyReference p2=qi::AutoAnyReference(),
  qi::AutoAnyReference p3=qi::AutoAnyReference())
{
  qi::GenericFunctionParameters res;
  if (p1.type()) res.push_back(p1); else return res;
  if (p2.type()) res.push_back(p2); else return res;
  if (p3.type()) res.push_back(p3); else return res;
  return res;
}

TEST(TestMetaObject, findMethod)
{
  qi::MetaObjectBuilder b;
  unsigned int f   = b.addMethod("i", "f", "(i)");
  unsigned int g1  = b.addMethod("i", "g", "(i)");
  unsigned int g2  = b.addMethod("i", "g", "(ii)");
  unsigned int h1i = b.addMethod("i", "h", "(i)");
  unsigned int h1s = b.addMethod("i", "h", "(s)");
  unsigned int h2  = b.addMethod("i", "h", "(ii)");

  qi::MetaObject mo = b.metaObject();
  bool canCache;
  int mid;
  mid = mo.findMethod("f", args(1), &canCache);
  EXPECT_EQ(mid, f); EXPECT_TRUE(canCache);
  mid = mo.findMethod("g", args(1), &canCache);
  EXPECT_EQ(mid, g1); EXPECT_TRUE(canCache);
  mid = mo.findMethod("g", args(1, 1), &canCache);
  EXPECT_EQ(mid, g2); EXPECT_TRUE(canCache);
  // no garantee is made on result of findmethod(g, "foo"), so not tested
  mid = mo.findMethod("h", args(1), &canCache);
  EXPECT_EQ(mid, h1i); EXPECT_FALSE(canCache);
  mid = mo.findMethod("h", args("foo"), &canCache);
  EXPECT_EQ(mid, h1s); EXPECT_FALSE(canCache);
  mid = mo.findMethod("h", args(1, 1), &canCache);
  EXPECT_EQ(mid, h2); EXPECT_TRUE(canCache);

  mid = mo.findMethod("h::(i)", args(1), &canCache);
  EXPECT_EQ(mid, h1i); EXPECT_TRUE(canCache);

  // check null canCache
  mo.findMethod("h::(i)", args(1), 0);
  mid = mo.findMethod("h", args("foo"), 0);
  EXPECT_TRUE(true);
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
