#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_LOG_CONSOLELOGHANDLER_HPP_
#define _QI_LOG_CONSOLELOGHANDLER_HPP_

#include <boost/noncopyable.hpp>
#include <qi/log.hpp>

namespace qi
{
namespace log
{
  class PrivateConsoleLogHandler;

  /**
   * \includename{qi/log/consoleloghandler.hpp}
   * \brief Print colored logs to the console.
   *
   * Colors will only be enabled if the output is a tty.
   */
  class QI_API ConsoleLogHandler : private boost::noncopyable
  {
  public:
    /// Initialize everything the console log handler needs to print on the
    /// console with colors.
    ConsoleLogHandler();

    /// Unloads any data managed by ConsoleLogHandler. Destructor is not
    /// virtual.
    ~ConsoleLogHandler();

    /**
     * \brief Prints a log message on the console.
     * \param verb verbosity of the log message.
     * \param date qi::Clock date at which the log message was issued.
     * \param date qi::SystemClock date at which the log message was issued.
     * \param category will be used in future for filtering
     * \param msg actual message to log.
     * \param file filename from which this log message was issued.
     * \param fct function name from which this log message was issued.
     * \param line line number in the issuer file.
     */
    void log(const qi::LogLevel verb,
             const qi::Clock::time_point date,
             const qi::SystemClock::time_point systemDate,
             const char* category,
             const char* msg,
             const char* file,
             const char* fct,
             const int line);

    /**
     * \brief Update color status (Never, Always, Auto)
     */
    void updateColor();

  protected:
    PrivateConsoleLogHandler* _p;
  };
}
}

#endif // _QI_LOG_CONSOLELOGHANDLER_HPP_
