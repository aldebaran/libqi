#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_LOG_JOURNALDLOGHANDLER_HPP_
#define _QI_LOG_JOURNALDLOGHANDLER_HPP_

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <qi/log.hpp>

namespace qi
{
namespace log
{
  /**
   * \includename{qi/log/journaldloghandler.hpp}
   *
   * This class outputs all logs to journald.
   */
  class QI_API JournaldLogHandler: private boost::noncopyable
  {
  public:
    /**
     * \brief Write logs messages on a file.
     * \param verb verbosity of the log message.
     * \param date date at which the log message was issued.
     * \param category will be used in future for filtering
     * \param msg actual message to log.
     * \param file filename from which this log message was issued.
     * \param fct function name from which this log message was issued.
     * \param line line number in the issuer file.
     */
    void log(const qi::LogLevel verb,
             const char* category,
             const char* msg,
             const char* file,
             const char* fct,
             const int line);
  };
}; // !log
}; // !qi

#endif

