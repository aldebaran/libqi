/*
 *  Author(s):
 *  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
 *
 *  Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 */

#include <gtest/gtest.h>
#include <qi/os.hpp>
#include <qi/perf/measure.hpp>
#include <ka/scoped.hpp>

#ifdef __linux__
TEST(TestMeasure, TestNumFD)
#else
TEST(TestMeasure, DISABLED_TestNumFD)
#endif
{
#ifdef __linux__
  const int numFD = qi::measure::getNumFD();
  ASSERT_NE(numFD, -1);

  const std::string tmp = qi::os::mktmpdir() + std::string("test");
  auto _ = ka::scoped(qi::os::fopen(tmp.c_str(), "w"), [](FILE* f) {
    if (f)
      fclose(f);
  });
  ASSERT_EQ(numFD + 1, qi::measure::getNumFD());
#endif
}
