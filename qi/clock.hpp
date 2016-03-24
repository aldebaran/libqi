/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#pragma once
#ifndef _QI_CLOCK_HPP_
# define _QI_CLOCK_HPP_

# include <qi/api.hpp>
# include <qi/types.hpp>
# include <boost/chrono/chrono.hpp>

namespace qi
{
  /// Convenience typedefs

  template<class Rep, class Ratio>
  using DurationType = boost::chrono::duration<Rep, Ratio>;

  template<class ClockType>
  using TimePoint = boost::chrono::time_point<ClockType>;

  using NanoSeconds = DurationType<int64_t, boost::nano>;
  using MicroSeconds = DurationType<int64_t, boost::micro>;
  using MilliSeconds = DurationType<int64_t, boost::milli>;
  using Seconds = DurationType<int64_t, boost::ratio<1>>;
  using Minutes = DurationType<int64_t, boost::ratio<60>>;
  using Hours = DurationType<int64_t, boost::ratio<3600>>;

  using Duration = NanoSeconds;

  /**
   * \brief The SteadyClock class represents a monotonic clock.
   * \includename{qi/clock.hpp}
   *
   * The time points of this clock cannot decrease as physical
   * time moves forward. This clock is not related to wall clock time,
   * and is best suitable for measuring intervals.
   */
  class QI_API SteadyClock
  {
  public:
    using rep = Duration::rep;       ///< The representation type of the duration and time_point.
    using period = Duration::period; ///< The tick period of the clock in nanoseconds.
    using duration = Duration;       ///< The duration type of the clock.
    /**
     * The time_point type of the clock. Different clocks are permitted
     * to share a time_point definition if it is valid to compare their
     * time_points by comparing their respective durations.
     */
    using time_point = TimePoint<SteadyClock>;

    /**
     * true if t1 <= t2 is always true, else false.
     * \note A clock that can be adjusted backwards is not steady.
     */
    BOOST_STATIC_CONSTEXPR bool is_steady = boost::chrono::steady_clock::is_steady;

  public:
    QI_API_DEPRECATED
    typedef time_point SteadyClockTimePoint;

     /// \brief Enum expected argument
    enum Expect {
      Expect_SoonerOrLater, ///< Pick the nearest result to user-provided reference.
      Expect_Later,         ///< Result is expected to be later than user-provided reference.
      Expect_Sooner         ///< Result is expected to be sooner than user-provided reference.
    };

    /// Returns a time_point representing the current value of the clock.
    static time_point now();
  };


  /**
   * \brief The Clock class represents a system-wide clock, best suitable for
   * timestamping events. Typically monotonic and unaffected by the system clock
   * adjustment, altough this is not guaranteed.
   *
   * \includename{qi/clock.hpp}
   *
   */
  class QI_API Clock
  {
  public:
    using rep = Duration::rep;       ///< The representation type of the duration and time_point.
    using period = Duration::period; ///< The tick period of the clock in nanoseconds.
    using duration = Duration;       ///< The duration type of the clock.
    /**
     * The time_point type of the clock. Different clocks are permitted
     * to share a time_point definition if it is valid to compare their
     * time_points by comparing their respective durations.
     */
    using time_point = boost::chrono::time_point<Clock>;

    /**
     * true if t1 <= t2 is always true, else false.
     * \note A clock that can be adjusted backwards is not steady.
     */
    BOOST_STATIC_CONSTEXPR bool is_steady = boost::chrono::steady_clock::is_steady;

  public:
     /// \brief Enum expected argument
    enum Expect {
      Expect_SoonerOrLater, ///< Pick the nearest result to user-provided reference.
      Expect_Later,         ///< Result is expected to be later than user-provided reference.
      Expect_Sooner         ///< Result is expected to be sooner than user-provided reference.
    };

    /// Returns a time_point representing the current value of the clock.
    static time_point now();

    /**
     * \brief Convert the time point to a number of milliseconds on 32 bits.
     *
     * Since the 32 bits number overflows every 2^32 ms ~ 50 days,
     * this is a lossy operation.
     * \param t The time point to convert.
     * \return Unsigned int representing the time.
     */
    static uint32_t toUint32ms(const time_point &t) throw();
    /**
     * \brief Convert the time point to a number of milliseconds on 32 bits.
     *
     * Since the 32 bits number overflows every 2^32 ms ~ 50 days,
     * this is a lossy operation.
     * \param t The time point to convert.
     * \return Integer (int) representing the time.
     */
    static int32_t toInt32ms(const time_point &t) throw();

    /**
     * \brief Get a time point from a number of milliseconds on 32 bits.
     *
     * Since the 32 bits number overflows every ~50 days, an infinity of
     * time points match a given 32 bits number (all modulo ~50 days).
     * This function picks the result near the guess timepoint depending on
     * the expect argument:
     *
     *   - if expect == LATER, result is expected to be later than guess:
     *     guess <= result < guess + period
     *
     *   - if expect == SOONER, result is expected to be sooner than guess:
     *     guess - period < result <= guess
     *
     *   - if expect == SOONER_OR_LATER, pick the nearest result:
     *     guess - period/2 < result <= guess + period/2
     *
     * where period == 2^32 ms ~ 50 days
     */
    static time_point fromUint32ms(uint32_t t_ms, time_point guess,
                                   Expect expect=Expect_SoonerOrLater) throw();
    /// \copydoc Clock::fromUint32ms
    static time_point fromInt32ms(int32_t t_ms, time_point guess,
                                  Expect expect=Expect_SoonerOrLater) throw();
  };

