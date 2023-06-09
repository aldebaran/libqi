/*
 *  Author(s):
 *  - Pierre Roullon <proullon@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include <array>

#include <gtest/gtest.h>

#include <boost/assign/list_of.hpp>

#include <qi/application.hpp>
#include <qi/eventloop.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/session.hpp>
#include <qi/testutils/testutils.hpp>
#include <ka/errorhandling.hpp>
#include <ka/functional.hpp>
#include <testsession/testsessionpair.hpp>
#include "src/messaging/message.hpp"

qiLogCategory("test");

static const qi::MilliSeconds usualTimeout{200};

int getint()
{
  return 42;
}

bool getbool()
{
  return false;
}

std::string getstring()
{
  return "lol";
}

float getfloat()
{
  return 42.0f;
}

void getvoid()
{
}

namespace qi
{
  std::ostream& operator<<(std::ostream& os, const AnyReference& ref)
  {
    os << "Signature: " << ref.signature().toPrettySignature()
       << ", Type: " << ref.type()->infoString()
       << ", RawValue: " << ref.rawValue();
    return os;
  }
}

TEST(TestCall, Convert)
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("getint", &getint);
  ob.advertiseMethod("getbool", &getbool);
  ob.advertiseMethod("getstring", &getstring);
  ob.advertiseMethod("getfloat", &getfloat);
  ob.advertiseMethod("getvoid", &getvoid);

  TestSessionPair p;
  p.server()->registerService("Serv", ob.object());
  qi::AnyObject obj = p.client()->service("Serv").value();

  EXPECT_EQ(42, obj.call<float>("getint"));
  EXPECT_ANY_THROW(obj.call<bool>("getint"));
  EXPECT_ANY_THROW(obj.call<std::string>("getint"));

  EXPECT_EQ(42.0, obj.call<int>("getfloat"));
  EXPECT_ANY_THROW(obj.call<bool>("getfloat"));
  EXPECT_ANY_THROW(obj.call<std::string>("getfloat"));

  EXPECT_FALSE(obj.call<bool>("getbool"));
  EXPECT_EQ(0, obj.call<int>("getbool"));
  EXPECT_EQ(0.0, obj.call<float>("getbool"));

  EXPECT_ANY_THROW(obj.call<int>("getvoid"));
  EXPECT_ANY_THROW(obj.call<std::string>("getvoid"));
  EXPECT_ANY_THROW(obj.call<float>("getvoid"));
  EXPECT_ANY_THROW(obj.call<bool>("getvoid"));

  EXPECT_EQ("lol", obj.call<std::string>("getstring"));
  EXPECT_ANY_THROW(obj.call<int>("getstring"));
  EXPECT_ANY_THROW(obj.call<float>("getstring"));
  EXPECT_ANY_THROW(obj.call<bool>("getstring"));
}

int addOne(int v)
{
  qiLogDebug() << "addOne";
  return v+1;
}

long unsigned int print(std::list<std::pair<std::string, int> > robots)
{
  for(std::list<std::pair<std::string, int> >::iterator it = robots.begin(); it != robots.end(); ++it)
    std::cout << "Robot " << (*it).first << " has serial ID " << (*it).second << std::endl;

  return robots.size();
}

void foobar() {
  return;
}

void fooerr() {
  throw std::runtime_error("foobar");
}

int fakeRGBf(const std::string& /*name*/, int /*value*/, float /*duration*/)
{
  return (0);
}

int fakeRGBd(const std::string& /*name*/, int /*value*/, double /*duration*/)
{
  return (0);
}

int fakeemptysvec(const std::vector<std::string> &svec) {
  std::cout << "svec.size(): " << svec.size() << std::endl;
  EXPECT_TRUE(svec.empty());
  return 0;
}

int fakeemptygvec(const std::vector<qi::AnyValue> &sval) {
  std::cout << "sval.size(): " << sval.size() << std::endl;
  EXPECT_TRUE(sval.empty());
  return 0;
}

int fakesvec(const std::vector<std::string> &svec) {
  std::cout << "svec.size(): " << svec.size() << std::endl;
  EXPECT_EQ("titi", svec[0]);
  EXPECT_EQ("toto", svec[1]);
  return 0;
}

int fakegvec(const std::vector<qi::AnyValue> &sval) {
  std::cout << "sval.size(): " << sval.size() << std::endl;
  EXPECT_EQ("titi", sval[0].toString());
  EXPECT_EQ("toto", sval[1].toString());
  return 0;
}

int fakesvvec(const std::vector< std::vector<std::string> > &svec) {
  std::cout << "svec.size(): " << svec.size() << std::endl;
  EXPECT_EQ("titi", svec[0][0]);
  EXPECT_EQ("toto", svec[1][0]);
  return 0;
}

int fakegvvec(const std::vector< std::vector<qi::AnyValue> > &sval) {
  std::cout << "sval.size(): " << sval.size() << std::endl;
  EXPECT_EQ("titi", sval[0][0].toString());
  EXPECT_EQ("toto", sval[1][0].toString());
  return 0;
}

int fakegvvec2(const std::vector< qi::AnyValue > &sval) {
  std::cout << "sval.size(): " << sval.size() << std::endl;
  EXPECT_EQ("titi", sval[0].to< std::vector<std::string> >()[0]);
  EXPECT_EQ("toto", sval[1].to< std::vector<std::string> >()[0]);
  return 0;
}

int fakesmvvec(std::map<std::string, std::vector< std::vector<std::string> > > &svec) {
  std::cout << "svec.size(): " << svec.size() << std::endl;
  EXPECT_EQ("titi", svec["i"][0][0]);
  EXPECT_EQ("toto", svec["j"][0][0]);
  return 0;
}

int fakegmvvec(std::map<std::string, std::vector< std::vector<qi::AnyValue> > > &sval) {
  std::cout << "sval.size(): " << sval.size() << std::endl;
  EXPECT_EQ("titi", sval["i"][0][0].toString());
  EXPECT_EQ("toto", sval["j"][0][0].toString());
  return 0;
}

struct GenericTuple
{
  qi::AnyValue e1;
  qi::AnyValue e2;
  qi::AnyValue e3;
};
QI_TYPE_STRUCT(GenericTuple, e1, e2, e3)

struct SpecificTuple
{
  int e1;
  double e2;
  std::map<std::string, float> e3;
};
QI_TYPE_STRUCT(SpecificTuple, e1, e2, e3)

double eatSpecific(const SpecificTuple& v)
{
  if (v.e3.find("foo") != v.e3.end())
    return v.e1 + v.e2 + v.e3.find("foo")->second;
  else
    return v.e1 + v.e2;
}

qi::AnyValue pingCopy(qi::AnyValue arg, qi::AnyValue& target)
{
  target = arg;
  return arg;
}

TEST(TestCall, CallBufferInList)
{
  TestSessionPair          p;
  qi::DynamicObjectBuilder ob;
  qi::AnyValue val;
  ob.advertiseMethod("pingcopy",
    boost::function<qi::AnyValue(qi::AnyValue)>(boost::bind(&pingCopy, _1, boost::ref(val))));
  qi::AnyObject obj(ob.object());
  p.server()->registerService("test", obj);
  qi::AnyObject proxy = p.client()->service("test").value();
  std::vector<qi::AnyValue> args;
  args.push_back(qi::AnyValue::from(12));
  qi::Buffer buf;
  buf.write("canard", strlen("canard")+1);
  args.push_back(qi::AnyValue::from(buf));
  args.push_back(qi::AnyValue::from("foo"));
  qi::AnyValue result = proxy.call<qi::AnyValue>("pingcopy", args);
  {
    ASSERT_EQ(3u, val.size());
    ASSERT_EQ(12, val[0].toInt());
    qi::Buffer bufRes = val[1].to<qi::Buffer>();
    ASSERT_EQ(strlen("canard")+1, bufRes.size());
    ASSERT_EQ("foo", val[2].toString());
  }
  {
    std::vector<qi::AnyValue> l = result.to<std::vector<qi::AnyValue> >();
    ASSERT_EQ(3u, l.size());
    std::vector<qi::AnyValue>::iterator it = l.begin();
    ASSERT_EQ(12, (*it).toInt());
    ++it;
    qi::Buffer bufRes = (*it).to<qi::Buffer>();
    ASSERT_EQ(strlen("canard")+1, bufRes.size());
    ++it;
    ASSERT_EQ("foo", (*it).toString());
  }

}

TEST(TestCall, CallComplexType)
{
  std::list<std::pair<std::string, int> >  robots;
  TestSessionPair          p;
  qi::DynamicObjectBuilder ob;

  ob.advertiseMethod("print", &print);
  qi::AnyObject obj(ob.object());

  const auto serviceID = p.server()->registerService("serviceCall", obj).value();

  qi::AnyObject proxy = p.client()->service("serviceCall").value();

  robots.push_back(std::make_pair("Gibouna", 1234567));
  robots.push_back(std::make_pair("Wall-E", 2345678));
  robots.push_back(std::make_pair("Billy West", 3456789));
  robots.push_back(std::make_pair("33CL", 4567890));

  ASSERT_EQ(4u, proxy.call<long unsigned int>("print", robots));

  p.server()->unregisterService(serviceID);
}


