/*
** Author(s):
** - Nicolas Cornu <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QI_PERF_DATAPERF_P_HPP_
#define _QI_PERF_DATAPERF_P_HPP_

#include <qiperf/dataperf.hpp>

#include <qi/os.hpp>
#include <boost/timer.hpp>

namespace qi
{
  class DataPerfPrivate
  {
  public:
    //! Name of the benchmark
    std::string     benchmarkName;
    //! Used for measuring cpu time
    boost::timer    cpuTime;
    //! Used for store the stare time
    qi::os::timeval fStartTime;

    //! Total time taken for the benchmark
    double          wallClockElapsed;
    //! CPU time for the benchmark
    double          cpuElapsed;
    //! Number of loop done by this benchmark
    unsigned long   fLoopCount;
    //! Size of the message transmitted by this benchmark
    unsigned long   fMsgSize;
  };
}

#endif  // _QI_PERF_DATAPERF_P_HPP_
