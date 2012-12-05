 /*
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <gtest/gtest.h>

#include <qi/application.hpp>

#include <qitype/genericobject.hpp>
#include <qitype/objectfactory.hpp>

#include <simpleservice/simpleservice-proxy.hpp>

TEST(TestLoadService, Load)
{
  std::vector<std::string> objs = qi::loadObject("simpleservice");
  ASSERT_EQ(1, objs.size());
  ASSERT_EQ(std::string("SimpleService"), objs[0]);
  qi::ObjectPtr obj = qi::createObject("SimpleService");
  ASSERT_TRUE(obj);
  ASSERT_EQ(2, obj->call<int>("addOne", 1).value());
  SimpleServiceProxy proxy(obj);
  ASSERT_EQ(42, proxy.addOne(41));
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
