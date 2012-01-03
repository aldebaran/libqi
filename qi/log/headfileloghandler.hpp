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

    /** \brief Log the \a length first lines to file.
     *  \ingroup qilog
     *
     */
    class QI_API HeadFileLogHandler
    {
    public:
      HeadFileLogHandler(const std::string &filePath,
                         int length = 2000);
      virtual ~HeadFileLogHandler();

      void log(const qi::log::LogLevel verb,
               const qi::os::timeval   date,
               const char              *category,
               const char              *msg,
               const char              *file,
               const char              *fct,
               const int               line);

    private:
      QI_DISALLOW_COPY_AND_ASSIGN(HeadFileLogHandler);
      PrivateHeadFileLogHandler* _private;
    }; // !HeadFileLogHandler

  }; // !log
}; // !qi

#endif  // _LIBQI_QI_LOG_HEADFILELOGHANDLER_HPP_