TEST(TestCall, CallVoid)
{
  std::list<std::pair<std::string, int> >  robots;
  TestSessionPair          p;
  qi::DynamicObjectBuilder ob;

  ob.advertiseMethod("foobar", &foobar);
  qi::AnyObject obj(ob.object());

  const auto serviceID = p.server()->registerService("serviceCall", obj).value();

  qi::AnyObject proxy = p.client()->service("serviceCall").value();


  std::cout << "Calling" << std::endl;
  qi::Future<void> fut = proxy.async<void>("foobar");
  ASSERT_TRUE(test::finishesWithValue(fut));
  p.server()->unregisterService(serviceID);
}

struct Add
{
  int a;
  Add(int a) : a(a) {}
  inline int add(int b) const noexcept { return a + b; }
};
QI_REGISTER_OBJECT(Add, add);

TEST(TestCall, CallNoExcept)
{
  TestSessionPair p;
  p.server()->registerService("Add", boost::make_shared<Add>(329)).value();
  const auto clientAdd = p.client()->service("Add").value();
  const auto res = clientAdd.call<int>("add", 938);
  EXPECT_EQ(res, 1267);
}

TEST(TestCall, CallNoExceptFunctionObject)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder builder;
  auto add = boost::make_shared<Add>(321);
  builder.advertiseMethod("addImpl", add.get(), &Add::add);
  builder.advertiseMethod("add", [](const qi::AnyObject& self, int b) noexcept {
    return self.call<int>("add", b);
  });
  p.server()->registerService("Add", builder.object(add)).value();
  const auto clientAdd = p.client()->service("Add").value();
  const auto res = clientAdd.call<int>("add", add, 123);
  EXPECT_EQ(res, 444);
}

TEST(TestCall, CallVoidErr)
{
  std::list<std::pair<std::string, int> >  robots;
  TestSessionPair          p;
  qi::DynamicObjectBuilder ob;

  ob.advertiseMethod("fooerr", &fooerr);
  qi::AnyObject obj(ob.object());

  const auto serviceID = p.server()->registerService("serviceCall", obj).value();

  qi::AnyObject proxy = p.client()->service("serviceCall").value();

  std::cout << "Calling" << std::endl;
  qi::Future<void> fut = proxy.async<void>("fooerr");
  ASSERT_TRUE(test::finishesWithError(fut));
  p.server()->unregisterService(serviceID);
}

TEST(TestCall, TestDoubleToFloatConvertion)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder ob;
  double duration = 0.42;

  ob.advertiseMethod("fakeRGB", &fakeRGBf);
  qi::AnyObject obj(ob.object());

  const auto serviceID = p.server()->registerService("serviceConv", obj).value();
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv").value();
  ASSERT_TRUE(proxy);

  std::cout << "Calling FakeRGB" << std::endl;
  qi::Future<int> fut = proxy.async<int>("fakeRGB", "Haha", 42, duration);
}

TEST(TestCall, TestFloatToDoubleConvertion)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder ob;
  float duration = 0.42f;

  ob.advertiseMethod("fakeRGB", &fakeRGBd);
  qi::AnyObject obj(ob.object());

  const auto serviceID = p.server()->registerService("serviceConv", obj).value();
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv").value();
  ASSERT_TRUE(proxy);

  std::cout << "Calling FakeRGB" << std::endl;
  qi::Future<int> fut = proxy.async<int>("fakeRGB", "Haha", 42, duration);
}

struct TestCallOptional : testing::Test
{
  void SetUp() override
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseMethod(funcName, [](boost::optional<int> o){
      return o ? *o : -1;
    });

    serviceId = p.server()->registerService(serviceName, ob.object()).value();
    ASSERT_TRUE(serviceId != 0);

    proxy = p.client()->service(serviceName).value();
    ASSERT_TRUE(proxy);
  }

  void TearDown() override
  {
    p.server()->unregisterService(serviceId);
  }

  const std::string serviceName = "serviceConv";
  const std::string funcName = "funcOpt";
  TestSessionPair p;
  unsigned int serviceId;
  qi::AnyObject proxy;
};

// The function can be called with a optional<int> with a value.
TEST_F(TestCallOptional, CanBeCalledWithOptIntWithValue)
{
  auto fut = proxy.async<int>(funcName, boost::make_optional(42));
  int val;
  ASSERT_TRUE(test::finishesWithValue(fut, test::willAssignValue(val)));
  EXPECT_EQ(42, val);
}

// The function can be called with a optional<int> without a value.
TEST_F(TestCallOptional, CanBeCalledWithOptIntWithoutValue)
{
  auto fut = proxy.async<int>(funcName, boost::optional<int>());
  int val;
  EXPECT_TRUE(test::finishesWithValue(fut, test::willAssignValue(val)));
  EXPECT_EQ(-1, val);
}

// The function can be called with boost::none.
TEST_F(TestCallOptional, CanBeCalledWithOptNone)
{
  auto fut = proxy.async<int>(funcName, boost::none);
  int val;
  EXPECT_TRUE(test::finishesWithValue(fut, test::willAssignValue(val)));
  EXPECT_EQ(-1, val);
}

// The function can be called with an int (implicit conversion from T to Opt<T>).
TEST_F(TestCallOptional, CanBeCalledWithInt)
{
  auto fut = proxy.async<int>(funcName, 42);
  int val;
  EXPECT_TRUE(test::finishesWithValue(fut, test::willAssignValue(val)));
  EXPECT_EQ(42, val);
}

// The function can be called with a optional<AnyValue> with an int value.
TEST_F(TestCallOptional, CanBeCalledWithOptAnyValueWithIntValue)
{
  auto fut = proxy.async<int>(funcName, boost::make_optional(qi::AnyValue::from(13)));
  int val;
  EXPECT_TRUE(test::finishesWithValue(fut, test::willAssignValue(val)));
  EXPECT_EQ(13, val);
}

// The function can be called with a optional<AnyValue> without a value.
TEST_F(TestCallOptional, CanBeCalledWithOptAnyValueWithoutValue)
{
  auto fut = proxy.async<int>(funcName, boost::optional<qi::AnyValue>());
  int val;
  EXPECT_TRUE(test::finishesWithValue(fut, test::willAssignValue(val)));
  EXPECT_EQ(-1, val);
}

// The function can be called with a optional<AnyValue> with a value of another type we can convert
// from.
TEST_F(TestCallOptional, CanBeCalledWithOptAnyValueWithValueOfConvertibleType)
{
  auto fut = proxy.async<int>(funcName, boost::make_optional(qi::AnyValue::from(true)));
  int val;
  EXPECT_TRUE(test::finishesWithValue(fut, test::willAssignValue(val)));
  EXPECT_EQ(1, val);
}

// The function cannot be called with a optional<AnyValue> with a value of a type we can't convert
// from.
TEST_F(TestCallOptional, FailsIfCalledWithOptAnyValueWithValueOfNonConvertibleType)
{
  auto fut = proxy.async<int>(funcName, boost::make_optional(qi::AnyValue::from("foo")));
  EXPECT_TRUE(test::finishesWithError(fut));
}

qi::AnyObject createObject() {
  qi::DynamicObjectBuilder ob;

  ob.advertiseMethod("fakesvec", &fakesvec);
  ob.advertiseMethod("fakegvec", &fakegvec);
  ob.advertiseMethod("fakeemptysvec", &fakeemptysvec);
  ob.advertiseMethod("fakeemptygvec", &fakeemptygvec);

  ob.advertiseMethod("fakesvvec", &fakesvvec);
  ob.advertiseMethod("fakegvvec", &fakegvvec);
  ob.advertiseMethod("fakegvvec2", &fakegvvec2);
  ob.advertiseMethod("fakemsvvec", &fakesmvvec);
  ob.advertiseMethod("fakemgvvec", &fakegmvvec);

  ob.advertiseMethod("eatSpecific", &eatSpecific);
  return ob.object();
}

TEST(TestCall, TestGenericConversion) {
  TestSessionPair p;
  qi::AnyObject obj = createObject();
  const auto serviceID = p.server()->registerService("serviceConv", obj).value();
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv").value();
  ASSERT_TRUE(proxy);

  std::vector<std::string>      svec;
  std::vector<qi::AnyValue> gvec;

  std::vector<std::string>      esvec;
  std::vector<qi::AnyValue> egvec;

  svec.push_back("titi");
  svec.push_back("toto");

  qi::AnyValue gv;
  gv = qi::AnyValue::from(std::string("titi"));
  gvec.push_back(gv);
  gv = qi::AnyValue::from(std::string("toto"));
  gvec.push_back(gv);

  qi::Future<int> fut;

  //Check empty, same type
  fut = proxy.async<int>("fakeemptysvec", esvec);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakeemptygvec", egvec);
  EXPECT_TRUE(test::finishesWithValue(fut));

  //check call, same type
  fut = proxy.async<int>("fakesvec", svec);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakegvec", gvec);
  EXPECT_TRUE(test::finishesWithValue(fut));

  //check empty, type conv
  fut = proxy.async<int>("fakeemptysvec", egvec);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakeemptygvec", esvec);
  EXPECT_TRUE(test::finishesWithValue(fut));

  //check call, type conv
  fut = proxy.async<int>("fakesvec", gvec);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakegvec", svec);
  EXPECT_TRUE(test::finishesWithValue(fut));
}

