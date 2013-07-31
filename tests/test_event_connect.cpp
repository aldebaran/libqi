/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>
#include <gtest/gtest.h>
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qi/application.hpp>

static int lastPayload = 0;
static int lastPayload2 = 0;
static int completed = 0;

void onFire(const int& pl, qi::Promise<void> p)
{
  lastPayload = pl;
  p.setValue(0);
}
void onFire2(const int& pl, qi::Promise<void> p)
{
  lastPayload2 = pl;
  p.setValue(0);
}


void testDelete(bool afirst, bool disconnectFirst)
{
  qi::Promise<void> p0;
  qi::Promise<void> p1;
  qi::DynamicObjectBuilder oba, obb;
  qi::SignalLink fireId = oba.advertiseSignal<int>("fire");
  qi::SignalLink onFireId = obb.advertiseMethod("onFire", boost::bind<void>(&onFire, _1, p0));
  qi::SignalLink onFireId2 = obb.advertiseMethod("onFire2", boost::bind<void>(&onFire2, _1, p1));
  qi::AnyObject *a = new qi::AnyObject(oba.object());
  qi::AnyObject *b = new qi::AnyObject(obb.object());
  qi::SignalLink linkId = (*a)->connect(fireId, *b, onFireId);
  (*a)->connect(fireId, *b, onFireId2);
  //std::vector<qi::SignalSubscriber> subs = (*a)->subscribers(fireId);
  //EXPECT_EQ(static_cast<unsigned int>(2), subs.size());
  // Subs ordering is unspecified
  //EXPECT_EQ(subs[0].method + subs[1].method, onFireId + onFireId2);
  (*a)->post("fire", 12);
  EXPECT_TRUE(p0.future().hasValue(1000));
  EXPECT_TRUE(p1.future().hasValue(1000));
  EXPECT_EQ(12, lastPayload);
  EXPECT_EQ(12, lastPayload2);
  p0.reset();
  p1.reset();
  if (disconnectFirst)
  {
    (*a)->disconnect(linkId);
    (*a)->post("fire", 13);
    EXPECT_TRUE(p1.future().hasValue(1000));
    EXPECT_EQ(12, lastPayload);
    EXPECT_EQ(13, lastPayload2);
    p0.reset();
    p1.reset();
  }
  if (afirst)
  {
    delete a;
    delete b;
  }
  else
  {
    delete b;
    (*a)->post("fire", 12);
    delete a;
  }
  ++completed;
}

TEST(TestObject, Destruction)
{
  // Run test from object thread as they are synchronous
  for (int i=0; i<4; ++i)
    qi::getDefaultObjectEventLoop()->post(
      boost::bind(&testDelete, !!((int)i/2), !!((int)i%2)));
  while (completed < 4)
    qi::os::msleep(100);
  /*
  testDelete(false, false);
  testDelete(true, true);
  testDelete(false, true);
  testDelete(true, false);
  */
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
