/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>
#include <gtest/gtest.h>
#include <qimessaging/object.hpp>

static int lastPayload = 0;
static int lastPayload2 = 0;
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
  qi::Object& a = *new qi::Object();
  qi::Object& b = *new qi::Object();
  unsigned int fireId = a.advertiseEvent<void (*)(int)>("fire");
  unsigned int onFireId = b.advertiseMethod("onFire", &onFire);
  unsigned int onFireId2 = b.advertiseMethod("onFire2", &onFire2);
  unsigned int linkId = a.connect(fireId, &b, onFireId);
  unsigned int linkId2 = a.connect(fireId, &b, onFireId2);
  std::vector<qi::MetaEvent::Subscriber> subs = a.metaObject().event(fireId)->subscribers();
  EXPECT_EQ(2, subs.size());
  // Subs ordering is unspecified
  EXPECT_EQ(subs[0].method + subs[1].method, onFireId + onFireId2);
  a.emitEvent("fire", 12);
  EXPECT_EQ(12, lastPayload);
  EXPECT_EQ(12, lastPayload2);
  if (disconnectFirst)
  {
    a.disconnect(linkId);
    a.emitEvent("fire", 13);
    EXPECT_EQ(12, lastPayload);
    EXPECT_EQ(13, lastPayload2);
  }
  if (afirst)
  {
    delete &a;
    delete &b;
  }
  else
  {
    delete &b;
    a.emitEvent("fire", 12);
    delete &a;
  }
}

TEST(TestObject, Destruction)
{ 
  testDelete(false, false);
  testDelete(true, true);
  testDelete(false, true);
  testDelete(true, false);
}
