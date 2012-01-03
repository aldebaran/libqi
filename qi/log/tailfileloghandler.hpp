/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#pragma once
#ifndef _LIBQI_QI_LOG_TAILFILELOGHANDLER_HPP_
#define _LIBQI_QI_LOG_TAILFILELOGHANDLER_HPP_

# include <qi/log.hpp>
# include <string>

namespace qi {
  namespace log {
    class PrivateTailFileLogHandler;

    /** \brief Log the \a length first lines to file.
     *  \ingroup qilog
     *
     */
    class QI_API TailFileLogHandler
    {
    public:
      TailFileLogHandler(const std::string &filePath);
      virtual ~TailFileLogHandler();

      void log(const qi::log::LogLevel verb,
               const qi::os::timeval   date,
               const char              *category,
               const char              *msg,
               const char              *file,
               const char              *fct,
               const int               line);

    private:
      QI_DISALLOW_COPY_AND_ASSIGN(TailFileLogHandler);
      PrivateTailFileLogHandler* _private;
    }; // !TailFileLogHandler

  }; // !log
}; // !qi

#endif  // _LIBQI_QI_LOG_TAILFILELOGHANDLER_HPP_
