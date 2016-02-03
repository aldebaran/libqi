#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_LOG_CSVLOGHANDLER_HPP_
#define _QI_LOG_CSVLOGHANDLER_HPP_

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <qi/log.hpp>

namespace qi
{
namespace log
{
  struct PrivateCsvLogHandler;

  /**
   * \includename{qi/log/csvloghandler.hpp}
   *
   * This class writes all logs to a file in csv format.
   */
  class QI_API CsvLogHandler : private boost::noncopyable
  {
  public:
    /**
     * \brief Initialize the file handler on the file. File is opened directly on construction.
     * \param filePath the path to the file where log messages will be written.
     *
     * \verbatim
     * .. warning::
     *
     *      If the file could not be opened, it logs a warning and every log call
     *      will silently fail.
     * \endverbatim
     */
    explicit CsvLogHandler(const std::string& filePath);

    /**
     * \brief Closes the file.
     */
    ~CsvLogHandler();

    /**
     * \brief Write logs messages on a file.
     * \param verb verbosity of the log message.
     * \param date date at which the log message was issued.
     * \param category will be used in future for filtering
     * \param msg actual message to log.
     * \param file filename from which this log message was issued.
     * \param fct function name from which this log message was issued.
     * \param line line number in the issuer file.
     *
     * If the file could not be opened, this function will silently fail, otherwise
     * it will directly write the log message to the file and flush its output.
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
    boost::scoped_ptr<PrivateCsvLogHandler> _p;
  }; // !CsvLogHandler

}; // !log
}; // !qi

#endif // _QI_LOG_CSVLOGHANDLER_HPP_
