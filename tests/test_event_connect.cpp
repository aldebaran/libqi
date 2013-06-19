/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>
#include <gtest/gtest.h>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qi/application.hpp>

static int lastPayload = 0;
static int lastPayload2 = 0;
static int completed = 0;
void onFire(const int& pl)
{
  lastPayload = pl;
}
void onFire2(const int& pl)
{
  lastPayload2 = pl;
}


void testDelete(bool afirst, bool disconnectFirst)
{
  qi::DynamicObjectBuilder oba, obb;
  qi::Link fireId = oba.advertiseSignal<int>("fire");
  qi::Link onFireId = obb.advertiseMethod("onFire", &onFire);
  qi::Link onFireId2 = obb.advertiseMethod("onFire2", &onFire2);
  qi::AnyObject *a = new qi::AnyObject(oba.object());
  qi::AnyObject *b = new qi::AnyObject(obb.object());
  qi::Link linkId = (*a)->connect(fireId, *b, onFireId);
  (*a)->connect(fireId, *b, onFireId2);
  //std::vector<qi::SignalSubscriber> subs = (*a)->subscribers(fireId);
  //EXPECT_EQ(static_cast<unsigned int>(2), subs.size());
  // Subs ordering is unspecified
  //EXPECT_EQ(subs[0].method + subs[1].method, onFireId + onFireId2);
  (*a)->post("fire", 12);
  EXPECT_EQ(12, lastPayload);
  EXPECT_EQ(12, lastPayload2);
  if (disconnectFirst)
  {
    (*a)->disconnect(linkId);
    (*a)->post("fire", 13);
    EXPECT_EQ(12, lastPayload);
    EXPECT_EQ(13, lastPayload2);
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
