/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef     AL_TEST_DATAPERFTIMER_HPP_
# define    AL_TEST_DATAPERFTIMER_HPP_

#include <boost/timer.hpp>
#include <string>
#include <rttools/rttime.h>

namespace AL {
  namespace Test {

    /**
     * This class is a utility to calculate the msg/sec
     * and MB/sec performance of a function
     *
     * use like this:
     *
     * DataPerfTimer dpt();
     *
     * dpt.start(10, 512);
     * for (int i = 0; i < 10; ++i) {
     *   send(512octect);
     * }
     * dpt.stop();
     */
    class DataPerfTimer
    {
    public:
      DataPerfTimer(const std::string& testDescription = "", bool showHeader = true);

      void printHeader(const std::string& testDescription = "");
      void start(const unsigned long loopCount = 1, const unsigned long msgSize = 0);
      void stop(bool shouldPrint = true);
      void print();

    protected:
      RtTime        rt;
      unsigned long fLoopCount;
      unsigned long fMsgSize;
      double        fElapsed;
      double        fMsgPs;
      double        fMgbPs;
    };
  }
}


#endif  // AL_TEST_DATAPERFTIMER_HPP_

