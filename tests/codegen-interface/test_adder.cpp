#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qitype/objectfactory.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <testsession/testsessionpair.hpp>

using namespace qi;
TEST(Interface, Adder0)
{
  TestSessionPair p;
  p.server()->loadService("adder_interface");
  // In case of multiple tests, not sure if addService worked.
  p.server()->registerService("a0s", createObject("Adder0Service"));
  p.server()->registerService("a1s", createObject("Adder1Service"));
  ObjectPtr a0 = p.client()->service("a0s").value()->call<ObjectPtr>("create");
  ObjectPtr a1 = p.client()->service("a1s").value()->call<ObjectPtr>("create");
  ASSERT_TRUE(a0);
  ASSERT_TRUE(a1);
  a0->setProperty("value", 1);
  ASSERT_EQ(4, a0->call<int>("addTwo", 0, 3));
  a0->setProperty("value", -1);
  ASSERT_EQ(2, a0->call<int>("addTwo", 0, 3));
  // a1 has a setter that prevents sets below 0
  a1->setProperty("value", 1);
  ASSERT_EQ(4, a1->call<int>("addTwo", 0, 3));
  a1->setProperty("value", -1);
  ASSERT_EQ(1, a1->property<int>("value").value());
  ASSERT_EQ(4, a1->call<int>("addTwo", 0, 3));
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
