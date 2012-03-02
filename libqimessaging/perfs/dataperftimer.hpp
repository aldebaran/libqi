/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_DETAILS_DATAPERFTIMER_HPP_
#define _QIMESSAGING_DETAILS_DATAPERFTIMER_HPP_

#include <qimessaging/api.hpp>
#include <qi/os.hpp>
#include <boost/timer.hpp>
#include <string>

namespace qi {
  namespace perf {


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
    class QIMESSAGING_API DataPerfTimer
    {
    public:

      /// <summary> Constructor. </summary>
      /// <param name="testDescription"> Information describing the test. </param>
      /// <param name="showHeader"> true to show, false to hide the header. </param>
      DataPerfTimer(const std::string& testDescription = "", bool showHeader = true);

      /// <summary> Starts the timer </summary>
      /// <param name="loopCount"> Number of loops. </param>
      /// <param name="msgSize"> Size of the message. </param>
      void start(const unsigned long loopCount = 1, const unsigned long msgSize = 0);

      /// <summary> Stops the timer and optionaly prints results </summary>
      /// <param name="shouldPrint"> true if should print. </param>
      void stop(bool shouldPrint = true);


    protected:
      void printHeader(const std::string& testDescription = "");
      void print(bool cpu);

      //cpu time
      boost::timer    fRt;
      //wallclock time
      qi::os::timeval fStartTime;

      unsigned long   fLoopCount;
      unsigned long   fMsgSize;
      double          fMsgPs;
      double          fMgbPs;
      double          fPeriod;
    };
  }
}


#endif  // _QIMESSAGING_DETAILS_DATAPERFTIMER_HPP_

