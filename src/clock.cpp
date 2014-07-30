/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/clock.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>
#include <boost/thread.hpp>
#include <boost/chrono/ceil.hpp>
#ifndef BOOST_THREAD_USES_CHRONO
// needed for boost < 1.50
#include <boost/thread/thread_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#endif
qiLogCategory("qi.clock");

namespace chrono = boost::chrono;

namespace qi {

  SteadyClockTimePoint SteadyClock::now()
  {
    return SteadyClockTimePoint(chrono::steady_clock::now().time_since_epoch());
  }

  typedef chrono::duration<uint32_t, boost::milli > uint32ms;

  uint32_t SteadyClock::toUint32ms(const SteadyClockTimePoint &t) throw()
  {
    return chrono::duration_cast<uint32ms>(t.time_since_epoch()).count();
  }

  int32_t SteadyClock::toInt32ms(const SteadyClockTimePoint &t) throw()
  {
    return static_cast<int32_t>(toUint32ms(t));
  }

  SteadyClockTimePoint SteadyClock::fromUint32ms(uint32_t t_ms,
                                                 SteadyClockTimePoint guess,
                                                 Expect expect) throw()
  {
    // ms: a duration type with ms precision, but no overflow problem
    typedef chrono::milliseconds ms;
    typedef chrono::time_point<SteadyClock, ms> time_point_ms;
    // overflow period
    static const ms period(ms(uint32ms::max()) - ms(uint32ms::min()) + ms(uint32ms(1)));
    uint32ms guess_ms = chrono::duration_cast<uint32ms>(guess.time_since_epoch());
    // convert to ms intead of using SteadyClock::duration to avoid sub-ms noise
    time_point_ms origin(chrono::time_point_cast<ms>(guess)
                         - chrono::duration_cast<ms>(guess_ms));

    static const uint32_t half_period = uint32ms(period/2).count();
    switch (expect)
    {
    case Expect_SoonerOrLater: // guess - period/2 < result <= guess + period/2
      if (t_ms > guess_ms.count() && t_ms - guess_ms.count() > half_period)
        origin -= period;
      else if (t_ms < guess_ms.count() && guess_ms.count() - t_ms >= half_period)
        origin += period;
      break;
    case Expect_Later: // guess <= result < guess + period
      if (guess_ms.count() > t_ms)
        // the 32 bits clock has overflown between guess and t
        origin += period;
      break;
    case Expect_Sooner: // guess - period < result <= guess
      if (t_ms > guess_ms.count())
        // the 32 bits clock has overflown between t and guess
        origin -= period;
      break;
    }
    return origin + uint32ms(t_ms);
  }

  SteadyClockTimePoint SteadyClock::fromInt32ms(int32_t t_ms,
                                                SteadyClockTimePoint guess,
                                                Expect expect) throw()
  {
    return SteadyClock::fromUint32ms(static_cast<uint32_t>(t_ms), guess, expect);
  }

  WallClockTimePoint WallClock::now()
  {
    return WallClockTimePoint(
        chrono::system_clock::now().time_since_epoch());
  }

  std::time_t WallClock::to_time_t(const WallClockTimePoint &t) throw()
  {
    return chrono::system_clock::to_time_t(
        chrono::system_clock::time_point(
            chrono::duration_cast<chrono::system_clock::duration>(
                t.time_since_epoch())));
  }

  WallClockTimePoint WallClock::from_time_t(const std::time_t &t) throw()
  {
    return WallClockTimePoint(
        chrono::system_clock::from_time_t(t).time_since_epoch());
  }

  void sleepFor(const SteadyClock::duration &d)
  {
#ifdef BOOST_THREAD_USES_CHRONO
    boost::this_thread::sleep_for(d);
#else
# ifdef BOOST_DATE_TIME_HAS_NANOSECONDS
    boost::this_thread::sleep(boost::posix_time::nanoseconds(d.count()));
# else
    typedef chrono::microseconds us;
    boost::this_thread::sleep(
          boost::posix_time::microseconds(chrono::ceil<us>(d).count()));
# endif
#endif
  }

  void sleepUntil(const WallClockTimePoint &t)
  {
#ifdef BOOST_THREAD_USES_CHRONO
    boost::this_thread::sleep_until(t);
#else
    static const boost::gregorian::date epoch(1970, boost::date_time::Jan, 1);
# ifdef BOOST_DATE_TIME_HAS_NANOSECONDS
    boost::this_thread::sleep(boost::system_time(epoch,
      boost::posix_time::nanoseconds(t.time_since_epoch().count())));
# else
    typedef chrono::microseconds us;
    boost::this_thread::sleep(boost::system_time(epoch,
      boost::posix_time::microseconds(
        chrono::ceil<us>(t.time_since_epoch()).count())));
# endif
#endif
  }

  void sleepUntil(const SteadyClockTimePoint& t)
  {
    sleepFor(t - SteadyClock::now());
  }
}
