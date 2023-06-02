/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <map>
#include <unordered_map>
#include <thread>
#include <chrono>

#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include <boost/assign/list_of.hpp>

#include "test_object.hpp"

#include <qi/log.hpp>
#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/anymodule.hpp>
#include <random>
#include <boost/container/flat_map.hpp>
#include <boost/container/stable_vector.hpp>

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
  Foo()
    : f(3)
    , r(0)
  {}
  Foo(const Foo& b)
    : f(b.f+1)
    , r(0)
    , s(b.s)
  {}
  virtual ~Foo() {}
  void operator = (const Foo& b)
  {
    f = b.f + 1;
    r = b.r;
    s = b.s;
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
QI_TYPE_CONCRETE(C);
C* ptrfun(C* ptr) { return ptr;}
C& reffun(const C& ref) { return const_cast<C&>(ref);}
C valuefun(C val) { return val;}

template <typename... Args>
std::vector<qi::AnyReference> convert(Args&... args) // we explicitly take lvalue-ref
{
  qi::AnyReference r[]{ qi::AnyReference::from(args)... };
  return { std::begin(r), std::remove_if(std::begin(r), std::end(r),
                                         [](qi::AnyReference ref) { return !ref.rawValue(); }) };
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
  int addAdderByRef(Adder& b) { return v + b.v; }
  int addAdderByConstRef(const Adder& b) { return v + b.v; }
  int addAdderByPtr(Adder* b) { return b ? v + b->v : -1;}
  int addAdderByConstPtr(const Adder* b) { return b ? v + b->v : -1; }
  int v;
};
QI_TYPE_CONCRETE(Adder);
template<typename T> bool checkValue(qi::AnyReference v, const T& val)
{
  T actual = v.as<T>();
  bool ok = actual == val;
  if (!ok)
    qiLogError() << "expected: " << val <<"  actual: " << actual;
  return ok;
}

TEST(TestFunction, Typing)
{
  qiLogDebug() << "vfun";
  qi::AnyFunction fv = qi::AnyFunction::from(&vfun);
  qiLogDebug() << "fun";
  qi::AnyFunction fv2 = qi::AnyFunction::from(&fun);
  qiLogDebug() << "Foo::fun";
  qi::AnyFunction mv = qi::AnyFunction::from(&Foo::fun);
  const auto arg1 = 1; const auto arg2 = 2;
  std::vector<qi::AnyReference> args1 = convert(arg1, arg2);
  qi::AnyReference res = fv2.call(args1);
  ASSERT_TRUE(checkValue(res, 3));

  qi::AnyFunction adderAdd = qi::AnyFunction::from(&Adder::add);
  Adder add1(1);
  const auto arg = 41;
  std::vector<qi::AnyReference> argsAdd = convert(arg);
  res = adderAdd.call(qi::AnyReference::from(add1), argsAdd);
  ASSERT_TRUE(checkValue(res, 42));
}

TEST(TestFunction, BindType)
{
  int fun(char, float, std::string);
  using Bind = decltype(boost::bind(fun, boost::placeholders::_3, 3.14, boost::placeholders::_1));
  using BindFnType = typename qi::detail::bind::boost_bind_function_type<Bind>::type;
  static_assert(std::is_same_v<BindFnType, int(std::string, char)>);
  SUCCEED();
}

TEST(TestFunction, ABI)
{
  using namespace qi;
  const int intArg = 42;
  const float floatArg = 42.42f;
  const double doubleArg = 42.42;

  // We must declare inheritance between Foo and Parent
  ObjectTypeBuilder<Foo> b;
  b.inherits<Parent>();
  b.registerType();
  AnyFunction f;
  f = AnyFunction::from(&ping8);
  EXPECT_EQ(42, f.call(convert(intArg)).toInt());
  f = AnyFunction::from(&ping16);
  EXPECT_EQ(42, f.call(convert(intArg)).toInt());
  f = AnyFunction::from(&ping32);
  EXPECT_EQ(42, f.call(convert(intArg)).toInt());
  f = AnyFunction::from(&ping64);
  EXPECT_EQ(42, f.call(convert(intArg)).toInt());
  f = AnyFunction::from(&pingFloat);
  EXPECT_EQ(42.42f, f.call(convert(floatArg)).toFloat());
  f = AnyFunction::from(&pingDouble);
  EXPECT_EQ(42.42, f.call(convert(doubleArg)).toDouble());

  Foo foo;
  auto* const fooPtr = &foo;
  f = AnyFunction::from(&Foo::ping8);
  EXPECT_EQ(45, f.call(convert(fooPtr, intArg)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, intArg)).toInt());
  f = AnyFunction::from(&Foo::ping8, fooPtr);
  EXPECT_EQ(45, f.call(convert(intArg)).toInt());

  f = AnyFunction::from(&Foo::ping16);
  EXPECT_EQ(45, f.call(convert(fooPtr, intArg)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, intArg)).toInt());
  f = AnyFunction::from(&Foo::ping16, fooPtr);
  EXPECT_EQ(45, f.call(convert(intArg)).toInt());

  f = AnyFunction::from(&Foo::ping32);
  EXPECT_EQ(45, f.call(convert(fooPtr, intArg)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, intArg)).toInt());
  f = AnyFunction::from(&Foo::ping32, fooPtr);
  EXPECT_EQ(45, f.call(convert(intArg)).toInt());

  f = AnyFunction::from(&Foo::ping64);
  EXPECT_EQ(45, f.call(convert(fooPtr, intArg)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, intArg)).toInt());
  f = AnyFunction::from(&Foo::ping64, fooPtr);
  EXPECT_EQ(45, f.call(convert(intArg)).toInt());

  f = AnyFunction::from(&Foo::pingFloat);
  EXPECT_EQ(45.42f, f.call(convert(fooPtr, floatArg)).toFloat());
  EXPECT_EQ(45.42f, f.call(convert(foo, floatArg)).toFloat());
  f = AnyFunction::from(&Foo::pingFloat, fooPtr);
  EXPECT_EQ(45.42f, f.call(convert(floatArg)).toFloat());

  f = AnyFunction::from(&Foo::pingDouble);
  EXPECT_EQ(45.42, f.call(convert(fooPtr, doubleArg)).toDouble());
  EXPECT_EQ(45.42, f.call(convert(foo, doubleArg)).toDouble());
  f = AnyFunction::from(&Foo::pingDouble, fooPtr);
  EXPECT_EQ(45.42, f.call(convert(doubleArg)).toDouble());

  f = AnyFunction::from(&Parent::pping32);
  EXPECT_EQ(44, f.call(convert(fooPtr, intArg)).toInt());
  EXPECT_EQ(44, f.call(convert(foo, intArg)).toInt());
  auto* const parentPtr = static_cast<Parent*>(fooPtr);
  EXPECT_EQ(44, f.call(convert(parentPtr, intArg)).toInt());
  EXPECT_EQ(44, f.call(convert(static_cast<Parent&>(foo), intArg)).toInt());
  f = AnyFunction::from(&Parent::pping32, fooPtr);
  EXPECT_EQ(44, f.call(convert(intArg)).toInt());
  f = AnyFunction::from(&Parent::pping32, static_cast<Parent*>(fooPtr));
  EXPECT_EQ(44, f.call(convert(intArg)).toInt());

  f = AnyFunction::from(&Foo::pingV);
  f.call(convert(fooPtr, intArg));
  EXPECT_EQ(45, foo.r);

  f = AnyFunction::from(&Foo::pingB);
  const bool trueArg = true;
  const bool falseArg = false;
  EXPECT_EQ(true, f.call(convert(fooPtr, trueArg)).toInt() != 0);
  EXPECT_EQ(true, f.call(convert(fooPtr, falseArg)).toInt() == 0);

  f = AnyFunction::from(&Foo::vping32);
  EXPECT_EQ(45, f.call(convert(fooPtr, intArg)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, intArg)).toInt());
  f = AnyFunction::from(&Foo::vping32, fooPtr);
  EXPECT_EQ(45, f.call(convert(intArg)).toInt());
  EXPECT_EQ(45, f.call(convert(intArg)).toInt());
  f = AnyFunction::from(&Parent::vping32);
  EXPECT_EQ(45, f.call(convert(fooPtr, intArg)).toInt());
  EXPECT_EQ(45, f.call(convert(parentPtr, intArg)).toInt());
  EXPECT_EQ(45, f.call(convert(foo, intArg)).toInt());

  f = AnyFunction::from(&Foo::getRefF);
  EXPECT_EQ(3, f.call(convert(fooPtr)).toInt());

  f = AnyFunction::from(&Foo::pingString);
  EXPECT_EQ("foo3", f.call(convert(fooPtr, "foo")).toString());
  f = AnyFunction::from(&Foo::pingConstString);
  EXPECT_EQ("bar", f.call(convert(fooPtr, "bar")).toString());
}

