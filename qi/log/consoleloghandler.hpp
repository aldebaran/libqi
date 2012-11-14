/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */


#pragma once
#ifndef _LIBQI_QI_LOG_CONSOLELOGHANDLER_HPP_
#define _LIBQI_QI_LOG_CONSOLELOGHANDLER_HPP_

# include <cstdarg>
# include <qi/log.hpp>

namespace qi {
  namespace log {

    class PrivateConsoleLogHandler;

    /// Print colored logs to the console.
    class QI_API ConsoleLogHandler
    {
    public:
      /// Initialize everything the console log handler needs to print on the
      /// console with colors.
      ConsoleLogHandler();

      /// Unloads any data managed by ConsoleLogHandler. Destructor is not
      /// virtual.
      ~ConsoleLogHandler();

      /// Prints a log message on the console.
      void log(const qi::log::LogLevel verb,
               const qi::os::timeval   date,
               const char              *category,
               const char              *msg,
               const char              *file,
               const char              *fct,
               const int               line);


    protected:
      QI_DISALLOW_COPY_AND_ASSIGN(ConsoleLogHandler);
      PrivateConsoleLogHandler* _private;
    };
  }
}


#endif  // _LIBQI_QI_LOG_CONSOLELOGHANDLER_HPP_
