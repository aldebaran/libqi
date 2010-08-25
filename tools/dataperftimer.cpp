/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include "dataperftimer.hpp"
#include <boost/timer.hpp>
#include <iostream>

namespace AL {
  namespace Test {

    DataPerfTimer::DataPerfTimer()
      : fLoopCount(10000),
        fMsgSize(2),
        fMgbPs(0.0f),
        fMsgPs(0.0f),
        fElapsed(0.0)
    {
      std::cout << "Bytes, msg/s, MB/s" << std::endl;
    }

    void DataPerfTimer::start(
      const unsigned long loopCount,
      const unsigned long msgSize) {
      fLoopCount = loopCount;
      fMsgSize = msgSize;
      fTimer.restart();
    }

    void DataPerfTimer::stop(bool shouldPrint) {
      fElapsed = fTimer.elapsed();

      fMsgPs = 1.0f / ((float)fElapsed / (1.0f * fLoopCount) );
      fMgbPs = (fMsgPs * fMsgSize) / (1024 * 1024.0f);
      if (shouldPrint)
        print();
    }

    void DataPerfTimer::print()
    {
      std::cout << fMsgSize << ", " << fMsgPs << ", " << fMgbPs << std::endl;
    }
  }
}

