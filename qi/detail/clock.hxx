/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#pragma once
#ifndef _QI_CLOCK_HXX_
#define _QI_CLOCK_HXX_

#include <boost/chrono/ceil.hpp>

namespace qi {

  template <class Rep, class Period>
  void sleepFor(const boost::chrono::duration<Rep, Period>& d)
  {
    sleepFor(boost::chrono::ceil<SteadyClock::duration>(d));
  }

  template <class Duration>
  void sleepUntil(const boost::chrono::time_point<SteadyClock, Duration>& t)
  {
    sleepFor(t - SteadyClock::now());
  }

  template <class Duration>
  void sleepUntil(const boost::chrono::time_point<Clock, Duration>& t)
  {
    sleepFor(t - Clock::now());
  }

  template <class Duration>
  void sleepUntil(const boost::chrono::time_point<SystemClock, Duration>& t)
  {
    sleepFor(t - SystemClock::now());
  }

  template <class DurationTo, class TimePointFrom>
  DurationTo durationSince(const TimePointFrom& t)
  {
    typedef typename TimePointFrom::clock ClockFrom;
    return boost::chrono::duration_cast<DurationTo>(ClockFrom::now() - t);
  }
}

namespace boost
{
  namespace chrono
  {
    template <class CharT>
    struct clock_string<qi::SteadyClock, CharT>
    {
      static std::basic_string<CharT> name() {
        static const CharT a[] = {'q', 'i', ':', ':', 'S', 't', 'e', 'a', 'd', 'y', 'C', 'l', 'o', 'c', 'k'};
        return std::basic_string<CharT>(a, a + sizeof(a)/sizeof(a[0]));
      }
      static std::basic_string<CharT> since() {
        static const CharT a[] = {' ', 's', 'i', 'n', 'c', 'e', ' ', 'p', 'r', 'o', 'g', 'r', 'a', 'm', ' ', 's', 't', 'a', 'r', 't'};
        return std::basic_string<CharT>(a, a + sizeof(a)/sizeof(a[0]));
      }
    };

    template <class CharT>
    struct clock_string<qi::Clock, CharT>
    {
      static std::basic_string<CharT> name() {
        static const CharT a[] = {'q', 'i', ':', ':', 'C', 'l', 'o', 'c', 'k'};
        return std::basic_string<CharT>(a, a + sizeof(a)/sizeof(a[0]));
      }
      static std::basic_string<CharT> since() {
        return clock_string<boost::chrono::steady_clock, CharT>::since();
      }
    };

    template <class CharT>
    struct clock_string<qi::SystemClock, CharT>
    {
      static std::basic_string<CharT> name() {
        static const CharT a[] = {'q', 'i', ':', ':', 'S', 'y', 's', 't', 'e', 'm', 'C', 'l', 'o', 'c', 'k'};
        return std::basic_string<CharT>(a, a + sizeof(a)/sizeof(a[0]));
      }
      static std::basic_string<CharT> since() {
        return clock_string<boost::chrono::system_clock, CharT>::since();
      }
    };
  }
}

#endif  // _QI_CLOCK_HXX_