TEST(TestCall, TestGenericConversionComplexList) {
  TestSessionPair p;
  qi::AnyObject obj = createObject();
  const auto serviceID = p.server()->registerService("serviceConv", obj).value();
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv").value();
  ASSERT_TRUE(proxy);


  std::vector<std::vector<std::string> >      sss;
  sss.resize(2);
  sss[0].push_back("titi");
  sss[1].push_back("toto");

  std::vector<std::vector<qi::AnyValue> > ssg;
  ssg.resize(2);
  ssg[0].push_back(qi::AnyValue::from(std::string("titi")));
  ssg[1].push_back(qi::AnyValue::from(std::string("toto")));

  std::vector<qi::AnyValue>               sg;
  std::vector<qi::AnyValue> paf;
  paf.push_back(qi::AnyValue::from(std::string("titi")));
  sg.push_back(qi::AnyValue::from(paf));
  paf[0] = qi::AnyValue::from(std::string("toto"));
  sg.push_back(qi::AnyValue::from(paf));

  qi::Future<int> fut;

  //Check empty, same type
  fut = proxy.async<int>("fakesvvec", sss);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakesvvec", ssg);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakesvvec", sg);
  EXPECT_TRUE(test::finishesWithValue(fut));

  fut = proxy.async<int>("fakegvvec", sss);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakegvvec", ssg);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakegvvec", sg);
  EXPECT_TRUE(test::finishesWithValue(fut));


  fut = proxy.async<int>("fakegvvec2", sss);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakegvvec2", ssg);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakegvvec2", sg);
  EXPECT_TRUE(test::finishesWithValue(fut));
}

TEST(TestCall, TestGenericConversionComplexMap) {
  TestSessionPair p;
  qi::AnyObject obj = createObject();
  const auto serviceID = p.server()->registerService("serviceConv", obj).value();
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv").value();
  ASSERT_TRUE(proxy);


  std::vector<std::vector<std::string> >      sss;
  std::vector<std::vector<qi::AnyValue> > ssg;

  std::map<std::string, std::vector<std::vector<std::string> > >      msvvs;
  std::map<std::string, std::vector<std::vector<qi::AnyValue> > > msvvg;

  sss.resize(2);
  ssg.resize(2);
  sss[0].push_back("titi");
  ssg[0].push_back(qi::AnyValue::from(std::string("titi")));
  msvvs["i"] = sss;
  msvvg["i"] = ssg;

  sss[0][0] = "toto";
  ssg[0][0] = qi::AnyValue::from(std::string("toto"));
  msvvs["j"] = sss;
  msvvg["j"] = ssg;

  qi::Future<int> fut;

  fut = proxy.async<int>("fakemsvvec", msvvs);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakemsvvec", msvvg);
  EXPECT_TRUE(test::finishesWithValue(fut));

  fut = proxy.async<int>("fakemgvvec", msvvs);
  EXPECT_TRUE(test::finishesWithValue(fut));
  fut = proxy.async<int>("fakemgvvec", msvvg);
  EXPECT_TRUE(test::finishesWithValue(fut));
}


TEST(TestCall, TestGenericConversionTuple) {
  TestSessionPair p;
  qi::AnyObject obj = createObject();
  const auto serviceID = p.server()->registerService("serviceConv", obj).value();
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv").value();
  ASSERT_TRUE(proxy);

  qi::Future<double> f;
  SpecificTuple t;
  t.e1 = 1;
  t.e2 = 2;
  t.e3["foo"] = 3;
  f = proxy.async<double>("eatSpecific", t);
  ASSERT_TRUE(test::finishesWithValue(f));
  EXPECT_EQ(6, f.value());

  GenericTuple gt;
  gt.e1 = qi::AnyReference::from(1.0);
  gt.e2 = qi::AnyReference::from(2U);
  std::map<std::string, qi::AnyValue> map;
  map["foo"] = qi::AnyReference::from(3);
  gt.e3 = qi::AnyReference::from(map);
  f = proxy.async<double>("eatSpecific", gt);
  ASSERT_TRUE(test::finishesWithValue(f));
  EXPECT_EQ(6, f.value());

  std::map<unsigned int, std::string> ravMap;
  gt.e3 = qi::AnyReference::from(ravMap);
  f = proxy.async<double>("eatSpecific", gt);
  ASSERT_TRUE(test::finishesWithValue(f));
  EXPECT_EQ(3, f.value());
}


TEST(TestCall, PairClientListen)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("addOne", &addOne);
  qi::AnyObject obj(ob.object());
  auto client = p.client();
  client->listen("tcp://localhost:0").value();
  client->registerService("adder", obj);
  qi::AnyObject o = p.server()->service("adder").value();
  ASSERT_TRUE(o);
}

void onEvent(int v, qi::Promise<int>& eventValue, qi::AnyObject* ptr)
{
  qiLogDebug() << "onEvent";
  if (v == 0)
  {
    qi::AnyWeakObject weak = *ptr;
    delete ptr;
    if (weak.lock())
      qiLogWarning() << "Object destruction is going to be asynchronous";
  }
  eventValue.setValue(v);
}


void bindObjectEvent(qi::AnyObject ptr, const std::string& eventName,
  qi::Promise<int>& eventValue)
{
  // Keep ptr alive
  ptr.connect(eventName, boost::function<void(int)>(boost::bind(&onEvent, _1, boost::ref(eventValue),
    new qi::AnyObject(ptr))));
}

int makeObjectCall(qi::AnyObject ptr, const std::string& fname, int arg)
{
  return ptr.call<int>(fname, arg);
}

void bounceFuture(qi::Future<int> s, qi::Promise<int> d, qi::AnyObject)
{
  if (s.hasValue())
    d.setValue(s.value());
  else
    d.setError(s.error());
}

void onMakeObjectCall(qi::AnyObject ptr, const std::string& fname, int arg,
  qi::Promise<int>& result)
{
  qiLogDebug() << "onMakeObjectCall";
  qi::Future<int> fut = ptr.async<int>(fname, arg);
  // We must keep ptr alive until the call returns
  fut.connect(boost::bind(&bounceFuture, _1, result, ptr));
}

TEST(TestCall, TestObjectPassing)
{
  TestSessionPair p;
  qi::Promise<int> eventValue;

  auto testInfo = testing::UnitTest::GetInstance()->current_test_info();
  auto logPrefix = std::string(testInfo->test_case_name()) + "." + testInfo->name() + " - ";

  qiLogInfo() << logPrefix <<  "Creating object.";

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("makeObjectCall", &makeObjectCall);
  ob.advertiseMethod("bindObjectEvent", boost::function<void(qi::AnyObject, const std::string&)>(boost::bind(&bindObjectEvent, _1, _2, boost::ref(eventValue))));
  qi::AnyObject obj(ob.object());

  qiLogInfo() << logPrefix << "Registering service 's' on server.";
  p.server()->registerService("s", obj);

  qiLogInfo() << logPrefix << "Fetching service 's' on client.";
  qi::AnyObject proxy = p.client()->service("s").value();

  qi::AnyObject unregisteredObj;
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseMethod("add", &addOne);
    ob.advertiseSignal<int>("fire");
    unregisteredObj = ob.object();
  }

  // Transmit unregisteredObj through the network.
  qiLogInfo() << logPrefix << "Calling 'makeObjectCall' on client proxy object.";
  qi::Future<int> v = proxy.async<int>("makeObjectCall", unregisteredObj, "add", 1);
  ASSERT_TRUE(test::finishesWithValue(v));
  ASSERT_EQ(2, v.value());

  qiLogInfo() << logPrefix << "Calling 'bindObjectEvent' on client proxy object.";
  proxy.call<void>("bindObjectEvent", unregisteredObj, "fire");

  qiLogInfo() << logPrefix << " - Triggering 'fire(42)' event.";
  unregisteredObj.post("fire", 42);
  ASSERT_TRUE(test::finishesWithValue(eventValue.future()));
  ASSERT_EQ(42, eventValue.future().value());
  eventValue = qi::Promise<int>();

  // Check that object is locked by remote end
  qiLogInfo() << logPrefix << "Releasing local reference to passed object.";
  qi::AnyWeakObject unregisteredWeakObj = unregisteredObj;
  unregisteredObj.reset();
  unregisteredObj = unregisteredWeakObj.lock();
  ASSERT_TRUE(unregisteredObj);

  // This will delete the proxy
  qiLogInfo() << logPrefix << "Triggering 'fire(0)' event.";
  unregisteredObj.post("fire", 0);
  ASSERT_TRUE(test::finishesWithValue(eventValue.future()));
  ASSERT_EQ(0, eventValue.future().value());

  eventValue = qi::Promise<int>();
  ASSERT_TRUE(test::isStillRunning(eventValue.future(), test::willDoNothing(), qi::MilliSeconds{0}));

  // The following check is only useful if the mode is not "direct". Otherwise,
  // the `unregisteredObj` is shared by the client and the server (since they
  // are the same), therefore keeping it alive in this scope ensures that the
  // signal callback is called. Consequently, in "direct" mode, the `eventValue`
  // promise ends up being set with a value, which causes the test below to fail.
  if (p.mode() != TestMode::Mode_Direct)
  {
    qiLogInfo() << logPrefix << "Triggering 'fire(1)' event.";
    unregisteredObj.post("fire", 1);
    ASSERT_TRUE(test::isStillRunning(eventValue.future()));
  }

  // Check that unregisteredObj is no longer held
  unregisteredObj.reset();
  ASSERT_FALSE(unregisteredWeakObj.lock());
}