int summ(int p1, short p2, char p3, const char* s)
{
  return p1 + p2 + p3 + strlen(s);
}

qi::AnyObject getobj()
{
  qi::DynamicObjectBuilder b;
  b.advertiseMethod("summ", &summ);
  return b.object();
}

TEST(TestFunction, Call)
{
  qi::AnyFunction f = qi::AnyFunction::from(&summ);
  ASSERT_EQ(10, f.call<int>(1, 2, 3, "lola"));
  f = qi::AnyFunction::from(&getobj);
  qi::AnyWeakObject obj = f.call<qi::AnyObject>();
  ASSERT_FALSE(obj.lock());
}

TEST(TestObject, Accessors)
{
  struct Object
  {
    int i;
    int& getI();
    const int& getConstI() const;
  };
  using AccessorFreeFn = int& (&) (Object*);
  using MissingInstance = int& (&) ();
  using InstanceIsNotAClass = int& (&) (int*);
  using TooManyArgs = int& (&) (Object*, int);
  using ReturnTypeNotRValueRef = int (&) (Object*);

  using namespace qi::detail::accessor;
  static_assert( IsAccessor<decltype(&Object::i)>());
  static_assert( IsAccessor<decltype(&Object::getI)>());
  static_assert( IsAccessor<decltype(&Object::getConstI)>());
  static_assert( IsAccessor<AccessorFreeFn>());
  static_assert(!IsAccessor<MissingInstance>());
  static_assert(!IsAccessor<InstanceIsNotAClass>());
  static_assert(!IsAccessor<TooManyArgs>());
  static_assert(!IsAccessor<ReturnTypeNotRValueRef>());
  SUCCEED();
}

TEST(TestObject, Methods) {
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
  obj.async<void>("vtest", 21, 21).wait();
  EXPECT_EQ(42, gGlobalResult);
  gGlobalResult = 0;
  obj.async<void>("objvtest", 21, 21).wait();
  EXPECT_EQ(42, gGlobalResult);

  C f(42);
  EXPECT_EQ(&f, obj.call<C*>("ptrtest", &f));
  EXPECT_EQ(f, obj.call<C>("valuetest", f));
}

