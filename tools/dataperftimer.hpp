/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef     AL_TEST_DATAPERFTIMER_HPP_
# define    AL_TEST_DATAPERFTIMER_HPP_

#include <boost/timer.hpp>

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
      DataPerfTimer();

      void start(const unsigned long loopCount = 10000, const unsigned long msgSize = 2);
      void stop(bool shouldPrint = true);
      void print();

    protected:
      boost::timer  fTimer;
      unsigned long fLoopCount;
      unsigned long fMsgSize;
      double        fElapsed;
      float         fMsgPs;
      float         fMgbPs;
    };
  }
}


#endif  // AL_TEST_DATAPERFTIMER_HPP_