TEST(TestCall, TestObjectPassingReverse)
{ // Test server->client object passing (through emit)
  TestSessionPair p;
  qi::DynamicObjectBuilder ob;
  ob.advertiseSignal<qi::AnyObject, const std::string&, int>("makeObjectCallEvent");

  qi::AnyObject obj(ob.object());

  p.server()->registerService("s", obj);
  qi::AnyObject proxy = p.client()->service("s").value();

  qi::AnyObject unregisteredObj;
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseMethod("add", &addOne);
    ob.advertiseSignal<int>("fire");
    unregisteredObj = ob.object();
  }

  qi::Promise<int> value;
  // We connect a method client-side
  boost::function<void(qi::AnyObject, const std::string&, int)> makeObjectCallEvent =
    boost::bind(&onMakeObjectCall, _1, _2, _3, value);
  proxy.connect("makeObjectCallEvent", makeObjectCallEvent).wait(usualTimeout);
  // And emit server-side, this is the reverse of a method call
  obj.post("makeObjectCallEvent", unregisteredObj, "add", 41);
  /* This is what happens:
  - ob sends EVENT(makeObjectCall, unregisterObb, "add", 41) to obproxy
  - obProxy receives EVENT(makeObjetCall, unregisteredObProxy, "add", 41)
  - The slot onMakeObjectCall(unregisteredObProxy, "add", 41) is invoked
  - It invokes unregisteredObProxy->call("add", 41)
  - This sends CALL("add", 41) back on the socket
  - UnregisteredOb reiceives the call, execute, the result is send on the socket
  - onMetaObjectCall receives the result, put it in value.
  - TADAAA
  */

  ASSERT_TRUE(test::finishesWithValue(value.future()));
  ASSERT_EQ(42, value.future().value());
}

qi::AnyObject makeAdder(qi::AnyWeakObject& weak)
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("add", &addOne);
  ob.advertiseSignal<int>("fire");
  qi::AnyObject res = ob.object();
  qiLogDebug() << "unregistered object is " << res.asGenericObject();
  weak = res;
  return res;
}

TEST(TestCall, TestObjectPassingReturn)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder ob;
  qi::AnyWeakObject weak;
  ob.advertiseMethod("makeAdder", boost::function<qi::AnyObject()>(boost::bind(&makeAdder, boost::ref(weak))));
  qi::AnyObject obj(ob.object());

  p.server()->registerService("s", obj);
  qi::AnyObject proxy = p.client()->service("s").value();

  qi::AnyObject adder = proxy.call<qi::AnyObject>("makeAdder");
  ASSERT_TRUE(weak.lock());
  // One is client side, the other server side
  if (p.client() != p.server())
  {
    ASSERT_FALSE(weak.lock().asGenericObject() == adder.asGenericObject());
  }
  ASSERT_TRUE(adder);
  qi::Future<int> f = adder.async<int>("add", 41);
  ASSERT_TRUE(test::finishesWithValue(f));
  ASSERT_EQ(42, f.value());
  adder.reset();
  std::this_thread::sleep_for(std::chrono::milliseconds{ 300 });
  ASSERT_FALSE(weak.lock());
}

TEST(TestCall, TestConnectLambda)
{ // Test calling a anyValue.connect() with a lambda instead of a boost::function (redmine Feature #33409)

  TestSessionPair p;
  qi::DynamicObjectBuilder ob;
  ob.advertiseSignal<qi::AnyObject, const std::string&, int>("makeObjectCallEvent");

  qi::AnyObject obj(ob.object());

  p.server()->registerService("s", obj);
  qi::AnyObject proxy = p.client()->service("s").value();

  qi::AnyObject unregisteredObj;
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseMethod("add", &addOne);
    ob.advertiseSignal<int>("fire");
    unregisteredObj = ob.object();
  }

  qi::Promise<int> value;
  // We connect a method client-side
  proxy.connect("makeObjectCallEvent", [&](qi::AnyObject ptr, const std::string& fname, int arg) {
      return onMakeObjectCall(ptr, fname, arg, value);
  }).wait(usualTimeout);
  obj.post("makeObjectCallEvent", unregisteredObj, "add", 41);
  ASSERT_TRUE(test::finishesWithValue(value.future()));
  ASSERT_EQ(42, value.future().value());
}

TEST(TestCall, TestReturnSharedPtrRef)
{
  static const auto ptr = boost::make_shared<int>(9999);
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("over9000", [&]() -> const boost::shared_ptr<int>& { return ptr; });
  qi::AnyObject obj(ob.object());

  auto p = obj.call<boost::shared_ptr<int>>("over9000");
  ASSERT_EQ(9999, *p);
}

class TestClassInterface
{
public:
  virtual ~TestClassInterface() = default;
  virtual qi::Future<int> ping(int v) = 0;
  qi::Property<int> prop;
};

QI_REGISTER_MT_OBJECT(TestClassInterface, ping, prop)

class TestClass : public TestClassInterface
{
public:
  TestClass() : v(0) {}
  TestClass(int v) : v(v) {}
  ~TestClass()
  {
    qiLogDebug() << "~TestClass " << this;
    ++destructionCount;
  }
  qi::Future<int> ping(int w) override
  {
    qiLogDebug() << "TestClass::ping " << this << ' ' << v << ' ' << w;
    return qi::Future<int>(v+w);
  }
  static boost::shared_ptr<TestClass> make(int v)
  {
    TestClass* tc = new TestClass(v);
    qiLogDebug() << "Making a TestClass " << v << ' ' << tc;
    return boost::shared_ptr<TestClass>(tc);
  }
  bool unregisterService(qi::Session* session, unsigned int sid, qi::Atomic<int>* checker)
  {
    session->unregisterService(sid).wait(usualTimeout);
    std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
    return !(checker->load());
  }
  int v;
  static qi::Atomic<int> destructionCount;
};

qi::Atomic<int> TestClass::destructionCount;

QI_REGISTER_MT_OBJECT(TestClass, ping, unregisterService, prop);

TEST(TestCall, TestConcreteObjectPassingReturn)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("getTest", &TestClass::make);
  qi::AnyObject obj(ob.object());
  p.server()->registerService("getter", obj);
  qi::AnyObject proxy = p.client()->service("getter").value();
  qi::AnyObject test = proxy.call<qi::AnyObject>("getTest", 1);
  ASSERT_EQ(3, test.call<int>("ping", 2));
}

char          pingChar(char v) { return v; }
unsigned char pingUChar(unsigned char v) { return v;}
int           pingInt(int v) { return v;}
int           pingUInt(unsigned int v) { return v;}
std::string   pingString(const std::string& s) { return s;}

#define EXPECTTHROW(exp) do {                   \
  try {                                         \
    exp;                                        \
    qiLogError() << "Did not throw: " << #exp;  \
    EXPECT_TRUE(false);                         \
  }                                             \
  catch(const std::exception& e)                \
  {                                             \
    qiLogDebug() << "throw: " << e.what();      \
    EXPECT_TRUE(true);                          \
  }                                             \
} while(0)


TEST(TestCall, Overflow)
{
  using namespace qi;
  TestSessionPair p;
  DynamicObjectBuilder ob;
  ob.advertiseMethod("pingChar", &pingChar);
  ob.advertiseMethod("pingUChar", &pingUChar);
  ob.advertiseMethod("pingInt", &pingInt);
  ob.advertiseMethod("pingUInt", &pingUInt);

  ob.xAdvertiseMethod("i", "pingChar_i", "(i)", AnyFunction::from(&pingChar).dropFirstArgument());
  AnyObject obj = ob.object();
  p.server()->registerService("ping", obj);

  AnyObject client = p.client()->service("ping").value();
  ASSERT_EQ(1, client.call<int>("pingChar", 1));
  try
  {
    // Return cast check
    EXPECTTHROW(client.call<char>("pingInt", 256));
    EXPECTTHROW(client.call<char>("pingInt", 128));
    EXPECTTHROW(client.call<unsigned char>("pingInt", -1));
    EXPECTTHROW(client.call<unsigned int>("pingInt", -1));
    // call arg check
    EXPECTTHROW(client.call<int>("pingUInt", -1));
    EXPECTTHROW(client.call<int>("pingChar", 128));
    EXPECTTHROW(client.call<int>("pingUChar", 256));
    // call arg check at call backend site
    EXPECTTHROW(client.call<int>("pingChar_i", 512));
  }
  catch(const std::exception& e)
  {
    qiLogError() << "Unexpected exception: " << e.what();
    throw e;
  }
}