struct YetAnotherPoint
{
  bool operator == (const YetAnotherPoint& b) const { return x==b.x && y==b.y;}
  int x, y;
};

YetAnotherPoint point(int x, int y)
{
  YetAnotherPoint p; p.x = x; p.y = y; return p;
}

//QI_TYPE_SERIALIZABLE(YetAnotherPoint)
QI_TYPE_STRUCT(YetAnotherPoint, x, y);

struct FPoint { float x; float y;};
QI_TYPE_STRUCT(FPoint, x, y);

struct Test
{
  float x;
};

QI_TYPE_STRUCT_DECLARE(Test);
QI_TYPE_STRUCT_IMPLEMENT(Test, x);



YetAnotherPoint swapPoint(const YetAnotherPoint& b)
{
  return point(b.y, b.x);
}

TEST(TestObject, SerializationOfCalls)
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("swapPoint", &swapPoint);
  qi::AnyObject obj(ob.object());
  YetAnotherPoint p;
  p.x = 1; p.y = 2;
  YetAnotherPoint res = obj.call<YetAnotherPoint>("swapPoint", p);
  ASSERT_EQ(2, res.x);
  ASSERT_EQ(1, res.y);
}

TEST(TestObject, ConvertSimple)
{
  YetAnotherPoint p; p.x = 1; p.y = 2;
  FPoint p2 = qi::AnyReference::from(p).to<FPoint>();
  ASSERT_EQ(p2.x, p.x);
  ASSERT_EQ(p2.y, p.y);
}

TEST(TestObject, ConvertMapStruct)
{
  using MyMap = std::map<std::string, int>;
  MyMap m;

  m["x"] = 41;
  m["y"] = 42;

  qi::AnyReference r = qi::AnyReference::from(m);

  YetAnotherPoint p = r.to<YetAnotherPoint>();
  qiLogInfo() << "converted map to point";

  ASSERT_EQ(41, p.x);
  ASSERT_EQ(42, p.y);

//  qi::AnyReference r2 = qi::AnyReference::from(p);
//  MyMap m2;

//  m2 = r2.to<MyMap>();

//  ASSERT_EQ(41, m2["x"]);
//  ASSERT_EQ(42, m2["y"]);
}

TEST(TestObject, ConvertGenericMapStruct)
{
  using MyMap = std::map<qi::AnyValue, qi::AnyValue>;
  MyMap m;

  m[qi::AnyValue::from("x")] = qi::AnyValue::from(41);
  m[qi::AnyValue::from("y")] = qi::AnyValue::from(42);

  qi::AnyReference r = qi::AnyReference::from(m);

  YetAnotherPoint p = r.to<YetAnotherPoint>();

  ASSERT_EQ(41, p.x);
  ASSERT_EQ(42, p.y);

//  qi::AnyReference r2 = qi::AnyReference::from(p);
//  MyMap m2;

//  m2 = r2.to<MyMap>();

//  ASSERT_EQ(41, m2[qi::AnyValue::from("x")]);
//  ASSERT_EQ(42, m2[qi::AnyValue::from("y")]);
}


