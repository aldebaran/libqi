#pragma once
/*
 * Copyright (c) 2016 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_LOG_ANDROIDLOGHANDLER_HPP_
#define _QI_LOG_ANDROIDLOGHANDLER_HPP_

#include <qi/log.hpp>

namespace qi
{
namespace log
{
  /**
   * \includename{qi/log/androidloghandler.hpp}
   * @brief The AndroidLogHandler class
   *
   * Redirect logs to android log system.
   * Does nothing if used on a non Android device.
   */
  class AndroidLogHandler
  {
  public:
    AndroidLogHandler();

    AndroidLogHandler(const AndroidLogHandler&) = delete;
    AndroidLogHandler& operator=(const AndroidLogHandler&) = delete;

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
    void log(qi::LogLevel verb,
             qi::Clock::time_point date,
             qi::SystemClock::time_point systemDate,
             const char* category,
             const char* msg,
             const char* file,
             const char* fct,
             int line);
  };

} // namespace log
} // namespace qi

#endif // _QI_LOG_ANDROIDLOGHANDLER_HPP_