TEST(TestCall, ForceOverload)
{
  using namespace qi;
  TestSessionPair p;
  DynamicObjectBuilder ob;
  ob.advertiseMethod("pingChar", &pingChar);
  ob.advertiseMethod("pingUChar", &pingUChar);
  ob.advertiseMethod("pingInt", &pingInt);
  ob.advertiseMethod("pingString", &pingString);
  ob.advertiseMethod("ping", &pingInt);
  ob.advertiseMethod("ping", &pingString);
  AnyObject obj = ob.object();
  p.server()->registerService("ping", obj);

  AnyObject client = p.client()->service("ping").value();
  // no .value() on expectthrow: we expect a synchronous exception
  EXPECTTHROW(client.call<std::string>("pingInt", "foo"));
  ASSERT_EQ("foo", client.call<std::string>("ping", "foo"));
  ASSERT_EQ(12, client.call<int>("ping", 12));
  EXPECTTHROW(client.call<std::string>("ping::(s)", 5));
  EXPECTTHROW(client.call<int>("ping::(i)", "foo"));
  ASSERT_EQ(12, client.call<int>("ping::(i)", (char)12));
  ASSERT_EQ("foo", client.call<std::string>("ping::(s)", "foo"));
}

TEST(TestCall, Future)
{
  TestSessionPair p;

  static constexpr auto callCount = 2;
  struct
  {
    std::array<qi::Promise<void>, callCount> syncPromises;
    std::atomic<int> promIndex{ 0 };

    qi::Future<int> operator()(int value)
    {
      const auto idx = promIndex++;
      QI_ASSERT_TRUE(idx < static_cast<int>(syncPromises.size()));
      auto& syncProm = syncPromises[idx];

      qi::Promise<int> result;
      syncProm.future().connect([=](qi::Future<void>) mutable {
        if (value == -1)
          result.setError("-1");
        else
          result.setValue(value);
      });
      return result.future();
    }
  } delaySet;

  qi::DynamicObjectBuilder gob;
  gob.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  gob.advertiseMethod("delaySet", [&](int value){ return delaySet(value); });

  qi::AnyObject sobj = gob.object();
  p.server()->registerService("delayer", sobj);
  qi::AnyObject obj = p.client()->service("delayer").value();

  qi::Future<int> f = obj.async<int>("delaySet", 41);
  qi::Future<int> f2 = obj.async<int>("delaySet", -1);

  ASSERT_TRUE(test::isStillRunning(f, test::willDoNothing(), usualTimeout));
  ASSERT_TRUE(test::isStillRunning(f2, test::willDoNothing(), usualTimeout));

  for (auto& syncProm : delaySet.syncPromises)
    syncProm.setValue(nullptr);

  ASSERT_TRUE(test::finishesWithValue(f));
  ASSERT_EQ(41, f.value());
  ASSERT_TRUE(test::finishesWithError(f2));
}

TEST(TestCall, CallOnFutureReturn)
{
  TestSessionPair p;

  qi::DynamicObjectBuilder gob;
  gob.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  gob.advertiseMethod("futurize", [](int value){ return qi::futurize(value); });

  qi::AnyObject sobj = gob.object();
  p.server()->registerService("futurizer", sobj);
  qi::AnyObject obj = p.client()->service("futurizer").value();
  int f = obj.call<int>("futurize", 41);
  ASSERT_EQ(41, f);
}

TEST(TestCall, TestInvalidFuture)
{
  TestSessionPair p;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("getInvalid",
                     boost::function<qi::Future<void>()>(
                       []{ return qi::Future<void>(); }));
  ob.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  p.server()->registerService("test", ob.object());
  qi::AnyObject proxy = p.client()->service("test").value();

  qi::Future<void> future = proxy.async<void>("getInvalid");
  ASSERT_EQ(qi::detail::InvalidFutureError, future.error());
  try
  {
    proxy.call<void>("getInvalid");
    FAIL();
  }
  catch (std::exception& e)
  {
    ASSERT_EQ(qi::detail::InvalidFutureError, std::string(e.what()));
  }
}

void arrrg(int) {
}

TEST(TestCall, BadArguments)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder gob;
  gob.advertiseMethod("arrrg", &arrrg);
  qi::AnyObject sobj = gob.object();
  p.server()->registerService("a", sobj);
  qi::AnyObject obj = p.client()->service("a").value();
  qi::Future<qi::AnyReference> f = obj.metaCall("arrrg::(i)", qi::GenericFunctionParameters());
  ASSERT_TRUE(test::finishesWithError(f));

  qi::Future<qi::AnyReference> f2 = obj.metaCall("arrrg", qi::GenericFunctionParameters());
  ASSERT_TRUE(test::finishesWithError(f2));
}

TEST(TestCall, MetaCallFutureMatchesMethod)
{
  qi::Promise<void> promise;

  qi::DynamicObjectBuilder dob;
  dob.advertiseMethod("hop", [=]{ promise.future().wait(); });
  qi::AnyObject obj = dob.object();

  TestSessionPair sessions;
  sessions.server()->registerService("hip", obj);
  qi::AnyObject remoteObj = sessions.client()->service("hip").value();

  qi::Future<qi::AnyReference> future = remoteObj.metaCall("hop", qi::GenericFunctionParameters{});
  ASSERT_TRUE(test::isStillRunning(future, test::willDoNothing(), usualTimeout));
  promise.setValue(nullptr);
  ASSERT_TRUE(test::finishesWithValue(future));
}

TEST(TestCall, MetaCallFutureMatchesMethodFuture)
{
  qi::Promise<void> promise;

  qi::DynamicObjectBuilder dob;
  dob.advertiseMethod("hop", [=]{ return promise.future(); });
  qi::AnyObject obj = dob.object();

  TestSessionPair sessions;
  sessions.server()->registerService("hip", obj);
  qi::AnyObject remoteObj = sessions.client()->service("hip").value();

  qi::Future<qi::AnyReference> future = remoteObj.metaCall("hop", qi::GenericFunctionParameters{});
  ASSERT_TRUE(test::isStillRunning(future, test::willDoNothing(), usualTimeout));
  promise.setValue(nullptr);
  ASSERT_TRUE(test::finishesWithValue(future));
}

// TODO: fix races in ObjectStatistics to reenable this test
TEST(TestCall, DISABLED_Statistics)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder gob;
  const auto mid = gob.advertiseMethod("msleep", [](long long ms) {
    boost::this_thread::sleep_for(qi::MilliSeconds(ms));
  });
  qi::AnyObject srv = gob.object();
  p.server()->registerService("sleep", srv);
  qi::AnyObject obj = p.client()->service("sleep").value();
  obj.call<void>("msleep", 10);
  EXPECT_TRUE(obj.stats().empty());
  obj.call<void>("enableStats", true);
  obj.call<void>("msleep", 10);
  obj.call<void>("msleep", 100);
  qi::ObjectStatistics stats = obj.call<qi::ObjectStatistics>("stats");
  EXPECT_EQ(1u, stats.size());
  qi::MethodStatistics m = stats[mid];
  EXPECT_EQ(2u, m.count());
  // Don't expect too much sleep precision on wall time
  EXPECT_GT(0.05, std::abs(m.wall().minValue() - 0.010));
  EXPECT_GT(0.05, std::abs(m.wall().maxValue() - 0.100));
  obj.call<void>("clearStats");
  obj.call<void>("msleep", 0);
  stats = obj.call<qi::ObjectStatistics>("stats");
  m = stats[mid];
  EXPECT_EQ(1u, m.count());
  obj.call<void>("enableStats", false);
  obj.call<void>("clearStats");
  EXPECT_TRUE(!obj.call<bool>("isStatsEnabled"));
  obj.call<void>("msleep", 0);
  stats = obj.call<qi::ObjectStatistics>("stats");
  EXPECT_TRUE(stats.empty());
}

class ArgPack
{
public:
  qi::Property<qi::AnyValue> onCall;
  int callMe(const qi::AnyArguments& pack)
  {
    onCall.set(qi::AnyValue::from(pack.args()));
    return pack.args().size();
  }
};

QI_REGISTER_OBJECT(ArgPack, onCall, callMe);

