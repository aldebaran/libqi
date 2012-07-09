/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>
#include <gtest/gtest.h>
#include <qimessaging/object.hpp>

static int lastPayload = 0;

void onFire(const int& pl)
{
  lastPayload = pl;
}

TEST(TestObject, Simple)
{
  qi::Object obj;
  obj.advertiseEvent<void (*)(int)>("fire");
  EXPECT_EQ(1U, obj.metaObject().events().size());
  int linkId = obj.connect("fire", &onFire);
  obj.emitEvent("fire", 42);
  EXPECT_EQ(42, lastPayload);
  obj.emitEvent("fire", 51);
  EXPECT_EQ(51, lastPayload);
  obj.disconnect(linkId);
  obj.emitEvent("fire", 42);
  EXPECT_EQ(51, lastPayload);
}
