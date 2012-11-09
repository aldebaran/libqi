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
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <testsession/testsessionpair.hpp>

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
  float duration = 0.42;

  ob.advertiseMethod("fakeRGB", &fakeRGBd);
  qi::ObjectPtr obj(ob.object());

  serviceID = p.server()->registerService("serviceConv", obj);
  qi::ObjectPtr proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);

  std::cout << "Calling FakeRGB" << std::endl;
  qi::Future<int> fut = proxy->call<int>("fakeRGB", "Haha", 42, duration);
}

TEST(TestCall, TestGenericVector) {
  TestSessionPair p;
  qi::GenericObjectBuilder ob;

  std::vector<std::string>      svec;
  std::vector<qi::GenericValue> gvec;

  std::vector<std::string>      esvec;
  std::vector<qi::GenericValue> egvec;

  svec.push_back("titi");
  svec.push_back("toto");

  qi::GenericValue gv;
  gv = qi::GenericValue::from(std::string("titi"));
  gvec.push_back(gv);
  gv = qi::GenericValue::from(std::string("toto"));
  gvec.push_back(gv);

  int serviceID;
  float duration = 0.42;

  ob.advertiseMethod("fakesvec", &fakesvec);
  ob.advertiseMethod("fakegvec", &fakegvec);
  ob.advertiseMethod("fakeemptysvec", &fakeemptysvec);
  ob.advertiseMethod("fakeemptygvec", &fakeemptygvec);
  qi::ObjectPtr obj(ob.object());

  serviceID = p.server()->registerService("serviceConv", obj);
  qi::ObjectPtr proxy = p.client()->service("serviceConv");
  ASSERT_TRUE(proxy != 0);

  std::cout << "Calling FakeRGB" << std::endl;

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

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