TEST(TestCall, Dynamic)
{
  TestSessionPair p;
  boost::shared_ptr<ArgPack> ap(new ArgPack);
  qi::AnyObject os = qi::AnyReference::from(ap).to<qi::AnyObject>();
  p.server()->registerService("packer", os);
  qi::AnyObject o = p.client()->service("packer").value();
  qi::detail::printMetaObject(std::cerr, o.metaObject());
  EXPECT_EQ(3, o.call<int>("callMe", 1, 2, 3));
  qi::AnyValue args = o.property<qi::AnyValue>("onCall").value();
  std::vector<int> expect = boost::assign::list_of(1)(2)(3);
  EXPECT_EQ(expect, args.to<std::vector<int> >());
}

class TestOverload
{
public:
  const int& getI() const { return i;}
  int& getI() { return i;}
  int foo(int) {return 0;}
  int foo(std::string) { return 1;}
  int i;
};

TEST(TestAdvertise, Overload)
{
  TestSessionPair p;
  qi::ObjectTypeBuilder<TestOverload> builder;
  builder.advertiseMethod("getI", (int&(TestOverload::*)())&TestOverload::getI);
  builder.advertiseMethod("foo", (int(TestOverload::*)(int))&TestOverload::foo);
  builder.advertiseMethod("foo", (int(TestOverload::*)(std::string))&TestOverload::foo);
  qi::AnyObject o = builder.object(new TestOverload());
  p.server()->registerService("o", o);
  qi::AnyObject c = p.client()->service("o").value();
  EXPECT_EQ(0, c.call<int>("foo", 1));
  EXPECT_EQ(0, c.call<int>("foo", 1.5));
  EXPECT_EQ(1, c.call<int>("foo", "bar"));
  EXPECT_EQ(1, c.call<int>("foo", std::string("bar")));
}

// hand-written specialized proxy on TestCall
class TestClassProxy: public TestClassInterface, public qi::Proxy
{
public:
  TestClassProxy() {}
  TestClassProxy(qi::AnyObject o)
  : qi::Proxy(o) {}
  qi::Future<int> ping(int v) { return asObject().async<int>("ping", v); }
};

QI_REGISTER_PROXY_INTERFACE(TestClassProxy, TestClassInterface);

TEST(TestObjectT, Complete)
{
  TestSessionPair p;
  qi::Object<TestClass> o = qi::Object<TestClass>(new TestClass());
  p.server()->registerService("s", o);
  // Server! This is expected to fail on client in sd mode, TestClass is no proxy
  qi::Object<TestClass> olocal = p.server()->service("s");
  ASSERT_TRUE(!!olocal);
  EXPECT_EQ(12, olocal->ping(12).value());
  EXPECT_EQ(12, (*olocal).ping(12).value());
  EXPECT_EQ(12, olocal.asGenericObject()->call<int>("ping", 12));
  EXPECT_EQ(12, olocal.call<int>("ping", 12));
  // Object<T> way, does not require proxy registration actually

  qi::Object<TestClassInterface> oproxy = p.client()->service("s");
  // Look! It's the same code as above!
  EXPECT_EQ(12, oproxy->ping(12).value());
  EXPECT_EQ(12, (*oproxy).ping(12).value());
  EXPECT_EQ(12, oproxy.asGenericObject()->call<int>("ping", 12));
  EXPECT_EQ(12, oproxy.call<int>("ping", 12));

  // No interface, Object<Empty>
  qi::Object<> gproxy = p.client()->service("s").value();
  EXPECT_EQ(12, gproxy.call<int>("ping", 12));

  // old way for comparison. I don't see anything wrong with that :p
  boost::shared_ptr<TestClassInterface> oldproxy =
    qi::AnyValue(p.client()->service("s").value()).to<boost::shared_ptr<TestClassInterface> >();
  ASSERT_TRUE(!!oldproxy);
  EXPECT_EQ(12, oldproxy->ping(12).value());
  EXPECT_EQ(12, (*oldproxy).ping(12).value());
}

// hard enough to read without it
using namespace qi;

class PassObject
{
public:
  AnyObject pingaa(AnyObject o) { return o;}
  AnyObject pingat(Object<TestClassInterface> o) { return o;}
  Object<TestClassInterface> pingta(AnyObject o) {return o;}
  Object<TestClassInterface> pingtt(Object<TestClassInterface> o) { return o;}

  qi::Property<int> val;
};

QI_REGISTER_OBJECT(PassObject, pingaa, pingat, pingta, pingtt, val);

TEST(TestObjectT, Passing)
{
  TestSessionPair p;

  Object<PassObject> pingerService(new PassObject);
  p.server()->registerService("pinger", pingerService);
  AnyObject pinger = p.client()->service("pinger").value();
  Object<TestClass> tc(new TestClass());
  Object<TestClassInterface> tcprox = tc;
  tcprox = pinger.call<Object<TestClassInterface> >("pingaa", tc);
  EXPECT_EQ(42, tcprox->ping(42).value());
  tcprox = pinger.call<Object<TestClassInterface> >("pingat", tc);
  EXPECT_EQ(42, tcprox->ping(42).value());
  tcprox = pinger.call<Object<TestClassInterface> >("pingta", tc);
  EXPECT_EQ(42, tcprox->ping(42).value());
  tcprox = pinger.call<Object<TestClassInterface> >("pingtt", tc);
  EXPECT_EQ(42, tcprox->ping(42).value());
}

TEST(TestObjectT, Doom)
{
  TestSessionPair p;

  Object<PassObject> pingerService(new PassObject);
  p.server()->registerService("pinger", pingerService);
  AnyObject pinger = p.client()->service("pinger").value();
  Object<TestClass> tc(new TestClass());
  Object<TestClassInterface> tp = tc;
  // MUHAHAHAHAHA
  for (unsigned i=0; i<10; ++i)
  {
    tp = pinger.call<Object<TestClassInterface> >("pingaa", tp);
    ASSERT_NO_THROW(tp.setProperty("prop", 42));
  }
  ASSERT_EQ(42, tp->ping(42).value());
}

TEST(TestObjectT, weak)
{
  if (false)
  {
    // just test that conversions compile
    Object<PassObject> po(new PassObject());
    AnyWeakObject awo(po);
    Object<PassObject> po2(awo.lock());
    AnyObject ao(awo.lock());
    AnyObject ao2(po);
    AnyWeakObject awo2(ao);
  }
}

void inc_atomic_and_delete(TestClass* obj, qi::Atomic<int>* a)
{
  delete obj;
  ++(*a);
}

TEST(TestObject, callAndDropPointer)
{
  // We check that the object is deleted after the call
  int currentDCount = TestClass::destructionCount.load();
  TestSessionPair p;
  Session& s  = *p.server();
  qi::Atomic<int> checker;
  Object<TestClass> svc(new TestClass, boost::bind(&inc_atomic_and_delete, _1, &checker));
  const auto sid = s.registerService("test", svc).value();
  qi::GenericObject* go = svc.asGenericObject();
  svc.reset();
  // the object should be present while the call runs
  EXPECT_TRUE(go->call<bool>("unregisterService", &s, sid, &checker));
  // ... and should be gone by now
  EXPECT_EQ(1, checker.load());
  EXPECT_EQ(currentDCount + 1, TestClass::destructionCount.load());
}

TEST(TestObject, asyncCallAndDropPointer)
{
  // We check that the object is deleted after the call
  int currentDCount = TestClass::destructionCount.load();
  TestSessionPair p;
  Session& s  = *p.server();
  qi::Atomic<int> checker;
  Object<TestClass> svc(new TestClass, boost::bind(&inc_atomic_and_delete, _1, &checker));
  const auto sid = s.registerService("test", svc).value();
  qi::GenericObject* go = svc.asGenericObject();
  svc.reset();
  // the object should be present while the call runs
  qi::Future<bool> f = go->async<bool>("unregisterService", &s, sid, &checker);
  ASSERT_TRUE(test::finishesWithValue(f));
  ASSERT_TRUE(f.value());
  // ... and should be gone eventually
  for (unsigned i=0; i<20 && !checker.load(); ++i)
    std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
  EXPECT_EQ(1, checker.load());
  EXPECT_EQ(currentDCount + 1, TestClass::destructionCount.load());
}


void inc_atomic_and_delete_go(GenericObject* obj, qi::Atomic<int>* a)
{
  delete (qi::DynamicObject*)obj->value;
  ++(*a);
}

class MyDynamicObject: public DynamicObject
{
public:
  ~MyDynamicObject()
  {
    ++TestClass::destructionCount;
  }
};

