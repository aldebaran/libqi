/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#pragma once
#ifndef _QI_CLOCK_HXX_
#define _QI_CLOCK_HXX_

#include <boost/chrono/ceil.hpp>
#include <sstream>

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
    using ClockFrom = typename TimePointFrom::clock;
    return boost::chrono::duration_cast<DurationTo>(ClockFrom::now() - t);
  }

  namespace detail
  {
    template <class Period>
    void write_duration_unit_long_name(std::ostream &os);

    template <>
    inline void write_duration_unit_long_name<boost::nano>(std::ostream &os) {
      static const char a[] = {'n', 'a', 'n', 'o', 's', 'e', 'c', 'o', 'n', 'd', 's'};
      os.write(a, sizeof(a)/sizeof(char));
    }

    template <>
    inline void write_duration_unit_long_name<boost::micro>(std::ostream &os) {
      static const char a[] = {'m', 'i', 'c', 'r', 'o', 's', 'e', 'c', 'o', 'n', 'd', 's'};
      os.write(a, sizeof(a)/sizeof(char));
    }

    template <>
    inline void write_duration_unit_long_name<boost::milli>(std::ostream &os) {
      static const char a[] = {'m', 'i', 'l', 'l', 'i', 's', 'e', 'c', 'o', 'n', 'd', 's'};
      os.write(a, sizeof(a)/sizeof(char));
    }

    template <>
    inline void write_duration_unit_long_name<boost::ratio<1, 1>>(std::ostream &os) {
      static const char a[] = {'s', 'e', 'c', 'o', 'n', 'd', 's'};
      os.write(a, sizeof(a)/sizeof(char));
    }

    template <>
    inline void write_duration_unit_long_name<boost::ratio<60, 1>>(std::ostream &os) {
      static const char a[] = {'m', 'i', 'n', 'u', 't', 'e', 's'};
      os.write(a, sizeof(a)/sizeof(char));
    }

    template <>
    inline void write_duration_unit_long_name<boost::ratio<3600, 1>>(std::ostream &os) {
      static const char a[] = {'h', 'o', 'u', 'r', 's'};
      os.write(a, sizeof(a)/sizeof(char));
    }
  }

  template <class R, class P>
  inline std::string to_string(const boost::chrono::duration<R, P> &d)
  {
    std::ostringstream os;
    os << d.count() << ' ';
    detail::write_duration_unit_long_name<P>(os);
    return os.str();
  }

  template <class C, class D>
  inline std::string to_string(const boost::chrono::time_point<C, D> &t)
  {
    std::ostringstream os;
    os << t.time_since_epoch().count() << ' ';
    detail::write_duration_unit_long_name<typename D::period>(os);
    os << boost::chrono::clock_string<C, char>::since();
    return os.str();
  }
}

namespace boost
{
  namespace chrono
  {
    // helpers for boost chrono io (v1)
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
