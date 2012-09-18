/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/



#include <map>
#include <gtest/gtest.h>
#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/objectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/service_directory.hpp>



qi::detail::DynamicValue v;
static qi::Promise<int> *payload;

void onFire(const int& pl)
{
  std::cout << "onFire:" << pl << std::endl;
  std::cout.flush();
  payload->setValue(pl);
}

void value(qi::GenericValue mv)
{
  mv.type->toValue(mv.value, v);
  payload->setValue(0);
}

void valueList(std::vector<qi::GenericValue> mv)
{
  qi::detail::DynamicValue::DynamicValueList vl;
  for (unsigned i=0; i<mv.size(); ++i)
  {
    vl.push_back(qi::detail::DynamicValue());
    mv[i].type->toValue(mv[i].value, vl.back());
  }
  v.setList(vl);
  payload->setValue(0);
}

class TestObject: public ::testing::Test
{
public:
  TestObject()
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseEvent<void (*)(const int&)>("fire");
    ob.advertiseMethod("value", &value);
    ob.advertiseMethod("value", &valueList);
    oserver = ob.object();
  }

protected:
  void SetUp()
  {
    ASSERT_TRUE(sd.listen("tcp://127.0.0.1:0"));
    ASSERT_TRUE(session.connect(sd.listenUrl()));
    ASSERT_TRUE(session.listen("tcp://0.0.0.0:0"));
    ASSERT_GT(session.registerService("coin", oserver).wait(), 0);
    EXPECT_EQ(1U, session.services(qi::Session::ServiceLocality_Local).value().size());

    ASSERT_TRUE(sclient.connect(sd.listenUrl()));
    std::vector<qi::ServiceInfo> services = sclient.services();
    EXPECT_EQ(2U, services.size());
    oclient = sclient.service("coin");
    payload = &prom;
  }

  void TearDown()
  {
    payload = 0;
    sclient.close();
    session.close();
    sd.close();
  }

public:
  qi::Promise<int>     prom;
  qi::ServiceDirectory sd;
  qi::Session          session;
  qi::Object           oserver;
  qi::Session          sclient;
  qi::Object           oclient;
};


TEST_F(TestObject, meta)
{
  using namespace qi;
  // Remote test
  Object target = oclient;
  {
    /* WATCH OUT, qi::AutoGenericValue(12) is what call expects!
    * So call(AutoGenericValue(12)) will *not* call with the value
    * "a metavalue containing 12", it will call with "12".
    */
  target.call<void>("value", 12).wait();
  ASSERT_EQ(v.toDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target.call<void>("value", myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target.call<void>("value", GenericValue(AutoGenericValue(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  target.call<void>("value", qi::GenericValue(qi::AutoGenericValue(12))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>("value", qi::GenericValue(qi::AutoGenericValue(12.0))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>("value", qi::GenericValue(qi::AutoGenericValue(12.0f))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>("value", qi::GenericValue(qi::AutoGenericValue("foo"))).wait();
  ASSERT_EQ(v.toString(), "foo");
  target.call<void>("value", "foo").wait();
  ASSERT_EQ(v.toString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target.call<void>("value", qi::GenericValue(qi::AutoGenericValue(in))).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  target.call<void>("value", in).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  std::vector<GenericValue> args;
  args.push_back(AutoGenericValue(12));
  args.push_back(AutoGenericValue("foo"));
  args.push_back(AutoGenericValue(in));
  target.call<void>("value", args).wait();
  ASSERT_EQ(v.type, detail::DynamicValue::List);
  detail::DynamicValue::DynamicValueList res = v.toList();
  ASSERT_EQ(static_cast<size_t>(3), res.size());
  ASSERT_EQ(12, res[0].toDouble());
  ASSERT_EQ("foo", res[1].toString());
  ASSERT_EQ(in, res[2].as<std::vector<double> >());
  }
  // Plugin copy test
  target = oserver;
  {
  target.call<void>("value", 12).wait();
  ASSERT_EQ(v.toDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target.call<void>("value", myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target.call<void>("value", GenericValue(AutoGenericValue(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.toDouble(), 12);
  }
  target.call<void>("value", qi::GenericValue(qi::AutoGenericValue(12))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>("value", qi::GenericValue(qi::AutoGenericValue(12.0))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>("value", qi::GenericValue(qi::AutoGenericValue(12.0f))).wait();
  ASSERT_EQ(v.toDouble(), 12);
  target.call<void>("value", qi::GenericValue(qi::AutoGenericValue("foo"))).wait();
  ASSERT_EQ(v.toString(), "foo");
  target.call<void>("value", "foo").wait();
  ASSERT_EQ(v.toString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target.call<void>("value", qi::GenericValue(qi::AutoGenericValue(in))).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  target.call<void>("value", in).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  std::vector<GenericValue> args;
  args.push_back(AutoGenericValue(12));
  args.push_back(AutoGenericValue("foo"));
  args.push_back(AutoGenericValue(in));
  target.call<void>("value", args).wait();
  ASSERT_EQ(v.type, detail::DynamicValue::List);
  detail::DynamicValue::DynamicValueList res = v.toList();
  ASSERT_EQ(static_cast<size_t>(3), res.size());
  ASSERT_EQ(12, res[0].toDouble());
  ASSERT_EQ("foo", res[1].toString());
  ASSERT_EQ(in, res[2].as<std::vector<double> >());
  }
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
