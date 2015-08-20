/*
 *  Author(s):
 *  - Pierre Roullon <proullon@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <list>

#include <gtest/gtest.h>

#include <boost/assign/list_of.hpp>

#include <qi/application.hpp>
#include <qi/eventloop.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/session.hpp>
#include <testsession/testsessionpair.hpp>
#include "src/messaging/message.hpp"

qiLogCategory("test");

//TEST(TestMessage, COW)
//{
//  qi::Message m1;
//  const void* ptr = &m1.signature();
//  m1.setSignature("s");
//  ASSERT_EQ(ptr, &m1.signature());
//  qi::Message m2(m1);
//  ASSERT_EQ(ptr, &m1.signature());
//  ASSERT_EQ(ptr, &m2.signature());
//  m2.setService(1);
//  ASSERT_TRUE(&m1.signature() != &m2.signature());
//}

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

int fakeRGBf(const std::string &name, int value, float duration)
{
  return (0);
}

int fakeRGBd(const std::string &name, int value, double duration)
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
QI_TYPE_STRUCT(GenericTuple, e1, e2, e3);

struct SpecificTuple
{
  int e1;
  double e2;
  std::map<std::string, float> e3;
};
QI_TYPE_STRUCT(SpecificTuple, e1, e2, e3);

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
  qi::AnyObject proxy = p.client()->service("test");
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
  int serviceID;

  ob.advertiseMethod("print", &print);
  qi::AnyObject obj(ob.object());

  serviceID = p.server()->registerService("serviceCall", obj);

  qi::AnyObject proxy = p.client()->service("serviceCall");

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
  int serviceID;

  ob.advertiseMethod("foobar", &foobar);
  qi::AnyObject obj(ob.object());

  serviceID = p.server()->registerService("serviceCall", obj);

  qi::AnyObject proxy = p.client()->service("serviceCall");


  std::cout << "Calling" << std::endl;
  qi::Future<void> fut = proxy.async<void>("foobar");

  ASSERT_FALSE(fut.hasError());
  p.server()->unregisterService(serviceID);
}

TEST(TestCall, CallVoidErr)
{
  std::list<std::pair<std::string, int> >  robots;
  TestSessionPair          p;
  qi::DynamicObjectBuilder ob;
  int serviceID;

  ob.advertiseMethod("fooerr", &fooerr);
  qi::AnyObject obj(ob.object());

  serviceID = p.server()->registerService("serviceCall", obj);

  qi::AnyObject proxy = p.client()->service("serviceCall");

  std::cout << "Calling" << std::endl;
  qi::Future<void> fut = proxy.async<void>("fooerr");

  ASSERT_TRUE(fut.hasError());
  p.server()->unregisterService(serviceID);
}

TEST(TestCall, TestDoubleToFloatConvertion)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder ob;
  int serviceID;
  double duration = 0.42;

  ob.advertiseMethod("fakeRGB", &fakeRGBf);
  qi::AnyObject obj(ob.object());

  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);

  std::cout << "Calling FakeRGB" << std::endl;
  qi::Future<int> fut = proxy.async<int>("fakeRGB", "Haha", 42, duration);
}

TEST(TestCall, TestFloatToDoubleConvertion)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder ob;
  int serviceID;
  float duration = 0.42f;

  ob.advertiseMethod("fakeRGB", &fakeRGBd);
  qi::AnyObject obj(ob.object());

  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);

  std::cout << "Calling FakeRGB" << std::endl;
  qi::Future<int> fut = proxy.async<int>("fakeRGB", "Haha", 42, duration);
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
  int serviceID;
  qi::AnyObject obj = createObject();
  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);

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
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakeemptygvec", egvec);
  EXPECT_FALSE(fut.hasError());

  //check call, same type
  fut = proxy.async<int>("fakesvec", svec);
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakegvec", gvec);
  EXPECT_FALSE(fut.hasError());

  //check empty, type conv
  fut = proxy.async<int>("fakeemptysvec", egvec);
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakeemptygvec", esvec);
  EXPECT_FALSE(fut.hasError());

  //check call, type conv
  fut = proxy.async<int>("fakesvec", gvec);
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakegvec", svec);
  EXPECT_FALSE(fut.hasError());
}

TEST(TestCall, TestGenericConversionComplexList) {
  TestSessionPair p;
  int serviceID;
  qi::AnyObject obj = createObject();
  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);


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
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakesvvec", ssg);
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakesvvec", sg);
  EXPECT_FALSE(fut.hasError());

  fut = proxy.async<int>("fakegvvec", sss);
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakegvvec", ssg);
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakegvvec", sg);
  EXPECT_FALSE(fut.hasError());


  fut = proxy.async<int>("fakegvvec2", sss);
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakegvvec2", ssg);
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakegvvec2", sg);
  EXPECT_FALSE(fut.hasError());
}

TEST(TestCall, TestGenericConversionComplexMap) {
  TestSessionPair p;
  int serviceID;
  qi::AnyObject obj = createObject();
  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);


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
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakemsvvec", msvvg);
  EXPECT_FALSE(fut.hasError());

  fut = proxy.async<int>("fakemgvvec", msvvs);
  EXPECT_FALSE(fut.hasError());
  fut = proxy.async<int>("fakemgvvec", msvvg);
  EXPECT_FALSE(fut.hasError());
}


TEST(TestCall, TestGenericConversionTuple) {
  TestSessionPair p;
  int serviceID;
  qi::AnyObject obj = createObject();
  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::AnyObject proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);

  qi::Future<double> f;
  SpecificTuple t;
  t.e1 = 1;
  t.e2 = 2;
  t.e3["foo"] = 3;
  f = proxy.async<double>("eatSpecific", t);
  EXPECT_FALSE(f.hasError());
  EXPECT_EQ(6, f.value());

  GenericTuple gt;
  gt.e1 = qi::AnyReference::from(1.0);
  gt.e2 = qi::AnyReference::from(2U);
  std::map<std::string, qi::AnyValue> map;
  map["foo"] = qi::AnyReference::from(3);
  gt.e3 = qi::AnyReference::from(map);
  f = proxy.async<double>("eatSpecific", gt);
  EXPECT_FALSE(f.hasError());
  EXPECT_EQ(6, f.value());

  std::map<unsigned int, std::string> ravMap;
  gt.e3 = qi::AnyReference::from(ravMap);
  f = proxy.async<double>("eatSpecific", gt);
  EXPECT_FALSE(f.hasError());
  EXPECT_EQ(3, f.value());
}


void set_true(bool* b)
{
  *b = true;
}

int service_call(qi::SessionPtr s, const std::string& obj,
  const std::string& method, int arg)
{
  qiLogDebug() << "TEST: servicecall";
  qi::AnyObject o = s->service(obj);
  return o.call<int>(method, arg);
}

void servicecall_addone(qi::Promise<int>& prom, qi::SessionPtr s)
{
  qiLogDebug() << "TEST: call servicecall";
  qi::AnyObject obj2Proxy = s->service("caller");
  qiLogDebug() << "TEST: got service";
  qi::Future<int> v = obj2Proxy.async<int>("serviceCall", "adder", "addOne", 5);
  v.wait(500);
  if (!v.isFinished())
    prom.setError("timeout");
  else if (v.hasError())
    prom.setError(v.error());
  else
    prom.setValue(v.value());
}

TEST(TestCall, PairClientListen)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("addOne", &addOne);
  qi::AnyObject obj(ob.object());
  p.client()->registerService("adder", obj);
  qi::AnyObject o = p.server()->service("adder");
  ASSERT_TRUE(o);
}

TEST(TestCall, DeadLock)
{
  // This test deeadlocks if all objects are in the same monothreaded event loop
  qi::EventLoop* ev = new qi::EventLoop();
  ev->start();
  // One object calls another, both in singleThread mode
  TestSessionPair p;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("addOne", &addOne);
  qi::AnyObject obj(ob.object());
  p.server()->registerService("adder", obj);

  qi::DynamicObjectBuilder ob2;
  ob2.advertiseMethod("serviceCall",
    (boost::function<int(std::string, std::string, int)>)
    boost::bind(&service_call, p.client(), _1, _2, _3));
  qi::AnyObject obj2(ob2.object());
  p.client()->registerService("caller", obj2);

  qi::Promise<int> prom;

  // From the object event loop of process 'server', call a method from
  // object in client, which will call back a method in server
  qiLogDebug() << "TEST: go async servicecall_addone";
  qi::getEventLoop()->async(
    boost::bind(&servicecall_addone, boost::ref(prom), p.server()));

  for (unsigned i=0; i<20 && !prom.future().isFinished(); ++i)
    qi::os::msleep(50);
  ASSERT_TRUE(prom.future().isFinished());
  ASSERT_EQ(6, prom.future().value());
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

void bounceFuture(qi::Future<int> s, qi::Promise<int> d, qi::AnyObject obj)
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

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("makeObjectCall", &makeObjectCall);
  ob.advertiseMethod("bindObjectEvent", boost::function<void(qi::AnyObject, const std::string&)>(boost::bind(&bindObjectEvent, _1, _2, boost::ref(eventValue))));
  qi::AnyObject obj(ob.object());
  p.server()->registerService("s", obj);
  qi::AnyObject proxy = p.client()->service("s");

  qi::AnyObject unregisteredObj;
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseMethod("add", &addOne);
    ob.advertiseSignal<int>("fire");
    unregisteredObj = ob.object();
  }
  // Transmit unregisteredObj through the network.
  qi::Future<int> v = proxy.async<int>("makeObjectCall", unregisteredObj, "add", 1);
  v.wait(2000);
  ASSERT_TRUE(!v.hasError());
  ASSERT_EQ(2, v.value());
  proxy.call<void>("bindObjectEvent", unregisteredObj, "fire");
  unregisteredObj.post("fire", 42);
  eventValue.future().wait(); //fixme wait(2s)
  ASSERT_TRUE(eventValue.future().isFinished());
  ASSERT_EQ(42, eventValue.future().value());
  eventValue = qi::Promise<int>();

  // Check that object is locked by remote end
  qi::AnyWeakObject unregisteredWeakObj = unregisteredObj;
  unregisteredObj.reset();
  unregisteredObj = unregisteredWeakObj.lock();
  ASSERT_TRUE(unregisteredObj);
  if (p.client() == p.server())
    return; // test makes no sense in direct mode
  // This will delete the proxy
  unregisteredObj.post("fire", 0);
  eventValue.future().wait();
  ASSERT_TRUE(eventValue.future().isFinished());
  ASSERT_EQ(0, eventValue.future().value());
  eventValue = qi::Promise<int>();
  ASSERT_TRUE(!eventValue.future().isFinished());
  unregisteredObj.post("fire", 1);
  eventValue.future().wait(2000);
  ASSERT_TRUE(!eventValue.future().isFinished());

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
  qi::AnyObject proxy = p.client()->service("s");

  qi::AnyObject unregisteredObj;
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseMethod("add", &addOne);
    ob.advertiseSignal<int>("fire");
    unregisteredObj = ob.object();
  }

  qi::Promise<int> value;
  // We connect a method client-side
  proxy.connect("makeObjectCallEvent", boost::bind(&onMakeObjectCall, _1, _2, _3, value)).wait();
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
  value.future().wait(100000);
  if (value.future().hasError(qi::FutureTimeout_None))
    std::cerr << "Err:" << value.future().error() << std::endl;
  ASSERT_TRUE(value.future().isFinished());
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
  qi::AnyObject proxy = p.client()->service("s");

  qi::AnyObject adder = proxy.call<qi::AnyObject>("makeAdder");
  ASSERT_TRUE(weak.lock());
  // One is client side, the other server side
  if (p.client() != p.server())
    ASSERT_FALSE(weak.lock().asGenericObject() == adder.asGenericObject());
  ASSERT_TRUE(adder);
  qi::Future<int> f = adder.async<int>("add", 41);
  f.wait(1000);
  ASSERT_TRUE(f.isFinished());
  ASSERT_EQ(42, f.value());
  adder.reset();
  qi::os::msleep(300);
  ASSERT_FALSE(weak.lock());
}

class TestClass
{
public:
  TestClass() : v(0) {}
  TestClass(int v) : v(v) {}
  ~TestClass()
  {
    qiLogDebug() << "~TestClass " << this;
    ++destructionCount;
  }
  int ping(int w)
  {
    qiLogDebug() << "TestClass::ping " << this << ' ' << v << ' ' << w;
    return v+w;
  }
  static boost::shared_ptr<TestClass> make(int v)
  {
    TestClass* tc = new TestClass(v);
    qiLogDebug() << "Making a TestClass " << v << ' ' << tc;
    return boost::shared_ptr<TestClass>(tc);
  }
  bool unregisterService(qi::Session* session, int sid, qi::Atomic<int>* checker)
  {
    session->unregisterService(sid).wait();
    qi::os::msleep(100);
    return !(**checker);
  }
  int v;
  qi::Property<int> prop;
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
  qi::AnyObject proxy = p.client()->service("getter");
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

  AnyObject client = p.client()->service("ping");
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

  AnyObject client = p.client()->service("ping");
  // no .value() on expectthrow: we expect a synchronous exception
  EXPECTTHROW(client.call<std::string>("pingInt", "foo"));
  ASSERT_EQ("foo", client.call<std::string>("ping", "foo"));
  ASSERT_EQ(12, client.call<int>("ping", 12));
  EXPECTTHROW(client.call<std::string>("ping::(s)", 5));
  EXPECTTHROW(client.call<int>("ping::(i)", "foo"));
  ASSERT_EQ(12, client.call<int>("ping::(i)", (char)12));
  ASSERT_EQ("foo", client.call<std::string>("ping::(s)", "foo"));
}

void _delaySet(qi::Promise<int> p, unsigned long msDelay, int value)
{
  qi::os::msleep(msDelay);
  if (value == -1)
    p.setError("-1");
  else
    p.setValue(value);
}

qi::Future<int> delaySet(unsigned long msDelay, int value)
{
  qi::Promise<int> p;
  boost::thread(_delaySet, p, msDelay, value);
  return p.future();
}

TEST(TestCall, Future)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder gob;
  gob.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  gob.advertiseMethod("delaySet", &delaySet);
  qi::AnyObject sobj = gob.object();
  p.server()->registerService("delayer", sobj);
  qi::AnyObject obj = p.client()->service("delayer");
  qi::Future<int> f = obj.async<int>("delaySet", 500, 41);
  qi::Future<int> f2 =  obj.async<int>("delaySet", 500, -1);
  ASSERT_TRUE(!f.isFinished());
  ASSERT_TRUE(!f2.isFinished());
  f.wait();
  ASSERT_EQ(41, f.value());
  f2.wait();
  ASSERT_TRUE(f2.hasError());
}

TEST(TestCall, CallOnFutureReturn)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder gob;
  gob.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  gob.advertiseMethod("delaySet", &delaySet);
  qi::AnyObject sobj = gob.object();
  p.server()->registerService("delayer", sobj);
  qi::AnyObject obj = p.client()->service("delayer");
  int f = obj.call<int>("delaySet", 500, 41);
  ASSERT_EQ(41, f);
}

void arrrg(int v) {
}

TEST(TestCall, BadArguments)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder gob;
  gob.advertiseMethod("arrrg", &arrrg);
  qi::AnyObject sobj = gob.object();
  p.server()->registerService("a", sobj);
  qi::AnyObject obj = p.client()->service("a");
  qi::Future<qi::AnyReference> f = obj.metaCall("arrrg::(i)", qi::GenericFunctionParameters());
  EXPECT_TRUE(f.hasError(1000));

  qi::Future<qi::AnyReference> f2 = obj.metaCall("arrrg", qi::GenericFunctionParameters());
  EXPECT_TRUE(f2.hasError(1000));
}

TEST(TestCall, Statistics)
{
  TestSessionPair p;
  qi::DynamicObjectBuilder gob;
  int mid = gob.advertiseMethod("sleep", &qi::os::msleep);
  qi::AnyObject srv = gob.object();
  p.server()->registerService("sleep", srv);
  qi::AnyObject obj = p.client()->service("sleep");
  obj.call<void>("sleep", 10);
  EXPECT_TRUE(obj.stats().empty());
  obj.call<void>("enableStats", true);
  obj.call<void>("sleep", 10);
  obj.call<void>("sleep", 100);
  qi::ObjectStatistics stats = obj.call<qi::ObjectStatistics>("stats");
  EXPECT_EQ(1u, stats.size());
  qi::MethodStatistics m = stats[mid];
  EXPECT_EQ(2u, m.count());
  // Don't expect too much sleep precision on wall time
  EXPECT_GT(0.05, std::abs(m.wall().minValue() - 0.010));
  EXPECT_GT(0.05, std::abs(m.wall().maxValue() - 0.100));
  obj.call<void>("clearStats");
  obj.call<void>("sleep", 0);
  stats = obj.call<qi::ObjectStatistics>("stats");
  m = stats[mid];
  EXPECT_EQ(1u, m.count());
  obj.call<void>("enableStats", false);
  obj.call<void>("clearStats");
  EXPECT_TRUE(!obj.call<bool>("isStatsEnabled"));
  obj.call<void>("sleep", 0);
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
  qi::AnyObject o = p.client()->service("packer");
  qi::detail::printMetaObject(std::cerr, o.metaObject());
  EXPECT_EQ(3, o.call<int>("callMe", 1, 2, 3));
  qi::AnyValue args = o.property<qi::AnyValue>("onCall");
  std::vector<int> expect = boost::assign::list_of(1)(2)(3);
  EXPECT_EQ(expect, args.to<std::vector<int> >());
}

class TestOverload
{
public:
  const int& getI() const { return i;}
  int& getI() { return i;}
  int foo(int i) {return 0;}
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
  qi::AnyObject c = p.client()->service("o");
  EXPECT_EQ(0, c.call<int>("foo", 1));
  EXPECT_EQ(0, c.call<int>("foo", 1.5));
  EXPECT_EQ(1, c.call<int>("foo", "bar"));
  EXPECT_EQ(1, c.call<int>("foo", std::string("bar")));
}

// hand-written specialized proxy on TestClall
class TestClassProxy: public qi::Proxy
{
public:
  TestClassProxy() {}
  TestClassProxy(qi::AnyObject o)
  : qi::Proxy(o) {}
  qi::Future<int> ping(int v) { return asObject().async<int>("ping", v); }
};

QI_REGISTER_PROXY_INTERFACE(TestClassProxy, TestClass);

TEST(TestObjectT, Complete)
{
  TestSessionPair p;
  qi::Object<TestClass> o = qi::Object<TestClass>(new TestClass());
  p.server()->registerService("s", o);
  // Server! This is expected to fail on client in sd mode, TestClass is no proxy
  qi::Object<TestClass> olocal = p.server()->service("s");
  ASSERT_TRUE(!!olocal);
  EXPECT_EQ(12, olocal->ping(12));
  EXPECT_EQ(12, (*olocal).ping(12));
  EXPECT_EQ(12, olocal.asGenericObject()->call<int>("ping", 12));
  EXPECT_EQ(12, olocal.call<int>("ping", 12));
  qi::registerProxy<TestClassProxy>();
  // Object<T> way, does not require proxy registration actually

  qi::Object<TestClassProxy> oproxy = p.client()->service("s");
  // Look! It's the same code as above!
  EXPECT_EQ(12, oproxy->ping(12));
  EXPECT_EQ(12, (*oproxy).ping(12));
  EXPECT_EQ(12, oproxy.asGenericObject()->call<int>("ping", 12));
  EXPECT_EQ(12, oproxy.call<int>("ping", 12));

  // No interface, Object<Empty>
  qi::Object<> gproxy = p.client()->service("s");
  EXPECT_EQ(12, gproxy.call<int>("ping", 12));

  // old way for comparison. I don't see anything wrong with that :p
  boost::shared_ptr<TestClassProxy> oldproxy =
    qi::AnyValue(p.client()->service("s").value()).to<boost::shared_ptr<TestClassProxy> >();
  ASSERT_TRUE(!!oldproxy);
  EXPECT_EQ(12, oldproxy->ping(12));
  EXPECT_EQ(12, (*oldproxy).ping(12));
  EXPECT_EQ(12, oldproxy->asObject().call<int>("ping", 12));
}

// hard enough to read without it
using namespace qi;

class PassObject
{
public:
  AnyObject pingaa(AnyObject o) { return o;}
  AnyObject pingat(Object<TestClassProxy> o) { return o;}
  Object<TestClassProxy> pingta(AnyObject o) {return o;}
  Object<TestClassProxy> pingtt(Object<TestClassProxy> o) { return o;}

  qi::Property<int> val;
};

QI_REGISTER_OBJECT(PassObject, pingaa, pingat, pingta, pingtt, val);

TEST(TestObjectT, Passing)
{
  TestSessionPair p;
  Object<PassObject> pingerService(new PassObject);
  p.server()->registerService("pinger", pingerService);
  AnyObject pinger = p.client()->service("pinger");
  Object<TestClass> tc(new TestClass());
  Object<TestClassProxy> tcprox = tc;
  tcprox = pinger.call<Object<TestClassProxy> >("pingaa", tc);
  EXPECT_EQ(42, tcprox->ping(42));
  tcprox = pinger.call<Object<TestClassProxy> >("pingat", tc);
  EXPECT_EQ(42, tcprox->ping(42));
  tcprox = pinger.call<Object<TestClassProxy> >("pingta", tc);
  EXPECT_EQ(42, tcprox->ping(42));
  tcprox = pinger.call<Object<TestClassProxy> >("pingtt", tc);
  EXPECT_EQ(42, tcprox->ping(42));
}

TEST(TestObjectT, Doom)
{
  TestSessionPair p;
  Object<PassObject> pingerService(new PassObject);
  p.server()->registerService("pinger", pingerService);
  AnyObject pinger = p.client()->service("pinger");
  Object<TestClass> tc(new TestClass());
  Object<TestClassProxy> tp = tc;
  // MUHAHAHAHAHA
  for (unsigned i=0; i<10; ++i)
  {
    tp = pinger.call<Object<TestClassProxy> >("pingaa", tp);
    ASSERT_NO_THROW(tp.setProperty("prop", 42));
  }
  ASSERT_EQ(42, tp->ping(42));
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
  int currentDCount = *TestClass::destructionCount;
  TestSessionPair p;
  Session& s  = *p.server();
  qi::Atomic<int> checker;
  Object<TestClass> svc(new TestClass, boost::bind(&inc_atomic_and_delete, _1, &checker));
  int sid = s.registerService("test", svc);
  qi::GenericObject* go = svc.asGenericObject();
  svc.reset();
  // the object should be present while the call runs
  EXPECT_TRUE(go->call<bool>("unregisterService", &s, sid, &checker));
  // ... and should be gone by now
  EXPECT_EQ(1, *checker);
  EXPECT_EQ(currentDCount + 1, *TestClass::destructionCount);
}

TEST(TestObject, asyncCallAndDropPointer)
{
  // We check that the object is deleted after the call
  int currentDCount = *TestClass::destructionCount;
  TestSessionPair p;
  Session& s  = *p.server();
  qi::Atomic<int> checker;
  Object<TestClass> svc(new TestClass, boost::bind(&inc_atomic_and_delete, _1, &checker));
  int sid = s.registerService("test", svc);
  qi::GenericObject* go = svc.asGenericObject();
  svc.reset();
  // the object should be present while the call runs
  qi::Future<bool> f = go->async<bool>("unregisterService", &s, sid, &checker);
  f.wait();
  EXPECT_TRUE(f.value());
  // ... and should be gone eventually
  for (unsigned i=0; i<20 && !*checker; ++i)
    qi::os::msleep(50);
  EXPECT_EQ(1, *checker);
  EXPECT_EQ(currentDCount + 1, *TestClass::destructionCount);
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
  int currentDCount = *TestClass::destructionCount;

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

  int sid = s.registerService("test", svc);
  qi::GenericObject* go = svc.asGenericObject();
  svc.reset();
  EXPECT_EQ(0, *checker); // are you there?
  // the object should be present while the call runs
  qi::Future<bool> f = go->async<bool>("unregisterService", &s, sid, &checker);
  f.wait();
  EXPECT_TRUE(f.value());
  // ... and should be gone by now, but maybe asynchronously
  for(unsigned i=0; i<10 && !*checker; ++i)
    qi::os::msleep(100);
  EXPECT_EQ(1, *checker);
  EXPECT_EQ(currentDCount + 1, *TestClass::destructionCount);
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
  AnyObject o = p.client()->service("color");
  EXPECT_ANY_THROW(o.call<std::string>("inc"));
  EXPECT_EQ(0, *a);
  a = 0;
  EXPECT_ANY_THROW(o.call<bool>("inc", 42));
  EXPECT_EQ(0, *a);
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

// only allow drop of a if it equals 0 (the default-constructed value)
bool colorVersionHandler(ColorA* instance, const std::vector<std::string>& fields)
{
  qiLogDebug() << "colorVersionHandler " << instance->a;
  if (fields.size() != 1 || fields.front() != "a")
    return false;
  return instance->a == 0;
}

QI_TYPE_STRUCT_REGISTER(Color, r, g, b);
QI_TYPE_STRUCT_EXTENSION_DROP_HANDLER(ColorA, colorVersionHandler);
QI_TYPE_STRUCT_EXTENSION_FILL_FIELDS(ColorA, "a");
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
  AnyObject o = p.client()->service("color");
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

qi::Atomic<int> onCounter;
void onColor(Color& c) { ++onCounter;}
void onColorA(ColorA& c) { ++onCounter;}

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
  qi::Session c2;
  c2.connect(p.server()->url());
  AnyObject s = builder.object();
  p.server()->registerService("color", s);
  AnyObject o = p.client()->service("color");
  AnyObject o2 = c2.service("color");
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
  for (unsigned i=0; i<10 && *onCounter != 12; ++i) qi::os::msleep(100);
  EXPECT_EQ(12, *onCounter);
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
  qi::AnyObject proxy = p.client()->service("test");

  ASSERT_TRUE(promise.future().isCancelable());

  qi::Future<void> future = proxy.async<void>("getCancelableFuture");
  ASSERT_TRUE(future.isCancelable());
  future.cancel();
  future.wait();
  ASSERT_TRUE(future.isCanceled());
}

int main(int argc, char **argv) {
  qi::os::setenv("QI_IGNORE_STRUCT_NAME", "1");
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
