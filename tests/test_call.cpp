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

TEST(TestMessage, COW)
{
  qi::Message m1;
  const void* ptr = &m1.signature();
  m1.setSignature("s");
  ASSERT_EQ(ptr, &m1.signature());
  qi::Message m2(m1);
  ASSERT_EQ(ptr, &m1.signature());
  ASSERT_EQ(ptr, &m2.signature());
  m2.setService(1);
  ASSERT_TRUE(&m1.signature() != &m2.signature());
}

int addOne(int v)
{
  qiLogDebug("test") << "addOne";
  return v+1;
}

int print(std::list<std::pair<std::string, int> > robots)
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
  EXPECT_EQ("titi", sval[0].asString());
  EXPECT_EQ("toto", sval[1].asString());
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
  EXPECT_EQ("titi", sval[0][0].asString());
  EXPECT_EQ("toto", sval[1][0].asString());
  return 0;
}

int fakegvvec2(const std::vector< qi::GenericValue > &sval) {
  std::cout << "sval.size(): " << sval.size() << std::endl;
  EXPECT_EQ("titi", sval[0].as< std::vector<std::string> >()[0]);
  EXPECT_EQ("toto", sval[1].as< std::vector<std::string> >()[0]);
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
  EXPECT_EQ("titi", sval["i"][0][0].asString());
  EXPECT_EQ("toto", sval["j"][0][0].asString());
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
    qi::GenericListPtr l = val.asList();
    ASSERT_TRUE(l.type);
    ASSERT_EQ(3, l.size());
    qi::GenericListIteratorPtr it = l.begin();
    ASSERT_EQ(12, (*it).asInt());
    ++it;
    qi::Buffer bufRes = (*it).as<qi::Buffer>();
    ASSERT_EQ(strlen("canard")+1, bufRes.size());
    ++it;
    ASSERT_EQ("foo", (*it).asString());
    it.destroy();
  }
  {
    std::vector<qi::GenericValue> l = result.as<std::vector<qi::GenericValue> >();
    ASSERT_EQ(3, l.size());
    std::vector<qi::GenericValue>::iterator it = l.begin();
    ASSERT_EQ(12, (*it).asInt());
    ++it;
    qi::Buffer bufRes = (*it).as<qi::Buffer>();
    ASSERT_EQ(strlen("canard")+1, bufRes.size());
    ++it;
    ASSERT_EQ("foo", (*it).asString());
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

  ASSERT_EQ(4, proxy->call<int>("print", robots));

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
  gv = qi::GenericValuePtr::from(std::string("titi"));
  gvec.push_back(gv);
  gv = qi::GenericValuePtr::from(std::string("toto"));
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
  ssg[0].push_back(qi::GenericValuePtr::from(std::string("titi")));
  ssg[1].push_back(qi::GenericValuePtr::from(std::string("toto")));

  std::vector<qi::GenericValue>               sg;
  std::vector<qi::GenericValue> paf;
  paf.push_back(qi::GenericValuePtr::from(std::string("titi")));
  sg.push_back(qi::GenericValuePtr::from(paf));
  paf[0] = qi::GenericValuePtr::from(std::string("toto"));
  sg.push_back(qi::GenericValuePtr::from(paf));

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
  ssg[0].push_back(qi::GenericValuePtr::from(std::string("titi")));
  msvvs["i"] = sss;
  msvvg["i"] = ssg;

  sss[0][0] = "toto";
  ssg[0][0] = qi::GenericValuePtr::from(std::string("toto"));
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
  gt.e1 = qi::GenericValuePtr::from(1.0);
  gt.e2 = qi::GenericValuePtr::from(2U);
  std::map<std::string, qi::GenericValue> map;
  map["foo"] = qi::GenericValuePtr::from(3);
  gt.e3 = qi::GenericValuePtr::from(map);
  f = proxy->call<double>("eatSpecific", gt);
  EXPECT_FALSE(f.hasError());
  EXPECT_EQ(6, f.value());

  std::map<unsigned int, std::string> ravMap;
  gt.e3 = qi::GenericValuePtr::from(ravMap);
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
  qiLogDebug("qi.test") << "Cancelling monitorEventLoop";
  f.cancel(); // or eventloops will get stuck
  qiLogDebug("qi.test") << "Cancelling monitorEventLoop done";
}
int service_call(qi::Session* s, const std::string& obj,
  const std::string& method, int arg)
{
  qiLogDebug("qi.test") << "TEST: servicecall";
  qi::ObjectPtr o = s->service(obj);
  return o->call<int>(method, arg);
}

void servicecall_addone(qi::Promise<int>& prom, qi::Session* s)
{
  qiLogDebug("qi.test") << "TEST: call servicecall";
  qi::ObjectPtr obj2Proxy = s->service("caller");
  qiLogDebug("qi.test") << "TEST: got service";
  qi::Future<int> v = obj2Proxy->call<int>("serviceCall", "adder", "addOne", 5);
  v.wait(500);
  if (!v.isReady())
    prom.setError("timeout");
  else if (v.hasError())
    prom.setError(v.error());
  else
    prom.setValue(v.value());
}

TEST(TestCall, PairClientListen)
{
  TestSessionPair p;
  p.client()->listen("tcp://127.0.0.1:0");
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
  p.client()->listen("tcp://127.0.0.1:0");


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
  qiLogDebug("qi.test") << "TEST: go async servicecall_addone";
  qi::getDefaultObjectEventLoop()->async(
    boost::bind(&servicecall_addone, boost::ref(prom), p.server()));

  for (unsigned i=0; i<20 && !prom.future().isReady(); ++i)
    qi::os::msleep(50);
  ASSERT_TRUE(prom.future().isReady());
  ASSERT_EQ(6, prom.future().value());
}
int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
