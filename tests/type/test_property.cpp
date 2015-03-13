/*
** Copyright (C) 2015 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/property.hpp>
#include <qi/signalspy.hpp>

qiLogCategory("qi.test.property");

TEST(TestProperty, SetDuringCreation)
{
  qi::Property<int> prop(4);
  ASSERT_EQ(4, prop.value().to<int>());
}

TEST(TestProperty, SignalOfProperty)
{
  qi::Property<int> prop(4);
  qi::SignalSpy spy(prop);
  prop.setValue(2);
  qi::os::sleep(1);
  ASSERT_EQ(2, prop.value().to<int>());
  ASSERT_EQ(1, spy.getCounter());
}
