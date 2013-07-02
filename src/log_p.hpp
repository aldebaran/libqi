#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _SRC_LOG_P_HPP_
#define _SRC_LOG_P_HPP_

#include <qi/os.hpp>
#include <qi/log.hpp>

#define CATSIZEMAX 16

namespace qi
{
  namespace detail
  {
    static LogContext fileLogContext = qi::LogContextAttr_Verbosity | qi::LogContextAttr_Tid | qi::LogContextAttr_Date | qi::LogContextAttr_Category;

    // export so we can test it
    std::string logline(LogContext             context,
                        const qi::os::timeval  date,
                        const char            *category,
                        const char            *msg,
                        const char            *file,
                        const char            *fct,
                        const int              line,
                        const qi::LogLevel     verb
                        );


    const std::string dateToString(const qi::os::timeval date);
    const std::string usTimeToString(const qi::os::timeval date);
    const std::string tidToString();

    int rtrim(const char *msg);
  }
} // namespace qi::detail

#endif  // _SRC_LOG_P_HPP_
