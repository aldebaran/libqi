/*
**  Author(s):
**   - Nicolas Cornu <ncornu@aldebaran-robotics.com>
**
**  Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QI_PERF_DATAPERFSUITE_HPP_
#define _QI_PERF_DATAPERFSUITE_HPP_

#include <string>

#include <qiperf/api.hpp>
#include <qiperf/dataperf.hpp>

namespace qi
{
  class DataPerfSuitePrivate;

  //! A class to perform benchmarks
  /**
   * This class is used to perform benchmarks.
   *
   * Use:
   *
   * DataPerfSuite DPS("My_project", "My_executable", DataPerfSuite::OutputType_Normal, "data.xml");
   * (...)
   * DataPerf DP;
   * (...)
   * while (1) {
   *   DP.start();
   *   (...) // What I want to measure.
   *   DP.stop();
   *   (...)
   *   DPS << DP;
   * }
   *
   */
  class QIPERF_API DataPerfSuite
  {
    public:

      /**
       * An enum to precise the type of output.
       */
      enum OutputType {
        OutputType_Normal = 0, /**< The "normal" way */
        OutputType_Codespeed   /**< A type defined to be used by Codespeed */
      };

      //! Constructor
      /**
       * Constructor
       * @param filename filename where store output. Put an empty string to output on stdout
       * @param outputType type of output
       * @param projectName the name of the project being benchmarked
       * @param executableName the name of the executable use for benchmarking
       */
      DataPerfSuite(const std::string& projectName, const std::string& executableName, OutputType outputType = OutputType_Normal, const std::string& filename = "");

      //! Destructor
      ~DataPerfSuite();

      //! Overloading of the stream insertion operator.
      /**
       * This overloading is used for printing data out.
       * @param data data to print out
       */
      DataPerfSuite& operator<<(const DataPerf& data);

    private:
      DataPerfSuitePrivate *_p;
  };
}

#include <qiperf/details/dataperfsuite.hxx>

#endif  // _QI_PERF_DATAPERFSUITE_HPP_
