/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>

#include <gtest/gtest.h>

#include <qi/clock.hpp>
#include <qi/os.hpp>
#include <boost/chrono/chrono_io.hpp>

namespace chrono = boost::chrono;

//TEST(QiClock, clock_sleep)
//{
//  typedef chrono::duration<int64_t, boost::pico> picoseconds;
//  qi::sleepFor(chrono::milliseconds(1));
//  qi::sleepFor(chrono::milliseconds(1) + picoseconds(1));
//  qi::sleepFor(picoseconds(1));

//  qi::sleepUntil(qi::SteadyClock::now() + chrono::seconds(1));
//  qi::sleepUntil(qi::SteadyClock::now());
//  qi::sleepUntil(qi::SteadyClock::now() - chrono::seconds(1));

//  qi::sleepUntil(qi::WallClock::now() + chrono::seconds(1));
//  qi::sleepUntil(qi::WallClock::now());
//  qi::sleepUntil(qi::WallClock::now() - chrono::seconds(1));
//}

TEST(QiClock, clock_sleep_our)
{
  qi::sleepFor(qi::MilliSeconds(1));

  qi::sleepUntil(qi::SteadyClock::now() + qi::Seconds(1));
  qi::sleepUntil(qi::SteadyClock::now());
  qi::sleepUntil(qi::SteadyClock::now() - qi::Seconds(1));

  qi::sleepUntil(qi::WallClock::now() + qi::Seconds(1));
  qi::sleepUntil(qi::WallClock::now());
  qi::sleepUntil(qi::WallClock::now() - qi::Seconds(1));
}


template<class Clock>
void clock_output_()
{
  std::string s;
  chrono::milliseconds d_ms(500);
  typename Clock::duration d = d_ms;
  typename Clock::time_point t = Clock::now();
  std::cout << boost::chrono::clock_string<Clock, char>::name() << "\n"
             << d_ms << "\n" << d << "\n" << t << "\n" << t+d << "\n";
}

TEST(QiClock, clock_output)
{
  clock_output_<qi::SteadyClock>();
  clock_output_<qi::WallClock>();
}

typedef chrono::duration<uint32_t, boost::milli > uint32ms;

TEST(QiClock, tofromUint32ms)
{
  // a test to show how we can convert from 32-bits guess-what-my-epoch-is
  // time stamps to qi::SteadyClock::time_point
  qi::SteadyClock::duration period =
      qi::SteadyClock::duration(uint32ms::max())
      + qi::SteadyClock::duration(uint32ms(1));
  qi::SteadyClock::duration eps = chrono::milliseconds(4);

  // check sum-ms "noise" is removed
  qi::SteadyClock::duration noise = chrono::nanoseconds(654321);
  qi::SteadyClock::time_point t(period/4);
  uint32_t input_ms = chrono::duration_cast<uint32ms>(t.time_since_epoch()).count();
  EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t+noise, qi::SteadyClock::Expect_SoonerOrLater));
  EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t-noise, qi::SteadyClock::Expect_SoonerOrLater));

  // check we get the expected values
  for (int i = 0; i<8; ++i) {
    t = qi::SteadyClock::time_point((i*period)/4);
    input_ms = chrono::duration_cast<uint32ms>(t.time_since_epoch()).count();

    EXPECT_EQ(input_ms, qi::SteadyClock::toUint32ms(t));

    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t, qi::SteadyClock::Expect_SoonerOrLater));
    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t + eps, qi::SteadyClock::Expect_SoonerOrLater));
    EXPECT_FALSE(t == qi::SteadyClock::fromUint32ms(input_ms, t + period/2, qi::SteadyClock::Expect_SoonerOrLater));
    EXPECT_FALSE(t == qi::SteadyClock::fromUint32ms(input_ms, t + period/2 + eps, qi::SteadyClock::Expect_SoonerOrLater));
    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t - eps, qi::SteadyClock::Expect_SoonerOrLater));
    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t - period/2 + eps, qi::SteadyClock::Expect_SoonerOrLater));
    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t - period/2, qi::SteadyClock::Expect_SoonerOrLater));
    EXPECT_FALSE(t == qi::SteadyClock::fromUint32ms(input_ms, t - period/2 - eps, qi::SteadyClock::Expect_SoonerOrLater));

    EXPECT_FALSE(t == qi::SteadyClock::fromUint32ms(input_ms, t - eps, qi::SteadyClock::Expect_Sooner));
    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t, qi::SteadyClock::Expect_Sooner));
    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t + eps, qi::SteadyClock::Expect_Sooner));
    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t + period - eps, qi::SteadyClock::Expect_Sooner));
    EXPECT_FALSE(t == qi::SteadyClock::fromUint32ms(input_ms, t + period, qi::SteadyClock::Expect_Sooner));
    EXPECT_FALSE(t == qi::SteadyClock::fromUint32ms(input_ms, t + period + eps, qi::SteadyClock::Expect_Sooner));

    EXPECT_FALSE(t == qi::SteadyClock::fromUint32ms(input_ms, t - period - eps, qi::SteadyClock::Expect_Later));
    EXPECT_FALSE(t == qi::SteadyClock::fromUint32ms(input_ms, t - period, qi::SteadyClock::Expect_Later));
    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t - period + eps, qi::SteadyClock::Expect_Later));
    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t - eps, qi::SteadyClock::Expect_Later));
    EXPECT_TRUE(t == qi::SteadyClock::fromUint32ms(input_ms, t, qi::SteadyClock::Expect_Later));
    EXPECT_FALSE(t == qi::SteadyClock::fromUint32ms(input_ms, t + eps, qi::SteadyClock::Expect_Later));
  }
}

TEST(QiClock, clock_convert_two_int)
{
  // convert a time point to two ints, alvalue-like
  qi::SteadyClock::time_point t(chrono::seconds(123456789)
                                 + chrono::microseconds(123456)
                                 + chrono::nanoseconds(789));
  int seconds = static_cast<int>(t.time_since_epoch().count()/1000000000LL);
  int microseconds = static_cast<int>(
      (t.time_since_epoch().count()/1000LL) % 1000000LL);
  if (microseconds < 0) {
    microseconds += 1000000;
    --seconds;
  }
  EXPECT_EQ(123456789, seconds);
  EXPECT_EQ(123456, microseconds);

  // let do the contrary
  qi::SteadyClock::time_point t_back(chrono::seconds(seconds)
                              + chrono::microseconds(microseconds));
  EXPECT_TRUE(t - t_back < chrono::microseconds(1));
}
