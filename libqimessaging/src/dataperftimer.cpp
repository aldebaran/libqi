/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iomanip>
#include <qimessaging/details/dataperftimer.hpp>
#include <iostream>
//#include <rttools/rttime.h>

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
        std::cout << testDescription << std::endl;
      }
      std::cout << "bytes, msg/s, Mb/s, period (us)" << std::endl;
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
      if (shouldPrint)
        print(false);

      fMsgPs = 1.0 / (cpuElapsed / (1.0 * fLoopCount));
      if (fMsgSize > 0) {
        fMgbPs = (fMsgPs * fMsgSize) / (1024.0 * 1024.0);
      }
      fPeriod = (double)cpuElapsed * 1000.0 / fLoopCount * 1000.0;
      if (shouldPrint)
        print(true);
    }

    void DataPerfTimer::print(bool cpu)
    {
      std::string str = "wallclock: ";
      if (cpu)
        str = "cpu      : ";
      if (fMsgSize > 0) {
        std::cout << str
                  << std::fixed << std::setprecision(2) << fMsgSize << " b, "
                  << fMsgPs << " msg/s, "
                  << std::setprecision(12) << fMgbPs << " MB/s, "
                  << std::setprecision(0) << fPeriod << " us" << std::endl;
      } else {
        std::cout << str << std::setprecision(12) << fMsgPs  << " msg/s" << std::endl;
      }
    }
  }
}

