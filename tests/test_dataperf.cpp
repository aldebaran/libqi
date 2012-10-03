/*
 *  Author(s):
 *  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <string>

#include <gtest/gtest.h>

#include <qiperf/dataperf.hpp>

TEST(TestMsgSize, TestDataPerf)
{
  qi::DataPerf dp;

  ASSERT_EQ(dp.getMsgSize(), (unsigned int)0);

  dp.start("My_Bench", 1, 1);
  ASSERT_EQ(dp.getMsgSize(), (unsigned int)1);
  dp.stop();

  dp.start("My_Bench_2");
  ASSERT_EQ(dp.getMsgSize(), (unsigned int)0);
  dp.stop();
}

