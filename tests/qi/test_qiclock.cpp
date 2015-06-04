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


TEST(QiClock, initialization)
{
  qi::Duration d0;
  EXPECT_EQ(0, d0.count());

  qi::Duration d1 = qi::Duration::zero();
  EXPECT_EQ(0, d1.count());

  qi::Duration d2 = qi::Duration();
  EXPECT_EQ(0, d2.count());

  qi::Duration d3(0);
  EXPECT_EQ(0, d3.count());

  qi::Clock::time_point t0;
  EXPECT_EQ(0, t0.time_since_epoch().count());

  qi::Clock::time_point t1(qi::Duration::zero());
  EXPECT_EQ(0, t1.time_since_epoch().count());

  qi::Clock::time_point t2 = qi::Clock::time_point();
  EXPECT_EQ(0, t2.time_since_epoch().count());
}

//TEST(QiClock, clock_sleep)
//{
//  typedef chrono::duration<int64_t, boost::pico> picoseconds;
//  qi::sleepFor(chrono::milliseconds(1));
//  qi::sleepFor(chrono::milliseconds(1) + picoseconds(1));
//  qi::sleepFor(picoseconds(1));

//  qi::sleepUntil(qi::SteadyClock::now() + chrono::seconds(1));
//  qi::sleepUntil(qi::SteadyClock::now());
//  qi::sleepUntil(qi::SteadyClock::now() - chrono::seconds(1));

//  qi::sleepUntil(qi::SystemClock::now() + chrono::seconds(1));
//  qi::sleepUntil(qi::SystemClock::now());
//  qi::sleepUntil(qi::SystemClock::now() - chrono::seconds(1));
//}

TEST(QiClock, clock_sleep_our)
{
  qi::sleepFor(qi::MilliSeconds(1));

  qi::sleepUntil(qi::SteadyClock::now() + qi::Seconds(1));
  qi::sleepUntil(qi::SteadyClock::now());
  qi::sleepUntil(qi::SteadyClock::now() - qi::Seconds(1));

  qi::sleepUntil(qi::Clock::now() + qi::Seconds(1));
  qi::sleepUntil(qi::Clock::now());
  qi::sleepUntil(qi::Clock::now() - qi::Seconds(1));

  qi::sleepUntil(qi::SystemClock::now() + qi::Seconds(1));
  qi::sleepUntil(qi::SystemClock::now());
  qi::sleepUntil(qi::SystemClock::now() - qi::Seconds(1));
}

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

typedef chrono::duration<uint32_t, boost::milli > uint32ms;

TEST(QiClock, tofromUint32ms)
{
  // a test to show how we can convert from 32-bits guess-what-my-epoch-is
  // time stamps to qi::SteadyClock::time_point
  qi::Clock::duration period =
      qi::Clock::duration(uint32ms::max()) + qi::Clock::duration(uint32ms(1));
  qi::Clock::duration eps = chrono::milliseconds(4);

  // check sum-ms "noise" is removed
  qi::Clock::duration noise = chrono::nanoseconds(654321);
  qi::Clock::time_point t(period/4);
  uint32_t input_ms = chrono::duration_cast<uint32ms>(t.time_since_epoch()).count();
  EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t+noise, qi::Clock::Expect_SoonerOrLater));
  EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t-noise, qi::Clock::Expect_SoonerOrLater));

  // check we get the expected values
  for (int i = 0; i<8; ++i) {
    t = qi::Clock::time_point((i*period)/4);
    input_ms = chrono::duration_cast<uint32ms>(t.time_since_epoch()).count();

    EXPECT_EQ(input_ms, qi::Clock::toUint32ms(t));

    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t, qi::Clock::Expect_SoonerOrLater));
    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t + eps, qi::Clock::Expect_SoonerOrLater));
    EXPECT_FALSE(t == qi::Clock::fromUint32ms(input_ms, t + period/2, qi::Clock::Expect_SoonerOrLater));
    EXPECT_FALSE(t == qi::Clock::fromUint32ms(input_ms, t + period/2 + eps, qi::Clock::Expect_SoonerOrLater));
    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t - eps, qi::Clock::Expect_SoonerOrLater));
    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t - period/2 + eps, qi::Clock::Expect_SoonerOrLater));
    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t - period/2, qi::Clock::Expect_SoonerOrLater));
    EXPECT_FALSE(t == qi::Clock::fromUint32ms(input_ms, t - period/2 - eps, qi::Clock::Expect_SoonerOrLater));

    EXPECT_FALSE(t == qi::Clock::fromUint32ms(input_ms, t - eps, qi::Clock::Expect_Sooner));
    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t, qi::Clock::Expect_Sooner));
    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t + eps, qi::Clock::Expect_Sooner));
    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t + period - eps, qi::Clock::Expect_Sooner));
    EXPECT_FALSE(t == qi::Clock::fromUint32ms(input_ms, t + period, qi::Clock::Expect_Sooner));
    EXPECT_FALSE(t == qi::Clock::fromUint32ms(input_ms, t + period + eps, qi::Clock::Expect_Sooner));

    EXPECT_FALSE(t == qi::Clock::fromUint32ms(input_ms, t - period - eps, qi::Clock::Expect_Later));
    EXPECT_FALSE(t == qi::Clock::fromUint32ms(input_ms, t - period, qi::Clock::Expect_Later));
    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t - period + eps, qi::Clock::Expect_Later));
    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t - eps, qi::Clock::Expect_Later));
    EXPECT_TRUE(t == qi::Clock::fromUint32ms(input_ms, t, qi::Clock::Expect_Later));
    EXPECT_FALSE(t == qi::Clock::fromUint32ms(input_ms, t + eps, qi::Clock::Expect_Later));
  }
}


TEST(QiClock, toIso8601String)
{
  qi::SystemClockTimePoint t0(qi::Duration(0));
  EXPECT_EQ("1970-01-01T000000.000Z", qi::toISO8601String(t0));
  // we do not round, we ceil:
  EXPECT_EQ("1970-01-01T000000.000Z", qi::toISO8601String(t0 + qi::MicroSeconds(999)));
  EXPECT_EQ("1970-01-01T000001.042Z", qi::toISO8601String(t0 + qi::MilliSeconds(1042)));
  EXPECT_EQ("1970-02-01T010203.000Z", qi::toISO8601String(t0 + qi::Hours(24*31+1)+qi::Minutes(2)+qi::Seconds(3)));
}


TEST(QiClock, durationSince)
{
  qi::SteadyClock::time_point tp = qi::SteadyClock::now();

  const qi::Duration sleepDuration = qi::MilliSeconds(500);
  const qi::MilliSeconds sleepDurationMs = boost::chrono::duration_cast<qi::MilliSeconds>(sleepDuration);
  const qi::MicroSeconds sleepDurationUs = boost::chrono::duration_cast<qi::MicroSeconds>(sleepDuration);

  qi::sleepFor(sleepDuration);
  const qi::MilliSeconds durMs = qi::durationSince<qi::MilliSeconds>(tp);
  const qi::MicroSeconds durUs = qi::durationSince<qi::MicroSeconds>(tp);
  const qi::Duration tol = qi::MilliSeconds(1); // only needed on Windows
  EXPECT_GE(durMs + tol, sleepDurationMs);
  EXPECT_GE(durUs + tol, sleepDurationUs);
}
