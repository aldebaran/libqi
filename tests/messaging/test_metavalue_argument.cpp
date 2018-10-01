/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <map>
#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/type/typeinterface.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/session.hpp>

qiLogCategory("test");

qi::AnyValue v;

namespace
{
void value(qi::AnyValue mv)
{
  v = mv;
}

void valueList(std::vector<qi::AnyValue> mv)
{
  v = qi::AnyValue(mv);
}
} // anonymous

class MetaValueArgument: public ::testing::Test
{
public:
  MetaValueArgument()
  {
    sd = qi::makeSession();
    session = qi::makeSession();
    sclient = qi::makeSession();

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
    qi::Future<void> f = sd->listenStandalone("tcp://127.0.0.1:0");
    f.wait(3000);
    ASSERT_TRUE(!f.hasError());
    f = session->connect(sd->endpoints()[0]);
    f.wait(3000);
    ASSERT_TRUE(!f.hasError());
    f = session->listen("tcp://0.0.0.0:0");
    f.wait(3000);
    ASSERT_TRUE(!f.hasError());
    ASSERT_TRUE(session->registerService("coin", oserver).hasValue(3000));
    EXPECT_EQ(1U, session->services(qi::Session::ServiceLocality_Local).value().size());

    f = sclient->connect(sd->endpoints()[0]);
    f.wait(3000);
    ASSERT_TRUE(!f.hasError());
    std::vector<qi::ServiceInfo> services = sclient->services().value();
    EXPECT_EQ(2U, services.size());
    oclient = sclient->service("coin").value();
  }

  void TearDown()
  {
    sclient->close();
    session->close();
    sd->close();
  }

public:
  qi::Promise<int>     prom;
  qi::SessionPtr       sd;
  qi::SessionPtr       session;
  qi::AnyObject        oserver;
  qi::SessionPtr       sclient;
  qi::AnyObject        oclient;
};


TEST_F(MetaValueArgument, all)
{
  using namespace qi;
  qi::int64_t time = os::ustime();
  // Remote test
  AnyObject target = oclient;
  std::string function = "value";
  ASSERT_TRUE(target);
  {
  target.call<void>(function, 12);
  ASSERT_EQ(v.toDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target.async<void>(function, myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target.async<void>(function, AnyValue::from(myint));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  target.call<void>(function, qi::AnyValue::from(12));
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>(function, qi::AnyValue::from(12.0));
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>(function, qi::AnyValue::from(12.0f));
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>(function, qi::AnyValue::from("foo"));
  ASSERT_EQ(v.toString(), "foo");
  target.call<void>(function, "foo");
  ASSERT_EQ(v.toString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target.call<void>(function, qi::AnyValue::from(in));
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  target.call<void>(function, in);
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  AnyValueVector args;
  args.push_back(AnyValue::from(12));
  args.push_back(AnyValue::from("foo"));
  args.push_back(AnyValue::from(in));
  target.call<void>(function, args);
  ASSERT_EQ(v.kind(), TypeKind_List);
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
  target.call<void>(function, 12);
  ASSERT_EQ(v.toDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target.async<void>(function, myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target.async<void>(function, AnyValue::from(myint));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  target.call<void>(function, qi::AnyValue::from(12));
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>(function, qi::AnyValue::from(12.0));
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>(function, qi::AnyValue::from(12.0f));
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>(function, qi::AnyValue::from("foo"));
  ASSERT_EQ(v.toString(), "foo");
  target.call<void>(function, "foo");
  ASSERT_EQ(v.toString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target.call<void>(function, qi::AnyValue::from(in));
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  target.call<void>(function, in);
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  target.call<void>(function, in);
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  AnyValueVector args;
  args.push_back(AnyValue::from(12));
  args.push_back(AnyValue::from("foo"));
  args.push_back(AnyValue::from(in));
  target.call<void>(function, args);
  ASSERT_EQ(v.kind(), TypeKind_List);
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
  target.call<void>(function, 12);
  ASSERT_EQ(v.toDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target.async<void>(function, myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target.async<void>(function, AnyValue::from(myint));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  target.call<void>(function, qi::AnyValue::from(12));
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>(function, qi::AnyValue::from(12.0));
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>(function, qi::AnyValue::from(12.0f));
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>(function, qi::AnyValue::from("foo"));
  ASSERT_EQ(v.toString(), "foo");
  target.call<void>(function, "foo");
  ASSERT_EQ(v.toString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target.call<void>(function, qi::AnyValue::from(in));
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  target.call<void>(function, in);
  ASSERT_EQ(v.to<std::vector<double> >(), in);
  AnyValueVector args;
  args.push_back(AnyValue::from(12));
  args.push_back(AnyValue::from("foo"));
  args.push_back(AnyValue::from(in));
  target.call<void>(function, args);
  ASSERT_EQ(v.kind(), TypeKind_List);
  // iterate
  ASSERT_EQ(12, v[0].toDouble());
  ASSERT_EQ("foo", v[1].toString());
  ASSERT_EQ(in, v[2].to<std::vector<double> >());
  }
  qiLogVerbose() << "plugin sync us: " << os::ustime() - time;
  time = os::ustime();
}
