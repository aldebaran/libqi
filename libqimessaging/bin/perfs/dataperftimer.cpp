/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iomanip>
#include "dataperftimer.hpp"
#include <qi/log.hpp>

namespace qi {
  namespace perf {

    DataPerfTimer::DataPerfTimer(const std::string& testDescription, bool showHeader)
      : fLoopCount(1),
        fMsgSize(2),
        fMsgPs(0.0),
        fMgbPs(0.0),
        fPeriod(0.0)
    {
      if (showHeader) {
        printHeader(testDescription);
      }
    }

    void DataPerfTimer::printHeader(const std::string& testDescription) {
      if (! testDescription.empty()) {
        qiLogInfo("DataPerfTimer") << testDescription;
      }
      qiLogInfo("DataPerfTimer") << "bytes, msg/s, Mb/s, period (us), cpu/total";
    }

    void DataPerfTimer::start(
      const unsigned long loopCount,
      const unsigned long msgSize) {
      fLoopCount = loopCount;
      fMsgSize = msgSize;
      fRt.restart();
      qi::os::gettimeofday(&fStartTime);
    }

    void DataPerfTimer::stop(bool shouldPrint) {
      double          wallClockElapsed;
      double          cpuElapsed;
      qi::os::timeval tv;

      qi::os::gettimeofday(&tv);
      cpuElapsed = fRt.elapsed();

      wallClockElapsed  = (tv.tv_sec - fStartTime.tv_sec);
      wallClockElapsed += (double)(tv.tv_usec - fStartTime.tv_usec) / 1000 / 1000;

      fMsgPs = 1.0 / (wallClockElapsed / (1.0 * fLoopCount));
      if (fMsgSize > 0) {
        fMgbPs = (fMsgPs * fMsgSize) / (1024.0 * 1024.0);
      }
      fPeriod = (double)wallClockElapsed * 1000.0 * 1000.0 / fLoopCount;
      fCpu = (double)cpuElapsed / (double)wallClockElapsed * 100;
      if (shouldPrint)
        print();
    }

    void DataPerfTimer::print()
    {
      if (fMsgSize > 0) {
        qiLogInfo("DataPerfTimer") << "wallclock: "
                  << std::fixed << std::setprecision(2) << fMsgSize << " b, "
                  << fMsgPs << " msg/s, "
                  << std::setprecision(12) << fMgbPs << " MB/s, "
                  << std::setprecision(0) << fPeriod << " us, "
                  << std::setprecision(1) << fCpu << " %";
      } else {
        qiLogInfo("DataPerfTimer") << "wallclock: " << std::setprecision(12)
                  << fMsgPs  << " msg/s"
                  << fCpu << " %";
      }
    }
  }
}

