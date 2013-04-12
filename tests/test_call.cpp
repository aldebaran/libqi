/*
 *  Author(s):
 *  - Pierre Roullon <proullon@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <list>
#include <iostream>

#include <gtest/gtest.h>

#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qi/eventloop.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <testsession/testsessionpair.hpp>
#include "src/message.hpp"

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

int fakeemptygvec(const std::vector<qi::GenericValue> &sval) {
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

int fakegvec(const std::vector<qi::GenericValue> &sval) {
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

int fakegvvec(const std::vector< std::vector<qi::GenericValue> > &sval) {
  std::cout << "sval.size(): " << sval.size() << std::endl;
  EXPECT_EQ("titi", sval[0][0].toString());
  EXPECT_EQ("toto", sval[1][0].toString());
  return 0;
}

int fakegvvec2(const std::vector< qi::GenericValue > &sval) {
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

int fakegmvvec(std::map<std::string, std::vector< std::vector<qi::GenericValue> > > &sval) {
  std::cout << "sval.size(): " << sval.size() << std::endl;
  EXPECT_EQ("titi", sval["i"][0][0].toString());
  EXPECT_EQ("toto", sval["j"][0][0].toString());
  return 0;
}

struct GenericTuple
{
  qi::GenericValue e1;
  qi::GenericValue e2;
  qi::GenericValue e3;
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

qi::GenericValue pingCopy(qi::GenericValue arg, qi::GenericValue& target)
{
  target = arg;
  return arg;
}

TEST(TestCall, CallBufferInList)
{
  TestSessionPair          p;
  qi::GenericObjectBuilder ob;
  qi::GenericValue val;
  ob.advertiseMethod("pingcopy",
    boost::function<qi::GenericValue(qi::GenericValue)>(boost::bind(&pingCopy, _1, boost::ref(val))));
  qi::ObjectPtr obj(ob.object());
  p.server()->registerService("test", obj);
  qi::ObjectPtr proxy = p.client()->service("test");
  std::vector<qi::GenericValue> args;
  args.push_back(qi::GenericValue::from(12));
  qi::Buffer buf;
  buf.write("canard", strlen("canard")+1);
  args.push_back(qi::GenericValue::from(buf));
  args.push_back(qi::GenericValue::from("foo"));
  qi::GenericValue result = proxy->call<qi::GenericValue>("pingcopy", args);
  {
    ASSERT_EQ(3u, val.size());
    ASSERT_EQ(12, val[0].toInt());
    qi::Buffer bufRes = val[1].to<qi::Buffer>();
    ASSERT_EQ(strlen("canard")+1, bufRes.size());
    ASSERT_EQ("foo", val[2].toString());
  }
  {
    std::vector<qi::GenericValue> l = result.to<std::vector<qi::GenericValue> >();
    ASSERT_EQ(3u, l.size());
    std::vector<qi::GenericValue>::iterator it = l.begin();
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
  qi::GenericObjectBuilder ob;
  int serviceID;

  ob.advertiseMethod("print", &print);
  qi::ObjectPtr obj(ob.object());

  serviceID = p.server()->registerService("serviceCall", obj);

  qi::ObjectPtr proxy = p.client()->service("serviceCall");

  robots.push_back(std::make_pair("Gibouna", 1234567));
  robots.push_back(std::make_pair("Wall-E", 2345678));
  robots.push_back(std::make_pair("Billy West", 3456789));
  robots.push_back(std::make_pair("33CL", 4567890));

  ASSERT_EQ(4u, proxy->call<long unsigned int>("print", robots));

  p.server()->unregisterService(serviceID);
}


TEST(TestCall, CallVoid)
{
  std::list<std::pair<std::string, int> >  robots;
  TestSessionPair          p;
  qi::GenericObjectBuilder ob;
  int serviceID;

  ob.advertiseMethod("foobar", &foobar);
  qi::ObjectPtr obj(ob.object());

  serviceID = p.server()->registerService("serviceCall", obj);

  qi::ObjectPtr proxy = p.client()->service("serviceCall");


  std::cout << "Calling" << std::endl;
  qi::Future<void> fut = proxy->call<void>("foobar");

  ASSERT_FALSE(fut.hasError());
  p.server()->unregisterService(serviceID);
}

TEST(TestCall, CallVoidErr)
{
  std::list<std::pair<std::string, int> >  robots;
  TestSessionPair          p;
  qi::GenericObjectBuilder ob;
  int serviceID;

  ob.advertiseMethod("fooerr", &fooerr);
  qi::ObjectPtr obj(ob.object());

  serviceID = p.server()->registerService("serviceCall", obj);

  qi::ObjectPtr proxy = p.client()->service("serviceCall");

  std::cout << "Calling" << std::endl;
  qi::Future<void> fut = proxy->call<void>("fooerr");

  ASSERT_TRUE(fut.hasError());
  p.server()->unregisterService(serviceID);
}

TEST(TestCall, TestDoubleToFloatConvertion)
{
  TestSessionPair p;
  qi::GenericObjectBuilder ob;
  int serviceID;
  double duration = 0.42;

  ob.advertiseMethod("fakeRGB", &fakeRGBf);
  qi::ObjectPtr obj(ob.object());

  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::ObjectPtr proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);

  std::cout << "Calling FakeRGB" << std::endl;
  qi::Future<int> fut = proxy->call<int>("fakeRGB", "Haha", 42, duration);
}

TEST(TestCall, TestFloatToDoubleConvertion)
{
  TestSessionPair p;
  qi::GenericObjectBuilder ob;
  int serviceID;
  float duration = 0.42f;

  ob.advertiseMethod("fakeRGB", &fakeRGBd);
  qi::ObjectPtr obj(ob.object());

  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::ObjectPtr proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);

  std::cout << "Calling FakeRGB" << std::endl;
  qi::Future<int> fut = proxy->call<int>("fakeRGB", "Haha", 42, duration);
}

qi::ObjectPtr createObject() {
  qi::GenericObjectBuilder ob;

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
  qi::ObjectPtr obj = createObject();
  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::ObjectPtr proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);

  std::vector<std::string>      svec;
  std::vector<qi::GenericValue> gvec;

  std::vector<std::string>      esvec;
  std::vector<qi::GenericValue> egvec;

  svec.push_back("titi");
  svec.push_back("toto");

  qi::GenericValue gv;
  gv = qi::GenericValueRef(std::string("titi"));
  gvec.push_back(gv);
  gv = qi::GenericValueRef(std::string("toto"));
  gvec.push_back(gv);

  qi::Future<int> fut;

  //Check empty, same type
  fut = proxy->call<int>("fakeemptysvec", esvec);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakeemptygvec", egvec);
  EXPECT_FALSE(fut.hasError());

  //check call, same type
  fut = proxy->call<int>("fakesvec", svec);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakegvec", gvec);
  EXPECT_FALSE(fut.hasError());

  //check empty, type conv
  fut = proxy->call<int>("fakeemptysvec", egvec);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakeemptygvec", esvec);
  EXPECT_FALSE(fut.hasError());

  //check call, type conv
  fut = proxy->call<int>("fakesvec", gvec);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakegvec", svec);
  EXPECT_FALSE(fut.hasError());
}

TEST(TestCall, TestGenericConversionComplexList) {
  TestSessionPair p;
  int serviceID;
  qi::ObjectPtr obj = createObject();
  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::ObjectPtr proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);


  std::vector<std::vector<std::string> >      sss;
  sss.resize(2);
  sss[0].push_back("titi");
  sss[1].push_back("toto");

  std::vector<std::vector<qi::GenericValue> > ssg;
  ssg.resize(2);
  ssg[0].push_back(qi::GenericValue::from(std::string("titi")));
  ssg[1].push_back(qi::GenericValue::from(std::string("toto")));

  std::vector<qi::GenericValue>               sg;
  std::vector<qi::GenericValue> paf;
  paf.push_back(qi::GenericValue::from(std::string("titi")));
  sg.push_back(qi::GenericValue::from(paf));
  paf[0] = qi::GenericValueRef(std::string("toto"));
  sg.push_back(qi::GenericValue::from(paf));

  qi::Future<int> fut;

  //Check empty, same type
  fut = proxy->call<int>("fakesvvec", sss);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakesvvec", ssg);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakesvvec", sg);
  EXPECT_FALSE(fut.hasError());

  fut = proxy->call<int>("fakegvvec", sss);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakegvvec", ssg);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakegvvec", sg);
  EXPECT_FALSE(fut.hasError());


  fut = proxy->call<int>("fakegvvec2", sss);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakegvvec2", ssg);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakegvvec2", sg);
  EXPECT_FALSE(fut.hasError());
}

TEST(TestCall, TestGenericConversionComplexMap) {
  TestSessionPair p;
  int serviceID;
  qi::ObjectPtr obj = createObject();
  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::ObjectPtr proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);


  std::vector<std::vector<std::string> >      sss;
  std::vector<std::vector<qi::GenericValue> > ssg;

  std::map<std::string, std::vector<std::vector<std::string> > >      msvvs;
  std::map<std::string, std::vector<std::vector<qi::GenericValue> > > msvvg;

  sss.resize(2);
  ssg.resize(2);
  sss[0].push_back("titi");
  ssg[0].push_back(qi::GenericValue::from(std::string("titi")));
  msvvs["i"] = sss;
  msvvg["i"] = ssg;

  sss[0][0] = "toto";
  ssg[0][0] = qi::GenericValue::from(std::string("toto"));
  msvvs["j"] = sss;
  msvvg["j"] = ssg;

  qi::Future<int> fut;

  fut = proxy->call<int>("fakemsvvec", msvvs);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakemsvvec", msvvg);
  EXPECT_FALSE(fut.hasError());

  fut = proxy->call<int>("fakemgvvec", msvvs);
  EXPECT_FALSE(fut.hasError());
  fut = proxy->call<int>("fakemgvvec", msvvg);
  EXPECT_FALSE(fut.hasError());
}


TEST(TestCall, TestGenericConversionTuple) {
  TestSessionPair p;
  int serviceID;
  qi::ObjectPtr obj = createObject();
  serviceID = p.server()->registerService("serviceConv", obj);
  ASSERT_TRUE(serviceID != 0);
  qi::ObjectPtr proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);

  qi::Future<double> f;
  SpecificTuple t;
  t.e1 = 1;
  t.e2 = 2;
  t.e3["foo"] = 3;
  f = proxy->call<double>("eatSpecific", t);
  EXPECT_FALSE(f.hasError());
  EXPECT_EQ(6, f.value());

  GenericTuple gt;
  gt.e1 = qi::GenericValueRef(1.0);
  gt.e2 = qi::GenericValueRef(2U);
  std::map<std::string, qi::GenericValue> map;
  map["foo"] = qi::GenericValueRef(3);
  gt.e3 = qi::GenericValueRef(map);
  f = proxy->call<double>("eatSpecific", gt);
  EXPECT_FALSE(f.hasError());
  EXPECT_EQ(6, f.value());

  std::map<unsigned int, std::string> ravMap;
  gt.e3 = qi::GenericValueRef(ravMap);
  f = proxy->call<double>("eatSpecific", gt);
  EXPECT_FALSE(f.hasError());
  EXPECT_EQ(3, f.value());
}


void set_true(bool* b)
{
  *b = true;
}

TEST(TestEventLoop, MonitorEventLoop)
{
  TestSessionPair p;
  bool loopStuck = false;
  qi::Future<void> f = qi::getDefaultObjectEventLoop()->monitorEventLoop(qi::getDefaultNetworkEventLoop(), 100000);
  f.connect(boost::bind(&set_true, &loopStuck));
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("delay", &qi::os::msleep);
  qi::ObjectPtr obj(ob.object());
  obj->forceEventLoop(qi::getDefaultObjectEventLoop());
  p.server()->registerService("delayer", obj);
  qi::ObjectPtr proxy = p.client()->service("delayer");
  ASSERT_TRUE(!loopStuck);
  proxy->call<void>("delay", 1000).wait();
  ASSERT_TRUE(loopStuck);
  qiLogDebug() << "Cancelling monitorEventLoop";
  f.cancel(); // or eventloops will get stuck
  qiLogDebug() << "Cancelling monitorEventLoop done";
}
int service_call(qi::Session* s, const std::string& obj,
  const std::string& method, int arg)
{
  qiLogDebug() << "TEST: servicecall";
  qi::ObjectPtr o = s->service(obj);
  return o->call<int>(method, arg);
}

void servicecall_addone(qi::Promise<int>& prom, qi::Session* s)
{
  qiLogDebug() << "TEST: call servicecall";
  qi::ObjectPtr obj2Proxy = s->service("caller");
  qiLogDebug() << "TEST: got service";
  qi::Future<int> v = obj2Proxy->call<int>("serviceCall", "adder", "addOne", 5);
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
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("addOne", &addOne);
  qi::ObjectPtr obj(ob.object());
  p.client()->registerService("adder", obj);
  qi::ObjectPtr o = p.server()->service("adder");
  ASSERT_TRUE(o);
}

TEST(TestCall, DeadLock)
{
  // This test deeadlocks if all objects are in the same monothreaded event loop
  qi::EventLoop* ev = new qi::EventLoop();
  ev->start();
  // One object calls another, both in singleThread mode
  TestSessionPair p;

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("addOne", &addOne);
  qi::ObjectPtr obj(ob.object());
  p.server()->registerService("adder", obj);

  qi::GenericObjectBuilder ob2;
  ob2.advertiseMethod("serviceCall",
    (boost::function<int(std::string, std::string, int)>)
    boost::bind(&service_call, p.client(), _1, _2, _3));
  qi::ObjectPtr obj2(ob2.object());
  p.client()->registerService("caller", obj2);

  qi::Promise<int> prom;

  // From the object event loop of process 'server', call a method from
  // object in client, which will call back a method in server
  qiLogDebug() << "TEST: go async servicecall_addone";
  qi::getDefaultObjectEventLoop()->async(
    boost::bind(&servicecall_addone, boost::ref(prom), p.server()));

  for (unsigned i=0; i<20 && !prom.future().isFinished(); ++i)
    qi::os::msleep(50);
  ASSERT_TRUE(prom.future().isFinished());
  ASSERT_EQ(6, prom.future().value());
}

void onEvent(int v, qi::Promise<int>& eventValue, qi::ObjectPtr* ptr)
{
  qiLogDebug() << "onEvent";
  if (v == 0)
  {
    qi::ObjectWeakPtr weak(*ptr);
    delete ptr;
    if (weak.lock())
      qiLogWarning() << "Object destruction is going to be asynchronous";
  }
  eventValue.setValue(v);
}


void bindObjectEvent(qi::ObjectPtr ptr, const std::string& eventName,
  qi::Promise<int>& eventValue)
{
  // Keep ptr alive
  ptr->connect(eventName, boost::bind(&onEvent, _1, eventValue,
    new qi::ObjectPtr(ptr)));
}

int makeObjectCall(qi::ObjectPtr ptr, const std::string& fname, int arg)
{
  return ptr->call<int>(fname, arg).value();
}

void bounceFuture(qi::Future<int> s, qi::Promise<int> d, qi::ObjectPtr obj)
{
  if (s.hasValue())
    d.setValue(s.value());
  else
    d.setError(s.error());
}

void onMakeObjectCall(qi::ObjectPtr ptr, const std::string& fname, int arg,
  qi::Promise<int>& result)
{
  qiLogDebug() << "onMakeObjectCall";
  qi::Future<int> fut = ptr->call<int>(fname, arg);
  // We must keep ptr alive until the call returns
  fut.connect(boost::bind(&bounceFuture, _1, result, ptr));
}

TEST(TestCall, TestObjectPassing)
{
  TestSessionPair p;
  qi::Promise<int> eventValue;

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("makeObjectCall", &makeObjectCall);
  ob.advertiseMethod("bindObjectEvent", boost::bind(&bindObjectEvent, _1, _2, eventValue));
  qi::ObjectPtr obj(ob.object());
  p.server()->registerService("s", obj);
  qi::ObjectPtr proxy = p.client()->service("s");

  qi::ObjectPtr unregisteredObj;
  {
    qi::GenericObjectBuilder ob;
    ob.advertiseMethod("add", &addOne);
    ob.advertiseEvent<void(int)>("fire");
    unregisteredObj = ob.object();
  }
  // Transmit unregisteredObj through the network.
  qi::Future<int> v = proxy->call<int>("makeObjectCall", unregisteredObj, "add", 1);
  v.wait(2000);
  ASSERT_TRUE(!v.hasError());
  ASSERT_EQ(2, v.value());
  proxy->call<void>("bindObjectEvent", unregisteredObj, "fire").wait();
  unregisteredObj->emitEvent("fire", 42);
  eventValue.future().wait(); //fixme wait(2s)
  ASSERT_TRUE(eventValue.future().isFinished());
  ASSERT_EQ(42, eventValue.future().value());
  eventValue.reset();

  // Check that object is locked by remote end
  qi::ObjectWeakPtr unregisteredWeakObj(unregisteredObj);
  unregisteredObj.reset();
  unregisteredObj = unregisteredWeakObj.lock();
  ASSERT_TRUE(unregisteredObj);
  if (p.client() == p.server())
    return; // test makes no sense in direct mode
  // This will delete the proxy
  unregisteredObj->emitEvent("fire", 0);
  eventValue.future().wait();
  ASSERT_TRUE(eventValue.future().isFinished());
  ASSERT_EQ(0, eventValue.future().value());
  eventValue.reset();
  ASSERT_TRUE(!eventValue.future().isFinished());
  unregisteredObj->emitEvent("fire", 1);
  eventValue.future().wait(1000);
  ASSERT_TRUE(!eventValue.future().isFinished());

  // Check that unregisteredObj is no longuer held
  unregisteredObj.reset();
  ASSERT_FALSE(unregisteredWeakObj.lock());
}

TEST(TestCall, TestObjectPassingReverse)
{ // Test server->client object passing (through emit)
  TestSessionPair p;
  qi::GenericObjectBuilder ob;
  ob.advertiseEvent<void (qi::ObjectPtr ptr, const std::string& fname, int arg)>("makeObjectCallEvent");

  qi::ObjectPtr obj(ob.object());

  p.server()->registerService("s", obj);
  qi::ObjectPtr proxy = p.client()->service("s");

  qi::ObjectPtr unregisteredObj;
  {
    qi::GenericObjectBuilder ob;
    ob.advertiseMethod("add", &addOne);
    ob.advertiseEvent<void(int)>("fire");
    unregisteredObj = ob.object();
  }

  qi::Promise<int> value;
  // We connect a method client-side
  proxy->connect("makeObjectCallEvent", boost::bind(&onMakeObjectCall, _1, _2, _3, value)).wait();
  // And emit server-side, this is the reverse of a method call
  obj->emitEvent("makeObjectCallEvent", unregisteredObj, "add", 41);
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

qi::ObjectPtr makeAdder(qi::ObjectWeakPtr& weak)
{
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("add", &addOne);
  ob.advertiseEvent<void(int)>("fire");
  qi::ObjectPtr res = ob.object();
  qiLogDebug() << "unregistered object is " << res.get();
  weak = res;
  return res;
}

TEST(TestCall, TestObjectPassingReturn)
{
  TestSessionPair p;
  qi::GenericObjectBuilder ob;
  qi::ObjectWeakPtr weak;
  ob.advertiseMethod("makeAdder", boost::function<qi::ObjectPtr()>(boost::bind(&makeAdder, boost::ref(weak))));
  qi::ObjectPtr obj(ob.object());

  p.server()->registerService("s", obj);
  qi::ObjectPtr proxy = p.client()->service("s");

  qi::ObjectPtr adder = proxy->call<qi::ObjectPtr>("makeAdder");
  ASSERT_TRUE(weak.lock());
  // One is client side, the other server side
  if (p.client() != p.server())
    ASSERT_FALSE(weak.lock().get() == adder.get());
  ASSERT_TRUE(adder);
  qi::Future<int> f = adder->call<int>("add", 41);
  f.wait(1000);
  ASSERT_TRUE(f.isFinished());
  ASSERT_EQ(42, f.value());
  adder.reset();
  qi::os::msleep(300);
  ASSERT_FALSE(weak.lock());
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
  GenericObjectBuilder ob;
  ob.advertiseMethod("pingChar", &pingChar);
  ob.advertiseMethod("pingUChar", &pingUChar);
  ob.advertiseMethod("pingInt", &pingInt);
  ob.advertiseMethod("pingUInt", &pingUInt);

  ob.xAdvertiseMethod("i", "pingChar_i::(i)", makeGenericFunction(&pingChar).dropFirstArgument());
  ObjectPtr obj = ob.object();
  p.server()->registerService("ping", obj);

  ObjectPtr client = p.client()->service("ping");
  ASSERT_EQ(1, client->call<int>("pingChar", 1));
  try
  {
    // Return cast check
    EXPECTTHROW(client->call<char>("pingInt", 256).value());
    EXPECTTHROW(client->call<char>("pingInt", 128).value());
    EXPECTTHROW(client->call<unsigned char>("pingInt", -1).value());
    EXPECTTHROW(client->call<unsigned int>("pingInt", -1).value());
    // call arg check
    EXPECTTHROW(client->call<int>("pingUInt", -1).value());
    EXPECTTHROW(client->call<int>("pingChar", 128).value());
    EXPECTTHROW(client->call<int>("pingUChar", 256).value());
    // call arg check at call backend site
    EXPECTTHROW(client->call<int>("pingChar_i", 512).value());
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
  GenericObjectBuilder ob;
  ob.advertiseMethod("pingChar", &pingChar);
  ob.advertiseMethod("pingUChar", &pingUChar);
  ob.advertiseMethod("pingInt", &pingInt);
  ob.advertiseMethod("pingString", &pingString);
  ob.advertiseMethod("ping", &pingInt);
  ob.advertiseMethod("ping", &pingString);
  ObjectPtr obj = ob.object();
  p.server()->registerService("ping", obj);

  ObjectPtr client = p.client()->service("ping");
  // no .value() on expectthrow: we expect a synchronous exception
  EXPECTTHROW(client->call<std::string>("pingInt", "foo"));
  ASSERT_EQ("foo", client->call<std::string>("ping", "foo").value());
  ASSERT_EQ(12, client->call<int>("ping", 12).value());
  EXPECTTHROW(client->call<std::string>("ping::(s)", 5));
  EXPECTTHROW(client->call<int>("ping::(i)", "foo"));
  ASSERT_EQ(12, client->call<int>("ping::(i)", (char)12).value());
  ASSERT_EQ("foo", client->call<std::string>("ping::(s)", "foo").value());
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
  qi::GenericObjectBuilder gob;
  gob.advertiseMethod("delaySet", &delaySet);
  qi::ObjectPtr sobj = gob.object();
  p.server()->registerService("delayer", sobj);
  qi::ObjectPtr obj = p.client()->service("delayer");
  qi::Future<int> f = obj->call<int>("delaySet", 500, 41);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_EQ(41, f.value());
  f =  obj->call<int>("delaySet", 500, -1);
  ASSERT_TRUE(!f.isFinished());
  f.wait();
  ASSERT_TRUE(f.hasError());
}


int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