TEST(TestObject, asyncCallAndDropPointerGeneric)
{
  qi::Atomic<int> checker;

  TestClass to; // this instance doesn't matter realy
  int currentDCount = TestClass::destructionCount.load();

  DynamicObject* dobj = new MyDynamicObject();
  AnyObject svc;
  {
    qi::DynamicObjectBuilder builder(dobj, false);
    builder.advertiseMethod("unregisterService", &to, &TestClass::unregisterService);

    svc = builder.object(
      boost::bind(&inc_atomic_and_delete_go, _1, &checker));
  }
  // We check that the object is deleted after the call
  TestSessionPair p;
  Session& s  = *p.server();

  const auto sid = s.registerService("test", svc).value();
  qi::GenericObject* go = svc.asGenericObject();
  svc.reset();
  EXPECT_EQ(0, checker.load()); // are you there?
  // the object should be present while the call runs
  qi::Future<bool> f = go->async<bool>("unregisterService", &s, sid, &checker);
  ASSERT_TRUE(test::finishesWithValue(f));
  ASSERT_TRUE(f.value());
  // ... and should be gone by now, but maybe asynchronously
  for(unsigned i=0; i<10 && !checker.load(); ++i)
    std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
  EXPECT_EQ(1, checker.load());
  EXPECT_EQ(currentDCount + 1, TestClass::destructionCount.load());
}

bool incrementAtomic(qi::Atomic<int>& a)
{
  ++a;
  return true;
}
TEST(TestObject, EarlyAbort)
{
  qi::Atomic<int> a;
  qi::DynamicObjectBuilder builder;
  builder.advertiseMethod("inc", boost::function<bool(void)>(boost::bind(&incrementAtomic, boost::ref(a))));
  TestSessionPair p;
  AnyObject s = builder.object();
  p.server()->registerService("color", s);
  AnyObject o = p.client()->service("color").value();
  EXPECT_ANY_THROW(o.call<std::string>("inc"));
  EXPECT_EQ(0, a.load());
  a = 0;
  EXPECT_ANY_THROW(o.call<bool>("inc", 42));
  EXPECT_EQ(0, a.load());
}

struct Color
{
  Color():r(0),g(0),b(0) {}
  int r,g,b;
};
// No inheritance, we emulate Color and ColorA being the same struct in
// two different CUs or SOs.
struct ColorA
{
  ColorA():r(0),g(0),b(0),a(1) {}
  int r,g,b,a;
};

// only allow drop of a if it equals 1
bool colorDropHandler(std::map<std::string, ::qi::AnyValue>& /*fields*/,
                      const std::vector<std::tuple<std::string, TypeInterface*>>& missing,
                      const std::map<std::string, ::qi::AnyReference>& dropfields)
{
  try
  {
    if (!missing.empty())
      return false;
    if (dropfields.size() != 1 || dropfields.begin()->first != "a")
      return false;
    return dropfields.begin()->second.toInt() == 0;
  }
  catch (...)
  {
    return false;
  }
}

bool colorFillHandler(std::map<std::string, ::qi::AnyValue>& fields,
                      const std::vector<std::tuple<std::string, TypeInterface*>>& missing,
                      const std::map<std::string, ::qi::AnyReference>& dropfields)
{
  if (!dropfields.empty())
    return false;
  if (missing.size() != 1 || std::get<0>(missing.front()) != "a")
    return false;
  fields["a"] = qi::AnyValue::from(0);
  return true;
}

QI_TYPE_STRUCT_REGISTER(Color, r, g, b);
QI_TYPE_STRUCT_EXTENSION_CONVERT_HANDLERS(ColorA, colorFillHandler, colorDropHandler)
QI_TYPE_STRUCT_REGISTER(ColorA, r, g, b, a);

int getColor(Color& c) { return c.r+c.g+c.b;}
int getColorA(ColorA& c) { return c.r+c.g+c.b + c.a;}
Color setColor(int r, int g, int b) { Color res; res.r = r; res.g = g; res.b = b; return res;}
ColorA setColorA(int r, int g, int b, int a) { ColorA res; res.r = r; res.g = g; res.b = b; res.a = a; return res;}


TEST(TestObject, StructVersioning)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder builder;
  builder.advertiseMethod("getColor", &getColor);
  builder.advertiseMethod("getColorA", &getColorA);
  builder.advertiseMethod("setColor", &setColor);
  builder.advertiseMethod("setColorA", &setColorA);
  AnyObject s = builder.object();
  p.server()->registerService("color", s);
  AnyObject o = p.client()->service("color").value();
  Color c;
  ColorA ca;
  c = o.call<Color>("setColor", 0, 1, 2);
  EXPECT_EQ(c.g, 1);
  EXPECT_NO_THROW(ca = o.call<ColorA>("setColor", 0, 1, 2));
  EXPECT_EQ(ca.g, 1);
  EXPECT_EQ(ca.a, 0);
  ca = o.call<ColorA>("setColorA", 0, 2, 5, 3);
  EXPECT_EQ(ca.g, 2);
  EXPECT_EQ(ca.a, 3);
  EXPECT_NO_THROW(c = o.call<Color>("setColorA", 0, 2, 5, 0)); /*FAILS*/
  EXPECT_EQ(c.g, 2);
  EXPECT_ANY_THROW(o.call<Color>("setColorA", 0, 2, 5, 1));

  c.r=0; c.g = 1; c.b = 2;
  int res;
#define EXPECT_EQ_NT(v, call) \
  EXPECT_NO_THROW(res = call); \
  EXPECT_EQ(v, res)

  EXPECT_EQ_NT(3, o.call<int>("getColor", c));
  EXPECT_EQ_NT(3, o.call<int>("getColorA", c));
  ca.r=0; ca.g = 1; ca.b = 2;
  ca.a = 0;
  EXPECT_EQ_NT(3, o.call<int>("getColor", ca));
  EXPECT_EQ_NT(3, o.call<int>("getColorA", ca));
  ca.a = 2;
  EXPECT_ANY_THROW(o.call<int>("getColor", ca));
  EXPECT_EQ_NT(5, o.call<int>("getColorA", ca));
}

static qi::Atomic<int> onCounter;
void onColor(Color&) { ++onCounter;}
void onColorA(ColorA&) { ++onCounter;}

TEST(TestObject, StructVersioningEvent)
{
  /* Careful with testing, one client is expected to only have one version of the struct
   *, so do not register multiple callbacks with different versions
   * of one struct on the same client
  */
  /* WARNING
  * This test cannot reproduce real-world conditions for two reasons:
  * - We use two differently named structs for the two versions, instead
  * of one same name in two address spaces. This is workarounded by
  * environment variable QI_IGNORE_STRUCT_NAME.
  * - We only have one address space, so synthetized types are never used
  * due to the struct factory. In real-world conditions, when data for
  * an unknown struct version is received, a synthetized TypeTuple is
  * used to represent it, which denies all drop/fill conversion attempts.
  * That is why the qimessaging code attempts conversion on both ends.
  *
  * Effective testing would require the sessions to be in different processes.
  */
  onCounter = 0;
  TestSessionPair p;
  qi::DynamicObjectBuilder builder;
  builder.advertiseSignal<Color>("onColor");
  builder.advertiseSignal<ColorA>("onColorA");
  auto c2 = qi::makeSession();
  c2->connect(p.server()->url());
  AnyObject s = builder.object();
  p.server()->registerService("color", s);
  AnyObject o = p.client()->service("color").value();
  AnyObject o2 = c2->service("color").value();
  o.connect("onColor", &onColor);
  o.connect("onColorA", &onColor);
  o2.connect("onColor", &onColorA);
  o2.connect("onColorA", &onColorA);
  s.connect("onColor", &onColorA);
  s.connect("onColorA", &onColorA);
  qiLogInfo() << "Connect done, signaling...";
  Color c;
  ColorA ca;
  ca.r = ca.g = ca.b = ca.a = 0;
  o.post("onColor", c);
  o.post("onColorA", c);
  o2.post("onColor", ca);
  o2.post("onColorA", ca); /* FAILS, double-remote */
  for (unsigned i=0; i<10 && onCounter.load() != 12; ++i)
    std::this_thread::sleep_for(std::chrono::seconds{ 1 });
  EXPECT_EQ(12, onCounter.load());
}

void doCancel(qi::Promise<void>& p)
{
  qiLogDebug() << "canceling !";
  p.setCanceled();
}

qi::Future<void> getCancelableFuture(qi::Promise<void> promise)
{
  qiLogDebug() << "returning future";
  return promise.future();
}

TEST(TestCall, TestAsyncFutureIsCancelable)
{
  TestSessionPair p;

  qi::DynamicObjectBuilder ob;
  qi::Promise<void> promise(&doCancel);
  ob.advertiseMethod("getCancelableFuture",
                     boost::function<qi::Future<void>()>(
                       boost::bind(&getCancelableFuture, promise)));
  ob.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  p.server()->registerService("test", ob.object());
  qi::AnyObject proxy = p.client()->service("test").value();

  qi::Future<void> future = proxy.async<void>("getCancelableFuture");
  future.cancel();
  future.wait(usualTimeout);
  ASSERT_TRUE(future.isCanceled());
}

class SimpleClass
{
public:
  int print() { return v; }
  int v = 0;
};
QI_REGISTER_MT_OBJECT(SimpleClass, print);

