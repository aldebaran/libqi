#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_LOG_TAILFILELOGHANDLER_HPP_
#define _QI_LOG_TAILFILELOGHANDLER_HPP_

#include <boost/noncopyable.hpp>
#include <qi/log.hpp>
#include <string>

namespace qi
{
namespace log
{
  struct PrivateTailFileLogHandler;

  /**
   * \brief Keeps at most 2 MiB of logs.
   * \includename{qi/log/tailfileloghandler.hpp}
   *
   * \verbatim
   * This class writes the logs to a file. When more than 1 MiB are written, it
   * moves the file to *filePath*.old, truncates *filePath*, and keeps writing
   * inside it. This means that you will get at most the last 2 MiB logged by
   * :cpp:class:`qi::log::TailFileLogHandler`.
   * \endverbatim
   */
  class QI_API TailFileLogHandler : private boost::noncopyable
  {
  public:
    /**
     * \brief Initialize the tail file log handler. File is opened on construction.
     * \param filePath path to the file.
     *
     * \verbatim
     * .. warning::
     *
     *      If the file could not be opened, it logs a warning and every log call
     *      will silently fail.
     * \endverbatim
     */
    TailFileLogHandler(const std::string& filePath);

    /**
     * \brief Closes the file.
     */
    virtual ~TailFileLogHandler();

    /**
     * \brief Writes the log message to the file.
     * \param verb verbosity of the log message.
     * \param date qi::Clock date at which the log message was issued.
     * \param date qi::SystemClock date at which the log message was issued.
     * \param category will be used in future for filtering
     * \param msg message to log.
     * \param file filename in the sources from which this log message was issued.
     * \param fct function name from which this log message was issued.
     * \param line line number in the issuer file.
     *
     * If the file could not be opened, this function will silently fail, otherwise
     * it will directly write the log message to the file and flush its output. see
     * detailed description for more details on what "tail" means.
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
    PrivateTailFileLogHandler* _p;
  }; // !TailFileLogHandler

}; // !log
}; // !qi

#endif // _QI_LOG_TAILFILELOGHANDLER_HPP_
