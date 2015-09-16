/*
 * Copyright (c) 2015 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>

#include <gtest/gtest.h>

#include <qi/clock.hpp>
#include <boost/chrono/chrono_io.hpp>

namespace chrono = boost::chrono;


template<class Clock>
void clock_output_()
{
  typename Clock::duration d(1);
  typename Clock::time_point t = Clock::now();
  std::cout << "name: " << boost::chrono::clock_string<Clock, char>::name() << ",\t"
            << "tick: " << d << ",\t"
            << "now: " << t << "\n";
  // same, using wide chars
  std::wcout << "name: " << boost::chrono::clock_string<Clock, wchar_t>::name() << ",\t"
             << "tick: " << d << ",\t"
             << "now: " << t << "\n";
}

TEST(QiClock, clock_output)
{
  clock_output_<qi::SteadyClock>();
  clock_output_<qi::Clock>();
  clock_output_<qi::SystemClock>();
}
