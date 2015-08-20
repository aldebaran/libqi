#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_LOG_HEADFILELOGHANDLER_HPP_
#define _QI_LOG_HEADFILELOGHANDLER_HPP_

#include <boost/noncopyable.hpp>
#include <qi/log.hpp>
#include <string>

namespace qi
{
namespace log
{
  struct PrivateHeadFileLogHandler;

  /**
   * \brief Log the first length lines to a file.
   * \includename{qi/log/headfileloghandler.hpp}
   *
   * This class writes the logs to a file, providing they are part of the first
   * ``length`` lines.
   */
  class QI_API HeadFileLogHandler : private boost::noncopyable
  {
  public:
    /**
     *
     * \brief Initialize the head file handler on the file. File is opened directly on construction.
     * \param filePath path to the file.
     * \param length number of messages that will be written to the file.
     *
     * \verbatim
     * .. warning::
     *
     *      If the file could not be open, it logs a warning and every log call
     *      will fail silently.
     * \endverbatim
     */
    HeadFileLogHandler(const std::string& filePath, int length = 2000);
    /**
     * \brief Closes the file.
     */
    virtual ~HeadFileLogHandler();

    /**
     * \brief Writes a log message to the file if it is part of the first length lines.
     * \param verb verbosity of the log message.
     * \param date qi::Clock date at which the log message was issued.
     * \param date qi::SystemClock date at which the log message was issued.
     * \param category will be used in future for filtering
     * \param msg actual message to log.
     * \param file filename from which this log message was issued.
     * \param fct function name from which this log message was issued.
     * \param line line number in the issuer file.
     *
     * If the file could not be open, this function will fail silently, otherwise
     * it will directly write the log message to the file and flush its output.
     *
     * When ``length`` messages will be written to the file, it will discard all
     * messages.
     */
    void log(const qi::LogLevel verb,
             const qi::Clock::time_point date,
             const qi::SystemClock::time_point systemDate,
             const char* category,
             const char* msg,
             const char* file,
             const char* fct,
             const int line);

  private:
    PrivateHeadFileLogHandler* _p;
  }; // !HeadFileLogHandler

}; // !log
}; // !qi

#endif // _QI_LOG_HEADFILELOGHANDLER_HPP_
