/*
** Author(s):
** - Nicolas Cornu <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QI_PERF_DATAPERF_HPP_
#define _QI_PERF_DATAPERF_HPP_

#include <qiperf/api.hpp>
#include <string>

namespace qi
{
  class DataPerfPrivate;

  //! Class to compute and store a benchmark time.
  /**
   * This class is used to compute and store a benchmark time.
   * It gives too some facilities.
   *
   * Use:
   *
   * DataPerf DP;
   * (...)
   * DP.start();
   * (...) // Things we want to benchmark
   * DP.stop();
   * (...)
   * std::cout << "Ratio CPU/Total: " << DP.getCPU();
   *
   */
  class QIPERF_API DataPerf
  {
  public:
    //! Constructor
    DataPerf();
    //! Destructor
    ~DataPerf();

    //! Start measuring time
    /**
     * This function is used to start measuring time.
     * @param loopCount How many time benchmarked code have been loop (used for calculus)
     * @param msgSize The size of the data transmitted (used for calculus)
     */
    void start(const std::string& benchmarkName, const unsigned long loopCount = 1, const unsigned long msgSize = 0);
    //! Stop measuring time
    void stop();

    //! Return the name of the benchmark
    std::string getBenchmarkName() const;
    //! Return the size of the message transmitted
    unsigned long getMsgSize() const;
    //! Return the average time taken by a single execution of the benchmarked code
    double getPeriod() const;
    //! Return the time taken by the CPU against the total time
    double getCpu() const;
    //! Return the number of messages transmitted in a single second
    double getMsgPerSecond() const;
    //! Return the MB transmitted in a single second
    double getMegaBytePerSecond() const;

  private:
    DataPerfPrivate *_p;
  };
}


#endif  // _QIPERF_DETAILS_DATAPERF_HPP_