  /**
   * \brief The SystemClock class represents the system-wide real time wall clock.
   *        It may not be monotonic: on most systems, the system time can be adjusted
   *        at any moment.
   *
   * \includename{qi/clock.hpp}
   */
  class QI_API SystemClock
  {
  public:
    using rep = Duration::rep;       ///< The representation type of the duration and time_point.
    using period = Duration::period; ///< The tick period of the clock in nanoseconds.
    using duration = Duration;       ///< The duration type of the clock.
    /**
     * The time_point type of the clock. Different clocks are permitted
     * to share a time_point definition if it is valid to compare their
     * time_points by comparing their respective durations.
     */
    using time_point = boost::chrono::time_point<SystemClock>;

    /// true if t1 <= t2 is always true, else false.
    /// \note A SystemClock is never steady.
    BOOST_STATIC_CONSTEXPR bool is_steady = false;

  public:

    QI_API_DEPRECATED
    typedef time_point WallClockTimePoint;

    /// Returns a time_point representing the current value of the clock.
    static time_point now();

    /**
     * \brief Converts a system clock time point to std::time_t
     * \param t Time point to convert.
     * \return A std::time_t representing \p t.
     */
    static std::time_t to_time_t(const time_point& t) throw();

    // Converts std::time_t to a system clock time point
    /**
     * \brief Converts std::time_t to a system clock time point
     * \param t std::time to convert.
     * \return A time point representing \p t.
     */
    static time_point from_time_t(const std::time_t &t) throw();
  };

  QI_API_DEPRECATED
  typedef SystemClock WallClock;

  using SteadyClockTimePoint = SteadyClock::time_point; ///< Steady clock time point.
  using ClockTimePoint = Clock::time_point; ///< qi::Clock time point.
  using SystemClockTimePoint = SystemClock::time_point; ///< System clock time point.

  QI_API_DEPRECATED
  typedef SystemClockTimePoint WallClockTimePoint; ///< System clock time point.

  /// \copydoc SteadyClock::now()
  QI_API_DEPRECATED
  inline QI_API SteadyClockTimePoint steadyClockNow() {
    return SteadyClock::now();
  }

  /// \copydoc SystemClock::now()
  QI_API_DEPRECATED
  inline QI_API SystemClockTimePoint wallClockNow() {
    return SystemClock::now();
  }

  /// @{
  /// Blocks the execution of the current thread for at least \p d.
  QI_API void sleepFor(const qi::Duration& d);
  template <class Rep, class Period>
  inline void sleepFor(const DurationType<Rep, Period>& d);
  /// @}

  /// @{
  /// \brief Blocks the execution of the current thread until \p t has been reached.
  ///
  /// This is equivalent to sleepFor(t - SteadyClockTimePoint::now())
  QI_API void sleepUntil(const SteadyClockTimePoint &t);
  template <class Duration>
  inline void sleepUntil(const boost::chrono::time_point<SteadyClock, Duration>& t);
  /// @}

  /// @{
  /// \brief Blocks the execution of the current thread until \p t has been reached.
  QI_API void sleepUntil(const ClockTimePoint &t);
  template <class Duration>
  inline void sleepUntil(const boost::chrono::time_point<Clock, Duration>& t);
  /// @}

  /// @{
  /// \brief Blocks the execution of the current thread until \p t has been reached.
  ///
  /// Adjustments of the clock are taken into account.
  /// Thus the duration of the block might, but might not, be less
  /// or more than t - SystemClock::now()
  QI_API void sleepUntil(const SystemClockTimePoint& t);
  template <class Duration>
  inline void sleepUntil(const boost::chrono::time_point<SystemClock, Duration>& t);
  /// @}

  /// @{
  /// \brief Return the date and time as a string in ISO 8601 format.
  /// The time is given up to millisecond precision, in UTC.
  /// The format does not include colon characters, to be suitable for
  /// inclusion in filenames on any filesystem.
  ///
  /// For instance the string for a quarter past nine PM on April 3rd, 2001 is
  /// "2001-04-03T211500.000Z"
  QI_API std::string toISO8601String(const SystemClockTimePoint &t);
  /// }@

  /// @{
  /// \brief Returns the duration elapsed since \p t.
  template <class DurationTo, class TimePointFrom>
  inline DurationTo durationSince(const TimePointFrom& t);
  /// @}

  template <class R, class P>
  inline std::string to_string(const DurationType<R, P> &d);

  template <class C, class D>
  inline std::string to_string(const boost::chrono::time_point<C, D> &t);
}

# ifdef __APPLE__
  // export template instanciation for RTTI issues across libraries. (mostly for OSX)
  template class QI_API boost::chrono::duration<int64_t, boost::nano>;
  template class QI_API boost::chrono::duration<int64_t, boost::micro>;
  template class QI_API boost::chrono::duration<int64_t, boost::milli>;
  template class QI_API boost::chrono::duration<int64_t>;
  template class QI_API boost::chrono::duration<int64_t, boost::ratio<60> >;
  template class QI_API boost::chrono::duration<int64_t, boost::ratio<3600> >;
  template class QI_API boost::chrono::time_point<qi::SteadyClock>;
  template class QI_API boost::chrono::time_point<qi::SystemClock>;
# endif

# include <qi/detail/clock.hxx>

#endif  // _QI_OS_HPP_