using SimpleClassPtrList = std::vector<qi::Object<SimpleClass>>;
class DummyProp
{
public:
  void setProp()
  {
    boost::shared_ptr<SimpleClass> simpleClass(new SimpleClass);
    qi::Object<SimpleClass> simpleClassObj = qi::AnyReference::from(simpleClass).toObject();

    std::vector<qi::Object<SimpleClass>> propList;
    propList.push_back(simpleClassObj);

    prop.set(propList);
  }

  qi::Property<SimpleClassPtrList> prop;
};
QI_REGISTER_MT_OBJECT(DummyProp, setProp, prop);

static void getAndSetObjProp(const TestSessionPair& p, const std::string& module, const std::string& prop)
{
  qi::SessionPtr sess = qi::makeSession();
  if (p.mode() == TestMode::Mode_Gateway)
    sess->connect(p.gatewayEndpoints()[0]);
  else
    sess->connect(p.serviceDirectoryEndpoints()[0]);

  qi::AnyObject obj = sess->service(module).value();
  auto objProp = obj.property<std::vector<qi::AnyObject>>(prop).value();

  obj.call<void>("setProp");
  sess->close();
}

TEST(TestCall, TestMultipleGetObjectProperty)
{
  TestSessionPair p;

  boost::shared_ptr<DummyProp> dummyProp(new DummyProp);
  qi::Object<DummyProp> dummyPropObj = qi::AnyReference::from(dummyProp).toObject();

  p.server()->registerService("Serv", dummyPropObj);
  qi::AnyObject obj = p.server()->service("Serv").value();
  obj.call<void>("setProp");

  for (int i = 0; i < 10; ++i)
    ASSERT_NO_THROW(getAndSetObjProp(p, "Serv", "prop"));
}

TEST(TestCall, TestIsConnected)
{
  TestSessionPair p0;
  TestSessionPair p1(TestSessionPair::ShareServiceDirectory, p0);
  ASSERT_TRUE(p0.server()->isConnected());
  ASSERT_TRUE(p0.client()->isConnected());
  ASSERT_TRUE(p1.server()->isConnected());
  ASSERT_TRUE(p1.client()->isConnected());
}


class DummyAlMemory
{

  boost::synchronized_value<qi::AnyObject> _subscriber;

public:
  DummyAlMemory() : _subscriber(boost::make_shared<SimpleClass>()) {}

  qi::AnyObject subscriber() {
    return _subscriber.get();
  }
};
QI_REGISTER_OBJECT(DummyAlMemory, subscriber);


TEST(TestCall, MetaObjectCacheFailureWhenCallsInParallel)
{
  TestSessionPair sessions;

  {
    qi::AnyObject service = boost::make_shared<DummyAlMemory>();
    auto server = sessions.server();
    server->registerService("ALMemory", service);
  }

  auto memory = sessions.client()->service("ALMemory").value();

  static const int PARALLEL_TASKS_TO_RUN = 10;

  std::vector<std::thread> threads;
  threads.reserve(PARALLEL_TASKS_TO_RUN);

  for (int i = 0; i < PARALLEL_TASKS_TO_RUN; ++i)
  {
    threads.emplace_back( [memory]{
      try
      {
        memory.call<qi::AnyObject>("subscriber");
      }
      catch (std::exception const &)
      {
        FAIL();
      }

    });
  }

  for(auto&& thread : threads)
    thread.join();

}

struct TypeErasedCancellableAsyncFuture
{
  qi::Promise<void> prom {[](qi::Promise<void>& p){ p.setCanceled(); }};

  qi::Future<void> cancellableAsyncFuture()
  {
    return prom.future();
  }

  qi::AnyReference operator()(const qi::GenericFunctionParameters&)
  {
    return qi::AnyReference::from(cancellableAsyncFuture()).clone();
  }
};

TEST(TestCall, callAdvertisedAnyFunctionReturningAFuture)
{
  TypeErasedCancellableAsyncFuture typeErasedCancellableAsyncFuture;
  auto anyFunction = qi::AnyFunction::fromDynamicFunction(typeErasedCancellableAsyncFuture);
  qi::DynamicObjectBuilder ob;
  ob.xAdvertiseMethod("v", "cancellableAsyncFuture", "()", std::move(anyFunction));
  auto anyObject = ob.object();
  auto future = anyObject.metaCall("cancellableAsyncFuture", qi::GenericFunctionParameters{});
  EXPECT_TRUE(test::isStillRunning(future, test::willDoNothing(), qi::MilliSeconds{100}));
}

namespace
{
class StrandedObjectWithReadOnlyProperty
{
  mutable qi::Strand _strand;
  qi::Property<int> sourceX;

public:
  explicit StrandedObjectWithReadOnlyProperty(int val)
    : sourceX{val, _strand}
    , x{sourceX}
  {}

  ~StrandedObjectWithReadOnlyProperty()
  {
    _strand.join();
  }

  qi::Future<int> add(int o)
  {
    return _strand.async([=] {
      return sourceX.get().async()
        .andThen(
          _strand.unwrappedSchedulerFor([=](int val) { return sourceX.set(val + o).async(); }))
        .unwrap()
        .andThen(_strand.unwrappedSchedulerFor([=](void*) { return sourceX.get().async(); }))
        .unwrap();
    }).unwrap();
  }

  qi::ReadOnlyProperty<int> x;
};
QI_REGISTER_OBJECT(StrandedObjectWithReadOnlyProperty, x, add);

template<typename T>
auto makeFutureErrorFromException()
  -> decltype(ka::compose(qi::makeFutureError<T>, ka::exception_message_t{}))
{
  return ka::compose(qi::makeFutureError<T>, ka::exception_message_t{});
}


class SyncObjectWithReadOnlyProperty
{
  qi::Property<int> sourceX;

public:
  SyncObjectWithReadOnlyProperty(int val)
    : sourceX{val}
    , x{sourceX}
  {
  }

  qi::Future<int> add(int o)
  {
    static std::mutex mut;
    std::lock_guard<std::mutex> lock{ mut };
    return sourceX.get().async()
      .andThen(FutureCallbackType_Sync, [=](int val){ return sourceX.set(val + o).async(); })
      .unwrap()
      .andThen(FutureCallbackType_Sync, [=](void*) { return sourceX.get().async(); })
      .unwrap();
  }

  qi::ReadOnlyProperty<int> x;
};
QI_REGISTER_OBJECT(SyncObjectWithReadOnlyProperty, x, add);

template<typename ObjectType>
class TestCallReadOnlyProperty : public testing::Test
{
public:
  TestCallReadOnlyProperty()
    : objectWithReadOnlyProperty{boost::make_shared<ObjectType>(propertyDefaultValue)}
    , serverObject{objectWithReadOnlyProperty}
  {
    static const auto serviceName = "ReadOnlyPropertyService";

    auto& server = *sessionPair.server();
    auto& client = *sessionPair.client();

    qiLogInfo() << "Registering service '" << serviceName << "' on the server session.";
    servId = server.registerService(serviceName, serverObject).value();

    qiLogInfo() << "Waiting for service '" << serviceName << "' on the client session.";
    EXPECT_EQ(qi::FutureState_FinishedWithValue, client.waitForService(serviceName).wait());

    qiLogInfo() << "Getting service '" << serviceName << "' from the client session.";
    clientObject = client.service(serviceName).value();
  }

  ~TestCallReadOnlyProperty()
  {
    sessionPair.server()->unregisterService(servId).wait();
  }

protected:
  const int propertyDefaultValue = 42;
  TestSessionPair sessionPair;
  boost::shared_ptr<ObjectType> objectWithReadOnlyProperty;
  qi::Object<ObjectType> serverObject;
  qi::AnyObject clientObject;
  unsigned int servId;
};

using ObjectWithReadOnlyPropertyTypes = testing::Types<
  StrandedObjectWithReadOnlyProperty, SyncObjectWithReadOnlyProperty
>;
}

TYPED_TEST_SUITE(TestCallReadOnlyProperty, ObjectWithReadOnlyPropertyTypes);

TYPED_TEST(TestCallReadOnlyProperty, GetProperty)
{
  qi::AnyObject& clientObj = this->clientObject;

  qiLogInfo() << "Getting the property from the client service object.";
  const auto fut = clientObj.property<int>("x").async();
  ASSERT_EQ(qi::FutureState_FinishedWithValue, fut.wait());
  ASSERT_EQ(this->propertyDefaultValue, fut.value());
}

TYPED_TEST(TestCallReadOnlyProperty, SetProperty)
{
  qi::AnyObject& clientObj = this->clientObject;

  qiLogInfo() << "Setting the property from the client service object.";
  const auto fut = clientObj.setProperty<int>("x", 231).async();
  ASSERT_EQ(qi::FutureState_FinishedWithError, fut.wait());
}

TYPED_TEST(TestCallReadOnlyProperty, CallAdd)
{
  qi::AnyObject& clientObj = this->clientObject;
  const auto addedValue = 12;

  qiLogInfo() << "Calling the method on the client service object.";
  const auto fut = clientObj.async<int>("add", addedValue);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, fut.wait());
  ASSERT_EQ(this->propertyDefaultValue + addedValue, fut.value());
}
