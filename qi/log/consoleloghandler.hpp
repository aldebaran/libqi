/*
 *  Author(s):
 *  - Cedric Gestes <gestes@aldebaran-robotics.com>
 *  - Chris  Kilner <ckilner@aldebaran-robotics.com>
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 *
 *  Copyright (C) 2010, 2011 Aldebaran Robotics
 */


#pragma once
#ifndef _LIBQI_QI_LOG_CONSOLELOGHANDLER_HPP_
#define _LIBQI_QI_LOG_CONSOLELOGHANDLER_HPP_

# include <cstdarg>
# include <qi/noncopyable.hpp>
# include <qi/log.hpp>

namespace qi {
  namespace log {

    class PrivateConsoleLogHandler;

    /// <summary> Can print colored logs to the console </summary>
    class QI_API ConsoleLogHandler : qi::noncopyable
    {
    public:
      ConsoleLogHandler();

      void log(const qi::log::LogLevel verb,
               const qi::os::timeval   date,
               const char              *category,
               const char              *msg,
               const char              *file,
               const char              *fct,
               const int               line);


    protected:
      PrivateConsoleLogHandler* _private;
    };
  }
}


#endif  // _LIBQI_QI_LOG_CONSOLELOGHANDLER_HPP_
