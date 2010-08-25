/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef     AL_MESSAGING_DATAPERFTIMER_HPP_
# define    AL_MESSAGING_DATAPERFTIMER_HPP_

#include <boost/timer.hpp>

namespace AL {
  namespace test {

    /**
     * This class is a utility to calculate the msg/sec
     * and MB/sec performance of a function
     *
     * use like this:
     *
     * DataPerfTimer dpt(10, 512);
     *
     * dpt.start();
     * for (int i = 0; i < 10; ++i) {
     *   send(512octect);
     * }
     * dpt.stop();
     */
    class DataPerfTimer
    {
    public:
      DataPerfTimer(const unsigned long loopCount, const unsigned long msgSize);

      void start();
      void stop(char shouldPrint = 1);
      void print();

    protected:
      boost::timer  timer;
      unsigned long loopCount;
      unsigned long msgSize;
      double        elapsed;
      float         msgPs;
      float         mgbPs;
    };

  }
}


#endif /* !DATAPERFTIMER_PP_ */
