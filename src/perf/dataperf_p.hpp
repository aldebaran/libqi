/*
** Author(s):
** - Nicolas Cornu <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QI_PERF_DATAPERF_P_HPP_
#define _QI_PERF_DATAPERF_P_HPP_

#include <qi/perf/dataperf.hpp>

#include <qi/os.hpp>

// TODO: Remove once we migrate to a non-deprecated boost timer type.
#define BOOST_ALLOW_DEPRECATED_HEADERS
#include <boost/timer.hpp>
#undef BOOST_ALLOW_DEPRECATED_HEADERS

namespace qi
{
  class DataPerfPrivate
  {
  public:
    //! Default constructor
    DataPerfPrivate();

    //! Name of the benchmark
    std::string     benchmarkName;
    //! Used for measuring cpu time
    boost::timer    cpuTime;
    //! Used for store the stare time
    qi::SteadyClock::time_point fStartTime;

    //! Total time taken for the benchmark
    double          wallClockElapsed;
    //! CPU time for the benchmark
    double          cpuElapsed;
    //! Number of loop done by this benchmark
    unsigned long   fLoopCount;
    //! Size of the message transmitted by this benchmark
    unsigned long   fMsgSize;
    //! Variable
    std::string variable;
  };
}

#endif  // _QI_PERF_DATAPERF_P_HPP_
