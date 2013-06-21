/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>
#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>

static int lastPayload = 0;
static qi::Promise<int> pPayload;
void onFire(const int& pl)
{
  pPayload.setValue(pl);
  lastPayload = pl;
}

TEST(TestObject, Simple)
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseSignal<int>("fire");
  qi::AnyObject obj(ob.object());
  EXPECT_LE(1U, obj->metaObject().signalMap().size());
  qi::SignalLink linkId = obj->connect("fire", &onFire);
  obj->post("fire", 42);
  EXPECT_TRUE(pPayload.future().wait(2000) != qi::FutureState_Running);
  EXPECT_EQ(42, lastPayload);
  pPayload.reset();
  obj->post("fire", 51);
  EXPECT_TRUE(pPayload.future().wait(2000) != qi::FutureState_Running);
  EXPECT_EQ(51, lastPayload);
  pPayload.reset();
  obj->disconnect(linkId);
  obj->post("fire", 42);
  EXPECT_FALSE(pPayload.future().wait(200) != qi::FutureState_Running);
  EXPECT_EQ(51, lastPayload);
}

void readString(const std::string&)
{
}

TEST(TestObject, ConnectBind)
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseSignal<int>("fire");
  ob.advertiseSignal<int, int>("fire2");
  qi::AnyObject obj(ob.object());
  qi::SignalLink link = obj->connect("fire", boost::bind<void>(&onFire, _1));
  obj->post("fire", 42);
  EXPECT_TRUE(pPayload.future().wait(2000) != qi::FutureState_Running);
  EXPECT_EQ(42, lastPayload);
  obj->disconnect(link);
  // The boost bind without _1 gives us a void (void) signature that does not match fire
  EXPECT_EQ(
    qi::SignalBase::invalidSignalLink,
    obj->connect("fire", boost::bind<void>(&onFire, 51)).value()
  );
  // Argument type mismatch
  EXPECT_EQ(
    qi::SignalBase::invalidSignalLink,
    obj->connect("fire", boost::bind<void>(&readString, _1)).value()
  );
  link = obj->connect("fire2", boost::bind(&onFire, _2));
  EXPECT_TRUE(link != 0);
  pPayload.reset();
  obj->post("fire2", 40, 41);
  EXPECT_TRUE(pPayload.future().wait(2000) != qi::FutureState_Running);
  EXPECT_EQ(41, lastPayload);
  obj->disconnect(link);
}

TEST(TestObject, EmitMethod)
{
  lastPayload = 0;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("fire", &onFire);
  qi::AnyObject obj(ob.object());
  pPayload.reset();
  obj->post("fire", 23);
  EXPECT_TRUE(pPayload.future().wait(2000) != qi::FutureState_Running);
  EXPECT_EQ(23, pPayload.future().value());
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