struct Complex
{
  bool operator == (const Complex& b) const {
    return points == b.points
    && foo == b.foo
    && baz == b.baz
    && stuff == b.stuff;
  }
  std::vector<YetAnotherPoint> points;
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
  auto id = ob.advertiseMethod("echo", &echoBack);
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
  // otherwise some arguments are copied
  builder.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  builder.advertiseMethod("add", &Adder::add);
  builder.advertiseMethod("addTwo", boost::function<int(Adder*, int, int)>(boost::bind(&Adder::addTwo, _2, _3)));
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
  ASSERT_TRUE(!oa1.executionContext()); // no eventloop override
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

class Dummy{};

TEST(TestObject, ObjectTypeBuilderTypeDescription)
{
  qi::ObjectTypeBuilder<Dummy> builder;

  std::string objectTypeDescription = "I am a dummy type";
  builder.setDescription(objectTypeDescription);

  Dummy dummy;
  auto object = builder.object(&dummy, &qi::AnyObject::deleteGenericObjectOnly);
  auto metaObject = object.metaObject();

  ASSERT_EQ(objectTypeDescription, metaObject.description());
}

class MAdder: public Adder, public qi::Manageable
{
public:
  MAdder() {}
  MAdder(int i ) : Adder(i) {}
};
QI_TYPE_CONCRETE(MAdder);

TEST(TestObject, ObjectTypeBuilderAsync)
{
  // We test both async calls, and calling methods with inherited argument type
  qi::ObjectTypeBuilder<MAdder> builder;
  builder.inherits<Adder>();
  builder.advertiseMethod("add", &Adder::add, qi::MetaCallType_Queued);
  builder.advertiseMethod("addTwo", boost::function<int(Adder*, int, int)>(boost::bind(&Adder::addTwo, _2, _3)));
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
  AnyReferenceVector vals = convert(i);
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

void _delaySet(qi::Promise<int> p, qi::MilliSeconds delay, int value)
{
  qiLogVerbose() << "Entering delaySet";
  boost::this_thread::sleep_for(delay);
  if (value == -1)
    p.setError("-1");
  else
    p.setValue(value);
  qiLogVerbose() << "Leaving delaySet";
}

qi::Future<int> delaySet(qi::MilliSeconds delay, int value)
{
  qi::Promise<int> p;
  boost::thread(_delaySet, p, delay, value);
  return p.future();
}

qi::FutureSync<int> delaySetSync(qi::MilliSeconds delay, int value)
{
  qi::Promise<int> p;
  boost::thread(_delaySet, p, delay, value);
  return p.future();
}

TEST(TestObject, Future)
{
  qi::DynamicObjectBuilder gob;
  gob.advertiseMethod("delaySet", &delaySet);
  qi::AnyObject obj = gob.object();
  qi::Future<int> f = obj.async<int>("delaySet", qi::MilliSeconds{ 500 }, 41);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_EQ(41, f.value());
  f = obj.async<int>("delaySet", qi::MilliSeconds{ 500 }, -1);
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

qi::Future<void> delaySetV(qi::MilliSeconds delay, int value, qi::Promise<int>& prom)
{
  qi::Promise<void> p;
  prom.future().connect(boost::bind<void>(&forward, _1, p));
  boost::thread(_delaySet, prom, delay, value);
  return p.future();
}

TEST(TestObject, FutureVoid)
{
  qi::DynamicObjectBuilder gob;
  qi::Promise<int> prom;
  gob.advertiseMethod("delaySet", boost::function<qi::Future<void>(qi::MilliSeconds, int)>(
                                      boost::bind(&delaySetV, _1, _2, boost::ref(prom))));
  qi::AnyObject obj = gob.object();
  qi::Future<void> f = obj.async<void>("delaySet", qi::MilliSeconds{ 500 }, 41);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_EQ(41, prom.future().value());
  prom = qi::Promise<int>();
  f = obj.async<void>("delaySet", qi::MilliSeconds{ 500 }, -1);
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
  qi::Future<int> f = obj.async<int>("delaySetSync", qi::MilliSeconds{ 500 }, 41);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_EQ(41, f.value());
  f = obj.async<int>("delaySetSync", qi::MilliSeconds{ 500 }, -1);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_TRUE(f.hasError());
  std::cerr << "ERR " << f.error() << std::endl;
}

// TODO: fix races in ObjectStatistics to reenable this test
TEST(TestObject, DISABLED_statisticsGeneric)
{
  qi::DynamicObjectBuilder gob;
  const auto mid = gob.advertiseMethod("sleep", [](qi::MilliSeconds dura) {
    boost::this_thread::sleep_for(dura);
  });
  qi::AnyObject obj = gob.object();
  obj.call<void>("sleep", qi::MilliSeconds{ 10 });
  EXPECT_TRUE(obj.stats().empty());
  obj.enableStats(true);
  obj.call<void>("sleep", qi::MilliSeconds{ 10 });
  obj.call<void>("sleep", qi::MilliSeconds{ 100 });
  qi::ObjectStatistics stats = obj.stats();
  EXPECT_EQ(1u, stats.size());
  qi::MethodStatistics m = stats[mid];
  EXPECT_EQ(2u, m.count());
  // Don't expect too much sleep precision
  EXPECT_GT(0.01, std::abs(m.wall().minValue() - 0.010));
  EXPECT_GT(0.03, std::abs(m.wall().maxValue() - 0.100));
  EXPECT_GT(0.01, m.system().maxValue());
  EXPECT_GT(0.01, m.user().maxValue());
  obj.clearStats();
  obj.call<void>("sleep", qi::MilliSeconds{ 0 });
  stats = obj.stats();
  m = stats[mid];
  EXPECT_EQ(1u, m.count());
  obj.clearStats();
  obj.enableStats(false);
  obj.call<void>("sleep", qi::MilliSeconds{ 0 });
  EXPECT_TRUE(obj.stats().empty());

  obj.clearStats();
  obj.enableStats(true);
  obj.call<void>("sleep", qi::MilliSeconds{ 0 });
  stats = obj.call<qi::ObjectStatistics>("stats");
  m = stats[mid];
  EXPECT_EQ(1u, m.count());
}

// TODO: fix races in ObjectStatistics to reenable this test
TEST(TestObject, DISABLED_statisticsType)
{
  qi::ObjectTypeBuilder<Adder> builder;
  const auto mid = builder.advertiseMethod("add", &Adder::add);
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
    boost::mutex& mutex,
    const qi::EventTrace& trace)
{
  boost::mutex::scoped_lock l(mutex);
  target.push_back(trace);
}

int throw_exception(const std::string& content)
{
  throw std::runtime_error(content);
}

static bool comparator(qi::EventTrace e1, qi::EventTrace e2)
{
  return static_cast<int>(e1.kind()) < static_cast<int>(e2.kind());
}

TEST(TestObject, traceGeneric)
{
  qi::DynamicObjectBuilder gob;
  const auto mid = gob.advertiseMethod("sleep", [](qi::MilliSeconds dura) {
    boost::this_thread::sleep_for(dura);
  });
  const auto mid2 = gob.advertiseMethod("boom", &throw_exception);
  qi::AnyObject obj = gob.object();
  boost::mutex mutex;
  std::vector<qi::EventTrace> traces;
  qi::SignalLink id = obj.connect("traceObject",
    (boost::function<void(qi::EventTrace)>)
    boost::bind(&pushTrace, boost::ref(traces), boost::ref(mutex), _1)).value();
  ASSERT_TRUE(qi::isValidSignalLink(id));
  obj.call<void>("sleep", qi::MilliSeconds{ 100 });
  for (unsigned i=0; i<20; ++i) {
    {
      boost::mutex::scoped_lock l(mutex);
      if (traces.size() >= 2)
        break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
  }
  std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
  ASSERT_EQ(2u, traces.size());
  std::sort(traces.begin(), traces.end(), comparator); // events may not be in order
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
  ASSERT_TRUE(obj.call<bool>("isTraceEnabled"));
  std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
  traces.clear();
  obj.async<void>("boom", "o<").wait();
  for (unsigned i=0; i<20; ++i) {
    {
      boost::mutex::scoped_lock l(mutex);
      if (traces.size() >= 2)
        break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
  }
  std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
  ASSERT_EQ(2u, traces.size());
  std::sort(traces.begin(), traces.end(), comparator); // events may not be in order
  EXPECT_EQ(qi::EventTrace::Event_Call, traces[0].kind());
  EXPECT_EQ(qi::EventTrace::Event_Error, traces[1].kind());
  EXPECT_EQ(mid2, traces[0].slotId());
  EXPECT_EQ(traces[0].id(), traces[1].id());
  obj.disconnect(id);
  std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
  ASSERT_TRUE(!obj.call<bool>("isTraceEnabled"));
}

TEST(TestObject, traceType)
{
  qi::ObjectTypeBuilder<Adder> builder;
  const auto mid = builder.advertiseMethod("add", &Adder::add);
  Adder a1(1);
  qi::AnyObject oa1 = builder.object(&a1, &qi::AnyObject::deleteGenericObjectOnly);

  EXPECT_EQ(3, oa1.call<int>("add", 2));

  boost::mutex mutex;
  std::vector<qi::EventTrace> traces;
  qi::SignalLink id = oa1.connect("traceObject",
    (boost::function<void(qi::EventTrace)>)
    boost::bind(&pushTrace, boost::ref(traces), boost::ref(mutex), _1)).value();
  ASSERT_TRUE(qi::isValidSignalLink(id));

  EXPECT_EQ(3, oa1.call<int>("add", 2));
  for (unsigned i=0; i<20; ++i) {
    boost::mutex::scoped_lock l(mutex);
    if (traces.size() >= 2)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
  }
  std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
  ASSERT_EQ(2u, traces.size());
  std::sort(traces.begin(), traces.end(), comparator); // events may not be in order
  EXPECT_EQ(qi::EventTrace::Event_Call, traces[0].kind());
  EXPECT_EQ(qi::EventTrace::Event_Result, traces[1].kind());
  EXPECT_EQ(mid, traces[0].slotId());
  ASSERT_TRUE(oa1.call<bool>("isTraceEnabled"));
  oa1.disconnect(id);
  std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
  ASSERT_TRUE(!oa1.call<bool>("isTraceEnabled"));
}

static void bim(int i, qi::Promise<void>& p, const std::string &name) {
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
  sig.connect(boost::bind<void>(&bim, _1, boost::ref(plocal), "local"));
  qi::DynamicObjectBuilder gob;
  unsigned int id = gob.advertiseSignal("sig", &sig);
  ASSERT_LT(0u, id);
  qi::AnyObject obj = gob.object();
  obj.connect("sig", boost::function<void(int)>(boost::bind<void>(&bim, _1, boost::ref(premote), "remote")));

  //test remote trigger
  qiLogInfo() << "remote trigger.";
  obj.post("sig", 42);
  plocal.future().wait(1000);
  premote.future().wait(1000);
  ASSERT_TRUE(plocal.future().hasValue(0));
  ASSERT_TRUE(premote.future().hasValue(0));

  plocal = qi::Promise<void>();
  premote = qi::Promise<void>();

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
  qi::detail::printMetaObject(std::cerr, o.metaObject());
  std::string sig = o.metaObject().findMethod("callMe")[0].parametersSignature().toString();
  EXPECT_EQ(sig, "m");
  o.call<void>("callMe", 1, 2, 3);
  qi::AnyValue args = o.property<qi::AnyValue>("onCall").value();
  std::vector<int> expect = boost::assign::list_of(1)(2)(3);
  EXPECT_EQ(expect, args.to<std::vector<int> >());
}


static qi::AnyValue gCallmee;

void callMee(const qi::AnyArguments& pack) {
  gCallmee = qi::AnyValue::from(pack.args());
}

void callMee2(int i, const qi::AnyArguments& pack) {
  EXPECT_EQ(i, 42);
  gCallmee = qi::AnyValue::from(pack.args());
}

TEST(TestObject, DynAnyArguments)
{
  qi::DynamicObjectBuilder gob;
  boost::function <void (const qi::AnyArguments&)> bf = boost::bind(&callMee2, 42, _1);
  gob.advertiseMethod("callMee", &callMee);
  gob.advertiseMethod("callMee2", bf);

  ArgPack ap;
  gob.advertiseMethod("callMee3", &ap, &ArgPack::callMe);

  qi::AnyObject o = gob.object();
  qi::detail::printMetaObject(std::cerr, o.metaObject());
  std::string sig;
  sig = o.metaObject().findMethod("callMee")[0].parametersSignature().toString();
  EXPECT_EQ(sig, "m");
  sig = o.metaObject().findMethod("callMee2")[0].parametersSignature().toString();
  EXPECT_EQ(sig, "m");
  sig = o.metaObject().findMethod("callMee3")[0].parametersSignature().toString();
  EXPECT_EQ(sig, "m");

  qi::AnyValue args;
  std::vector<int> expect;
  expect.push_back(1);
  expect.push_back(2);
  expect.push_back(3);

  o.call<void>("callMee", 1, 2, 3);
  args = gCallmee;
  EXPECT_EQ(expect, args.to<std::vector<int> >());

  gCallmee = qi::AnyValue();
  o.call<void>("callMee2", 1, 2, 3);
  args = gCallmee;
  EXPECT_EQ(expect, args.to<std::vector<int> >());

  o.call<void>("callMee3", 1, 2, 3);
  args = ap.onCall.get().value();
  EXPECT_EQ(expect, args.to<std::vector<int> >());
}

TEST(TestObject, NullAnyValueToObjectThrows)
{
  ASSERT_ANY_THROW(qi::AnyValue{}.to<qi::AnyObject>());
}

TEST(TestObject, NullAnyValueConvertToRegisteredStaticTypeFails)
{
  qi::AnyValue v;
  qi::AnyObject o{boost::make_shared<ArgPack>()};
  auto result = v.convert(o.asGenericObject()->type);
  ASSERT_EQ(qi::AnyReference{}, *result);
  ASSERT_FALSE(result.ownsReference());
}

TEST(TestObject, NullObjectHasNullGenericObject)
{
  qi::AnyObject o;
  ASSERT_EQ(nullptr, o.asGenericObject());
}

TEST(TestObject, NullAnyValueConvertToObjectType)
{
  qi::AnyValue v;
  auto result = v.convert(qi::typeOf<qi::AnyObject>());
  EXPECT_FALSE(result->isValid());
  ASSERT_FALSE(result.ownsReference());
}

TEST(TestObject, NullAnyValueAsReferenceConvertToObjectType)
{
  qi::AnyValue v;
  auto r = v.asReference();
  auto result = r.convert(qi::typeOf<qi::AnyObject>());
  EXPECT_FALSE(result->isValid());
  ASSERT_FALSE(result.ownsReference());
}

TEST(TestObject, NullAnyReferenceFromAnyValueConvertToObjectType)
{
  qi::AnyValue v;
  auto r = qi::AnyReference::from(v);
  auto result = r.convert(qi::typeOf(qi::AnyObject{}));
  EXPECT_FALSE(result->isValid());
  ASSERT_FALSE(result.ownsReference());
}

TEST(TestObject, CallingWithNullArgInsteadOfObjectThrows)
{
  qi::DynamicObjectBuilder gob;
  static const std::string methodName{"callMeMaybe"};
  gob.advertiseMethod(methodName, [](qi::AnyObject){});
  auto o = gob.object();
  std::string errorMessage;
  try
  {
    o.call<void>(methodName, qi::AnyValue{});
    FAIL() << "Calling method with a null value should throw if it was expecting an object";
  }
  catch (const std::exception& e)
  {
    errorMessage = e.what();
  }

  static const std::string expectedMessageStart{"Call argument number 0 conversion failure from Invalid to Object"};
  ASSERT_EQ(0, errorMessage.compare(0, expectedMessageStart.size(), expectedMessageStart))
      << "Got unexpected error message: " << errorMessage;
}

TEST(TestObject, CallingWithNullArgWorks)
{
  qi::DynamicObjectBuilder gob;
  static const std::string methodName{"callMeMaybe"};
  gob.advertiseMethod(methodName, [](qi::AnyValue){});
  auto o = gob.object();
  o.call<void>(methodName, qi::AnyValue{}); // should not throw
}

class Sleeper
{
public:
  qi::MilliSeconds sleep(qi::MilliSeconds duration)
  {
    boost::this_thread::sleep_for(duration);
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

QI_REGISTER_OBJECT(Sleeper, sleep);

class Apple
{
  public:
    Apple(std::string type) :
      weight(42),
      type(type)
    {
      qiLogDebug() << "new Apple default 42 " << type;
    }

    Apple(int weight, std::string type) :
      weight(weight),
      type(type)
    {
      qiLogDebug() << "new Apple " << weight << ' ' << type;
    }

    int getWeight() const
    {
      return weight;
    }
    std::string getType() const
    {
      return type;
    }

  private:
    int weight;
    std::string type;
};

QI_REGISTER_OBJECT(Apple, getWeight, getType);

bool init_testmodule_module(qi::ModuleBuilder* mb) {
  mb->advertiseFactory<Sleeper>("Sleeper");
  mb->advertiseFactory<Apple, std::string>("Apple");
  mb->advertiseFactory<Apple, int, std::string>("Fruit");
  return true;
}
QI_REGISTER_MODULE_EMBEDDED("testmodule", &init_testmodule_module);

TEST(TestObject, Factory)
{
  qi::AnyModule p = qi::import("testmodule");
  ASSERT_ANY_THROW(p.call<qi::AnyObject>("Apple"));
  ASSERT_ANY_THROW(p.call<qi::AnyObject>("Apple", 33.33));
  ASSERT_ANY_THROW(p.call<qi::AnyObject>("Fruit"));
  ASSERT_ANY_THROW(p.call<qi::AnyObject>("Fruit", "aaa"));
  ASSERT_ANY_THROW(p.call<qi::AnyObject>("Fruit", 18));

  qi::AnyObject apple = p.call<qi::AnyObject>("Apple", "red");
  ASSERT_TRUE(apple);
  EXPECT_EQ(42, apple.call<int>("getWeight"));
  EXPECT_EQ("red", apple.call<std::string>("getType"));

  qi::AnyObject fruit = p.call<qi::AnyObject>("Fruit", 188, "green");
  ASSERT_TRUE(fruit);
  EXPECT_EQ(188, fruit.call<int>("getWeight"));
  EXPECT_EQ("green", fruit.call<std::string>("getType"));

}

static void calla() {
};
static void callb(bool) {
};
static void callbb(bool, bool) {
};
static void callc(int) {
};


//*  -1 : no method found
//*  -2 : arguments do not matches
//*  -3 : ambiguous matches
TEST(TestObject, findMethodErr)
{
  qi::DynamicObjectBuilder gob;
  gob.advertiseMethod("call", &calla);
  gob.advertiseMethod("callb", &callb);
  gob.advertiseMethod("callb", &callbb);
  gob.advertiseMethod("callc", &callb);
  gob.advertiseMethod("callc", &callc);


  qi::AnyObject ao = gob.object();
  qi::GenericFunctionParameters gfp;
  EXPECT_EQ(-1, ao.findMethod("callNotFound", args(42)));

  EXPECT_EQ(-2, ao.findMethod("call", args(42)));

  //should fail but it expect the call to fail later
  //EXPECT_EQ(-2, ao.findMethod("callb", args("titi")));

  EXPECT_EQ(-2, ao.findMethod("callb", args(42, 42, 42)));

  EXPECT_EQ(-2, ao.findMethod("callc", args(42, 42, 42)));
  //should fail but it expect the call to fail later
  //EXPECT_EQ(-3, ao.findMethod("callc", args(1)));
}

TEST(TestObject, WeakObject)
{
  qi::AnyObject obj;

  {
    qi::DynamicObjectBuilder b;
    b.advertiseMethod("f", &calla);
    obj = b.object();
  }

  qi::AnyWeakObject wobj = obj;
  obj = qi::AnyObject();
  ASSERT_FALSE(wobj.lock());
}

class MyFoo
{
  int _i = 0;
public:
  explicit MyFoo(int i)
    : _i(i)
  {}

  int getI() const { return _i; }
};

TEST(TestObject, ObjectShared)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, 25000);
  auto const i = dis(gen);
  auto foo = boost::make_shared<MyFoo>(i);
  qi::DynamicObjectBuilder b;
  b.advertiseMethod("getI", foo.get(), &MyFoo::getI);
  auto obj = b.object(foo);
  EXPECT_EQ(i, obj.call<int>("getI"));
}

TEST(TestObject, EqualityDynamicObjectBuilder)
{
  Foo foo;
  qi::DynamicObjectBuilder builder;

  builder.advertiseMethod("test", &fun);
  builder.advertiseMethod("objtest", &foo, &Foo::fun);
  builder.advertiseMethod("testBind", (boost::function<int(const int&)>)boost::bind(&fun, 21, _1));
  qi::AnyObject o0(builder.object());
  qi::AnyObject o1(builder.object());

  EXPECT_EQ(o0, o1);
}

TEST(TestObject, EqualityObjectTypeBuilder)
{
  qi::ObjectTypeBuilder<Adder> builder;
  // otherwise some arguments are copied
  builder.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  builder.advertiseMethod("add", &Adder::add);
  builder.advertiseMethod("addTwo", boost::function<int(Adder*, int, int)>(boost::bind(&Adder::addTwo, _2, _3)));
  Adder a{0}, b{1};
  qi::AnyObject oa0 = builder.object(&a, &qi::AnyObject::deleteGenericObjectOnly);
  qi::AnyObject oa1 = builder.object(&a, &qi::AnyObject::deleteGenericObjectOnly);
  EXPECT_EQ(oa0, oa1);
  qi::AnyObject oa2 = builder.object(&a, &qi::AnyObject::deleteGenericObjectOnly);
  qi::AnyObject ob0 = builder.object(&b, &qi::AnyObject::deleteGenericObjectOnly);
  EXPECT_NE(oa2, ob0);
  EXPECT_EQ(oa0, oa2);
  EXPECT_EQ(oa1, oa2);
  EXPECT_NE(oa0, ob0);
  EXPECT_NE(oa1, ob0);
}

TEST(TestObject, EqualityObjectTypeBuilderAsync)
{
  // We test both async calls, and calling methods with inherited argument type
  qi::ObjectTypeBuilder<MAdder> builder;
  builder.inherits<Adder>();
  builder.advertiseMethod("add", &Adder::add, qi::MetaCallType_Queued);
  builder.advertiseMethod("addTwo", boost::function<int(Adder*, int, int)>(boost::bind(&Adder::addTwo, _2, _3)));
  MAdder a{0}, b{1};
  qi::AnyObject oa0 = builder.object(&a, &qi::AnyObject::deleteGenericObjectOnly);
  qi::AnyObject oa1 = builder.object(&a, &qi::AnyObject::deleteGenericObjectOnly);
  EXPECT_EQ(oa0, oa1);
  qi::AnyObject ob0 = builder.object(&b, &qi::AnyObject::deleteGenericObjectOnly);
  qi::AnyObject ob1 = builder.object(&b, &qi::AnyObject::deleteGenericObjectOnly);
  EXPECT_EQ(ob0, ob1);
  EXPECT_NE(oa0, ob0);
  EXPECT_NE(oa1, ob1);
}

TEST(TestObject, EqualityAnyArguments)
{
  boost::shared_ptr<ArgPack> ap(new ArgPack);
  qi::AnyObject o0 = qi::AnyValue::from(ap).to<qi::AnyObject>();
  qi::AnyObject o1 = qi::AnyValue::from(ap).to<qi::AnyObject>();
  EXPECT_EQ(o0, o1);
  qi::AnyObject o2 = ap;
  EXPECT_EQ(o0, o2);
  EXPECT_EQ(o1, o2);
}

TEST(TestObject, DifferenceFactory)
{
  qi::AnyModule p = qi::import("testmodule");
  EXPECT_NE(p.call<qi::AnyObject>("Apple", "red"), p.call<qi::AnyObject>("Apple", "red"));
  {
    qi::AnyObject o0 = p.call<qi::AnyObject>("Apple", "red");
    qi::AnyObject o1 = o0;
    EXPECT_EQ(o0, o1);
  }
}

TEST(TestObject, Hash)
{
  Foo foo0, foo1;
  qi::DynamicObjectBuilder builder0, builder1;

  builder0.advertiseMethod("test", &fun);
  builder1.advertiseMethod("test", &fun);
  qi::AnyObject o0(builder0.object()), o1(builder1.object());

  std::hash<qi::AnyObject> h;
  EXPECT_EQ(h(o0), h(o0));
  EXPECT_EQ(h(o1), h(o1));
  EXPECT_NE(h(o0), h(o1));

  qi::AnyObject o2(o1);
  EXPECT_EQ(h(o1), h(o2));
}

TEST(TestObject, UnorderedMap)
{
  Foo foo0, foo1;
  qi::DynamicObjectBuilder builder0, builder1;

  builder0.advertiseMethod("test", &fun);
  builder1.advertiseMethod("test", &fun);
  qi::AnyObject o0(builder0.object()), o1(builder1.object());

  std::unordered_map<qi::AnyObject, std::string> map{
    {o0, "o0"},
    {o1, "o1"}
  };
  EXPECT_EQ(map[o0], "o0");
  EXPECT_EQ(map[o1], "o1");
}

TEST(TestObject, FlatMap)
{
  qi::DynamicObjectBuilder builder0, builder1;

  builder0.advertiseMethod("test", &fun);
  builder1.advertiseMethod("test", &fun);
  qi::AnyObject o0(builder0.object()), o1(builder1.object());

  boost::container::flat_map<qi::AnyObject, std::string> map{
    {o0, "o0"},
    {o1, "o1"}
  };
  EXPECT_EQ(map[o0], "o0");
  EXPECT_EQ(map[o1], "o1");
}

TEST(TestObject, StableVector)
{
  const size_t objCount = 4;
  qi::DynamicObjectBuilder builders[objCount] = {};
  for (auto& b: builders)
  {
    b.advertiseMethod("test", &fun);
  }
  qi::AnyObject objects[objCount];
  int i = 0;
  for (auto& b: builders)
  {
    objects[i++] = b.object();
  }
  boost::container::stable_vector<qi::AnyObject> vec;
  for (auto& b: builders)
  {
    vec.push_back(b.object());
  }
  i = 0;
  for (const auto& o: vec)
  {
    EXPECT_EQ(o, objects[i++]);
  }
  std::sort(vec.begin(), vec.end());
}

TEST(TestObject, ObjectSharedLife)
{
  boost::weak_ptr<Foo> wFoo;
  Foo *fooPtr;
  {
    qi::AnyObject obj;
    {
      auto foo = boost::make_shared<Foo>();
      fooPtr = foo.get();
      wFoo = foo;
      qi::DynamicObjectBuilder b;
      b.advertiseMethod("sum", foo.get(), &Foo::fun);
      obj = b.object(foo);
    }
    EXPECT_EQ(fooPtr, wFoo.lock().get());
  }
  EXPECT_FALSE(wFoo.lock());
}

namespace
{
  /// Predicate<qi::Object> P
  template<typename P>
  void testValidity(P isValid)
  {
    using O = qi::Object<Apple>;
    {
      O obj = boost::make_shared<Apple>("kjhd");
      EXPECT_TRUE(isValid(obj));
      EXPECT_TRUE(isValid(O(obj)));
    }
    EXPECT_FALSE(isValid(O()));
    EXPECT_TRUE(isValid(O(static_cast<Apple*>(nullptr)))); // ...
    EXPECT_TRUE(isValid(O(boost::shared_ptr<Apple>()))); // ...
  }
} // namespace

TEST(TestObject, IsValid)
{
  SCOPED_TRACE("IsValid");
  testValidity([](const qi::Object<Apple>& o) {return o.isValid();});
}

TEST(TestObject, ConversionToBool)
{
  SCOPED_TRACE("ConversionToBool");
  testValidity([](const qi::Object<Apple>& o) {return static_cast<bool>(o);});
}

struct Exhaustive
{
  void call() {}
  qi::Property<int> prop;
  qi::Signal<> sig;
};
QI_REGISTER_OBJECT(Exhaustive, call, prop, sig);

TEST(TestObject, NullObjectApiThrows)
{
  qi::Object<Exhaustive> obj;

  EXPECT_THROW(obj.property<int>("prop").value(), std::runtime_error);
  EXPECT_THROW(obj.setProperty("prop", 42).value(), std::runtime_error);
  EXPECT_THROW(obj.connect("prop", [](int){}).value(), std::runtime_error);
  EXPECT_THROW(obj.connect("sig", []{}).value(), std::runtime_error);
  EXPECT_THROW(obj.disconnect(qi::SignalLink{ 42 }).value(), std::runtime_error);
  EXPECT_THROW(obj.post("sig"), std::runtime_error);
  EXPECT_THROW(obj.findMethod("call", qi::GenericFunctionParameters{}), std::runtime_error);
  EXPECT_THROW(obj.call<void>("call"), std::runtime_error);
  EXPECT_THROW(obj.async<void>("call").value(), std::runtime_error);
}
