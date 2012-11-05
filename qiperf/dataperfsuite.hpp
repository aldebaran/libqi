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

  /// A class to perform benchmarks.
  class QIPERF_API DataPerfSuite
  {
    public:

      /// An enum to precise the type of output.
      enum OutputType {
        OutputType_Normal    = 0, ///< The "normal" way.
        OutputType_Codespeed = 1, ///< A type defined to be used by Codespeed.
      };

      /// Constructor
      DataPerfSuite(const std::string& projectName, const std::string& executableName, OutputType outputType = OutputType_Normal, const std::string& filename = "");

      /// Destructor
      ~DataPerfSuite();

      /// Overloading used to print data out.
      DataPerfSuite& operator<<(const DataPerf& data);

    private:
      DataPerfSuitePrivate *_p;
  };
}

#include <qiperf/details/dataperfsuite.hxx>

#endif  // _QI_PERF_DATAPERFSUITE_HPP_
