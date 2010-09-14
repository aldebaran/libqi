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

    /// <summary>
    /// This class is a utility to calculate the msg/sec
    /// and MB/sec performance of a function
    ///
    /// Usage:
    ///
    /// @code
    /// DataPerfTimer dpt();
    ///
    /// dpt.start(10, 512);
    /// for (int i = 0; i < 10; ++i) {
    ///   send(512octect);
    /// }
    /// dpt.stop();
    /// @endcode
    /// </summary>
    class DataPerfTimer
    {
    public:

      /// <summary> Constructor. </summary>
      /// <param name="testDescription"> Information describing the test. </param>
      /// <param name="showHeader"> true to show, false to hide the header. </param>
      DataPerfTimer(const std::string& testDescription = "", bool showHeader = true);

      /// <summary> Print header. </summary>
      /// <param name="testDescription"> Information describing the test. </param>
      void printHeader(const std::string& testDescription = "");

      /// <summary> Starts the timer </summary>
      /// <param name="loopCount"> Number of loops. </param>
      /// <param name="msgSize"> Size of the message. </param>
      void start(const unsigned long loopCount = 1, const unsigned long msgSize = 0);

      /// <summary> Stops the timer and optionaly prints results </summary>
      /// <param name="shouldPrint"> true if should print. </param>
      void stop(bool shouldPrint = true);

      /// <summary> Prints this object. </summary>
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

