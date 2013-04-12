/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#pragma once
#ifndef _LIBQI_QI_LOG_HEADFILELOGHANDLER_HPP_
#define _LIBQI_QI_LOG_HEADFILELOGHANDLER_HPP_

# include <qi/log.hpp>
# include <string>

namespace qi {
  namespace log {
    class PrivateHeadFileLogHandler;

    /// Log the first length lines to a file.
    class QI_API HeadFileLogHandler
    {
    public:
      /// \brief Initialize the head file handler on the file. File is opened
      ///        directly on construction.
      HeadFileLogHandler(const std::string &filePath,
                         int length = 2000);
      /// Closes the file.
      virtual ~HeadFileLogHandler();

      /// \brief Writes a log message to the file if it is part of the first
      ///        length lines.
      void log(const qi::log::LogLevel verb,
               const qi::os::timeval   date,
               const char              *category,
               const char              *msg,
               const char              *file,
               const char              *fct,
               const int               line);

    private:
      QI_DISALLOW_COPY_AND_ASSIGN(HeadFileLogHandler);
      PrivateHeadFileLogHandler* _p;
    }; // !HeadFileLogHandler

  }; // !log
}; // !qi

#endif  // _LIBQI_QI_LOG_HEADFILELOGHANDLER_HPP_
