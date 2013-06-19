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

qiLogCategory("test");

qi::AnyValue v;

void onFire(const int& pl)
{
  std::cout << "onFire:" << pl << std::endl;
  std::cout.flush();
}

void value(qi::AnyValue mv)
{
  v = mv;
}

void valueList(std::vector<qi::AnyValue> mv)
{
  v = qi::AnyReference(mv).clone();
}

class TestObject: public ::testing::Test
{
public:
  TestObject()
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseSignal<const int&>("fire");
    ob.advertiseMethod("value", &value);
    ob.advertiseMethod("value", &valueList);
    ob.advertiseMethod("valueAsync", &value, "", qi::MetaCallType_Queued);
    ob.advertiseMethod("valueAsync", &valueList, "", qi::MetaCallType_Queued);
    oserver = ob.object();
  }

protected:
  void SetUp()
  {
    qi::Future<void> f = sd.listen("tcp://127.0.0.1:0");
    f.wait(3000);
    ASSERT_TRUE(!f.hasError());
    f = session.connect(sd.endpoints()[0]);
    f.wait(3000);
    ASSERT_TRUE(!f.hasError());
    f = session.listen("tcp://0.0.0.0:0");
    f.wait(3000);
    ASSERT_TRUE(!f.hasError());
    ASSERT_TRUE(session.registerService("coin", oserver).hasValue(3000));
    EXPECT_EQ(1U, session.services(qi::Session::ServiceLocality_Local).value().size());

    f = sclient.connect(sd.endpoints()[0]);
    f.wait(3000);
    ASSERT_TRUE(!f.hasError());
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
  qi::AnyObject        oserver;
  qi::Session          sclient;
  qi::AnyObject        oclient;
};


TEST_F(TestObject, meta)
{
  using namespace qi;
  qi::int64_t time = os::ustime();
  // Remote test
  AnyObject target = oclient;
  std::string function = "value";
  ASSERT_TRUE(target);
  {
    /* WATCH OUT, qi::AutoAnyReference(12) is what call expects!
    * So call(AutoAnyReference(12)) will *not* call with the value
    * "a metavalue containing 12", it will call with "12".
    */
  target->call<void>(function, 12).wait();
  ASSERT_EQ(v.toDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, AutoAnyReference(AnyValue::from(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(12))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(12.0))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(12.0f))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from("foo"))).wait();
  ASSERT_EQ(v.toString(), "foo");
  target->call<void>(function, "foo").wait();
  ASSERT_EQ(v.toString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(in))).wait();
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  target->call<void>(function, in).wait();
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  std::vector<AnyValue> args;
  args.push_back(AnyValue::from(12));
  args.push_back(AnyValue::from("foo"));
  args.push_back(AnyValue::from(in));
  target->call<void>(function, args).wait();
  ASSERT_EQ(v.kind(), TypeInterface::List);
  ASSERT_EQ(static_cast<size_t>(3), v.size());

  // iterate
  EXPECT_EQ(12, v[0].toDouble());
  ASSERT_EQ("foo", v[1].toString());
  ASSERT_EQ(in, v[2].to<std::vector<double> >());
  }
  qiLogVerbose() << "remote us: " << os::ustime() - time;
  time = os::ustime();

  // Plugin copy test
  target = oserver;
  function = "valueAsync";
  {
  target->call<void>(function, 12).wait();
  ASSERT_EQ(v.toDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, AutoAnyReference(AnyValue::from(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(12))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(12.0))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(12.0f))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from("foo"))).wait();
  ASSERT_EQ(v.toString(), "foo");
  target->call<void>(function, "foo").wait();
  ASSERT_EQ(v.toString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(in))).wait();
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  target->call<void>(function, in).wait();
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  target->call<void>(function, in).wait();
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  std::vector<AnyValue> args;
  args.push_back(AnyValue::from(12));
  args.push_back(AnyValue::from("foo"));
  args.push_back(AnyValue::from(in));
  target->call<void>(function, args).wait();
  ASSERT_EQ(v.kind(), TypeInterface::List);
  // iterate
  ASSERT_EQ(12, v[0].toDouble());
  ASSERT_EQ("foo", v[1].toString());
  ASSERT_EQ(in, v[2].to<std::vector<double> >());
  }
  qiLogVerbose() << "plugin async us: " << os::ustime() - time;
  time = os::ustime();
  // plugin direct test
  function = "value";
  {
  target->call<void>(function, 12).wait();
  ASSERT_EQ(v.toDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>(function, AutoAnyReference(AnyValue::from(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(12))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(12.0))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(12.0f))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from("foo"))).wait();
  ASSERT_EQ(v.toString(), "foo");
  target->call<void>(function, "foo").wait();
  ASSERT_EQ(v.toString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target->call<void>(function, qi::AutoAnyReference(qi::AnyValue::from(in))).wait();
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  target->call<void>(function, in).wait();
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  std::vector<AnyValue> args;
  args.push_back(AnyValue::from(12));
  args.push_back(AnyValue::from("foo"));
  args.push_back(AnyValue::from(in));
  target->call<void>(function, args).wait();
  ASSERT_EQ(v.kind(), TypeInterface::List);
  // iterate
  ASSERT_EQ(12, v[0].toDouble());
  ASSERT_EQ("foo", v[1].toString());
  ASSERT_EQ(in, v[2].to<std::vector<double> >());
  }
  qiLogVerbose() << "plugin sync us: " << os::ustime() - time;
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
