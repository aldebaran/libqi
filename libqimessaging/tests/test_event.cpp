/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>
#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qimessaging/object.hpp>

static int lastPayload = 0;
static qi::Promise<int> pPayload;
void onFire(const int& pl)
{
  pPayload.setValue(pl);
  lastPayload = pl;
}

TEST(TestObject, Simple)
{
  qi::ObjectBuilder ob;
  ob.advertiseEvent<void (*)(int)>("fire");
  qi::Object obj(ob.object());
  EXPECT_EQ(1U, obj.metaObject().signalMap().size());
  int linkId = obj.connect("fire", &onFire, 0);
  obj.emitEvent("fire", 42);
  EXPECT_TRUE(pPayload.future().wait(2000));
  EXPECT_EQ(42, lastPayload);
  pPayload.reset();
  obj.emitEvent("fire", 51);
  EXPECT_TRUE(pPayload.future().wait(2000));
  EXPECT_EQ(51, lastPayload);
  pPayload.reset();
  obj.disconnect(linkId);
  obj.emitEvent("fire", 42);
  EXPECT_FALSE(pPayload.future().wait(200));
  EXPECT_EQ(51, lastPayload);
}

TEST(TestObject, EmitMethod)
{
  lastPayload = 0;
  qi::ObjectBuilder ob;
  ob.advertiseMethod("fire", &onFire);
  qi::Object obj(ob.object());
  pPayload.reset();
  obj.emitEvent("fire", 23);
  EXPECT_TRUE(pPayload.future().wait(2000));
  EXPECT_EQ(23, pPayload.future().value());
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
