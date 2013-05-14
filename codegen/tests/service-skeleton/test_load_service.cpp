 /*
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <gtest/gtest.h>

#include <qi/application.hpp>

#include <qitype/genericobject.hpp>
#include <qitype/objectfactory.hpp>

#include <simpleservice/simpleservice-proxy.hpp>

void notify(int v, qi::Promise<int>& prom)
{
  prom.setValue(v);
}

TEST(TestLoadService, Load)
{
  std::vector<std::string> objs = qi::loadObject("simpleservice");
  ASSERT_EQ(1, objs.size());
  ASSERT_EQ(std::string("SimpleService"), objs[0]);
  qi::ObjectPtr obj = qi::createObject("SimpleService");
  ASSERT_TRUE(obj);
  ASSERT_EQ(2, obj->call<int>("addOne", 1).value());
  SimpleServiceProxy proxy(obj);
  //specialized call
  ASSERT_EQ(42, proxy.addOne(41));
  //specialized connect/disconnect
  qi::Promise<int> prom;
  qi::SignalBase::Link link = proxy.sig.connect(boost::bind(&notify, _1, boost::ref(prom)));
  obj->post("sig", 1);
  ASSERT_EQ(1, prom.future().value());
  proxy.sig.disconnect(link);
  obj->post("sig", 2);
  qi::os::msleep(300);
  ASSERT_EQ(1, prom.future().value());
  //specialized emit
  prom.reset();
  link = obj->connect("sig", boost::function<void(int)>(boost::bind(&notify, _1, boost::ref(prom))));
  proxy.sig(4);
  ASSERT_EQ(4, prom.future().value());
  obj->disconnect(link);
  proxy.sig(5);
  qi::os::msleep(300);
  ASSERT_EQ(4, prom.future().value());
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
