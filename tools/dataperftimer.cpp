/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <boost/timer.hpp>

#include "dataperftimer.hpp"

namespace AL {
  namespace test {

    DataPerfTimer::DataPerfTimer(const unsigned long loopCount, const unsigned long msgSize)
      : loopCount(loopCount),
        msgSize(msgSize)
    {}

    void DataPerfTimer::start() {
      std::cout << "Bytes, msg/s, MB/s" << std::endl;
      timer.restart();
    }

    void DataPerfTimer::stop(char shouldPrint) {
      elapsed = timer.elapsed();

      msgPs = 1.0f / ((float)elapsed / (1.0f * loopCount) );
      mgbPs = (msgPs * msgSize) / (1024 * 1024.0f);
      if (shouldPrint)
        print();
    }

    void DataPerfTimer::print()
    {
      std::cout << msgSize << ", " << msgPs << ", " << mgbPs << std::endl;
    }

  }
}

