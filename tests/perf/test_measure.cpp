/*
 *  Author(s):
 *  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
 *
 *  Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 */

#include <gtest/gtest.h>

#include <qiperf/measure.hpp>
#include <qi/os.hpp>
TEST(TestMeasure, TestNumFD)
{
  int numFD = qi::measure::getNumFD();
#ifdef __linux__
  ASSERT_NE(numFD, -1);
#else
  ASSERT_EQ(numFD, -1);
#endif

  std::string tmp = qi::os::mktmpdir();
  tmp += "test";
  qi::os::fopen(tmp.c_str(), "w");
#ifdef __linux__
  ASSERT_EQ(qi::measure::getNumFD(), numFD + 1);
#else
  ASSERT_EQ(qi::measure::getNumFD(), -1);
#endif
}
