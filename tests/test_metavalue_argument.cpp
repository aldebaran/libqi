/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/



#include <map>
#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>
#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qitype/type.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/servicedirectory.hpp>

qi::GenericValue v;

void onFire(const int& pl)
{
  std::cout << "onFire:" << pl << std::endl;
  std::cout.flush();
}

void value(qi::GenericValue mv)
{
  v = mv;
}

void valueList(std::vector<qi::GenericValue> mv)
{
  v = qi::GenericValuePtr::from(mv).clone();
}

class TestObject: public ::testing::Test
{
public:
  TestObject()
  {
    qi::GenericObjectBuilder ob;
    ob.advertiseEvent<void (*)(const int&)>("fire");
    ob.advertiseMethod("value", &value);
    ob.advertiseMethod("value", &valueList);
    ob.advertiseMethod("valueAsync", &value, "", qi::MetaCallType_Queued);
    ob.advertiseMethod("valueAsync", &valueList, "", qi::MetaCallType_Queued);
    oserver = ob.object();
  }

protected:
  void SetUp()
  {
    ASSERT_TRUE(sd.listen("tcp://127.0.0.1:0"));
    ASSERT_TRUE(session.connect(sd.endpoints()[0]));
    ASSERT_TRUE(session.listen("tcp://0.0.0.0:0"));
    ASSERT_GT(session.registerService("coin", oserver).wait(), 0);
    EXPECT_EQ(1U, session.services(qi::Session::ServiceLocality_Local).value().size());

    ASSERT_TRUE(sclient.connect(sd.endpoints()[0]));
    std::vector<qi::ServiceInfo> services = sclient.services();
    EXPECT_EQ(2U, services.size());
    oclient = sclient.service("coin");
  }

  void TearDown()
  {
    sclient.close();
    session.close();
    sd.close();
  }

public:
  qi::Promise<int>     prom;
  qi::ServiceDirectory sd;
  qi::Session          session;
  qi::ObjectPtr        oserver;
  qi::Session          sclient;
  qi::ObjectPtr        oclient;
};


TEST_F(TestObject, meta)
{
  using namespace qi;
  qi::int64_t time = os::ustime();
  // Remote test
  ObjectPtr target = oclient;
  std::string function = "value";
  ASSERT_TRUE(target);
  {
    /* WATCH OUT, qi::AutoGenericValuePtr(12) is what call expects!
    * So call(AutoGenericValuePtr(12)) will *not* call with the value
    * "a metavalue containing 12", it will call with "12".
    */
  target->call<void>(function, 12).wait();
  ASSERT_EQ(v.asDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, AutoGenericValuePtr(GenericValue::from(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(12))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(12.0))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(12.0f))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from("foo"))).wait();
  ASSERT_EQ(v.asString(), "foo");
  target->call<void>(function, "foo").wait();
  ASSERT_EQ(v.asString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(in))).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  target->call<void>(function, in).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  std::vector<GenericValue> args;
  args.push_back(AutoGenericValuePtr(12));
  args.push_back(AutoGenericValuePtr("foo"));
  args.push_back(AutoGenericValuePtr(in));
  target->call<void>(function, args).wait();
  ASSERT_EQ(v.kind(), Type::List);
  GenericListPtr l = v.asList();
  ASSERT_EQ(static_cast<size_t>(3), l.size());
  GenericListIteratorPtr i = l.begin();
  // iterate
  EXPECT_EQ(12, (*i).asDouble());
  i++;
  ASSERT_EQ("foo", (*i).asString());
  i++;
  ASSERT_EQ(in, (*i).as<std::vector<double> >());
  }
  qiLogVerbose("test") << "remote us: " << os::ustime() - time;
  time = os::ustime();

  // Plugin copy test
  target = oserver;
  function = "valueAsync";
  {
  target->call<void>(function, 12).wait();
  ASSERT_EQ(v.asDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, AutoGenericValuePtr(GenericValue::from(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(12))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(12.0))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(12.0f))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from("foo"))).wait();
  ASSERT_EQ(v.asString(), "foo");
  target->call<void>(function, "foo").wait();
  ASSERT_EQ(v.asString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(in))).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  target->call<void>(function, in).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  target->call<void>(function, in).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  std::vector<GenericValue> args;
  args.push_back(AutoGenericValuePtr(12));
  args.push_back(AutoGenericValuePtr("foo"));
  args.push_back(AutoGenericValuePtr(in));
  target->call<void>(function, args).wait();
  ASSERT_EQ(v.kind(), Type::List);
  GenericListPtr l = v.asList();
  GenericListIteratorPtr i = l.begin();
  // iterate
  ASSERT_EQ(12, (*i).asDouble());
  i++;
  ASSERT_EQ("foo", (*i).asString());
  i++;
  ASSERT_EQ(in, (*i).as<std::vector<double> >());
  }
  qiLogVerbose("test") << "plugin async us: " << os::ustime() - time;
  time = os::ustime();
  // plugin direct test
  function = "value";
  {
  target->call<void>(function, 12).wait();
  ASSERT_EQ(v.asDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, AutoGenericValuePtr(GenericValue::from(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(12))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(12.0))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(12.0f))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from("foo"))).wait();
  ASSERT_EQ(v.asString(), "foo");
  target->call<void>(function, "foo").wait();
  ASSERT_EQ(v.asString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target->call<void>(function, qi::AutoGenericValuePtr(qi::GenericValue::from(in))).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  target->call<void>(function, in).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  std::vector<GenericValue> args;
  args.push_back(AutoGenericValuePtr(12));
  args.push_back(AutoGenericValuePtr("foo"));
  args.push_back(AutoGenericValuePtr(in));
  target->call<void>(function, args).wait();
  ASSERT_EQ(v.kind(), Type::List);
  GenericListPtr l = v.asList();
  GenericListIteratorPtr i = l.begin();
  // iterate
  ASSERT_EQ(12, (*i).asDouble());
  i++;
  ASSERT_EQ("foo", (*i).asString());
  i++;
  ASSERT_EQ(in, (*i).as<std::vector<double> >());
  }
  qiLogVerbose("test") << "plugin sync us: " << os::ustime() - time;
  time = os::ustime();
}

int main(int argc, char *argv[])
{
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
