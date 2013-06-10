#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef   LOG_P_HPP_
# define  LOG_P_HPP_

#include <qi/os.hpp>
#include <qi/log.hpp>

#define CATSIZEMAX 16

namespace qi
{
  namespace detail
  {
    // export so we can test it
    std::string QI_API logline(const qi::os::timeval    date,
                        const char              *category,
                        const char              *msg,
                        const char              *file,
                        const char              *fct,
                        const int                line,
                        const qi::log::LogLevel  verb=qi::log::silent
                        );
    enum logCategories {
      LOG_NONE      = 0,
      LOG_VERBOSITY = 1 << 0,
      LOG_CATEGORY  = 1 << 1,
      LOG_MESSAGE   = 1 << 2,
      LOG_FILE      = 1 << 3,
      LOG_TID       = 1 << 4,
      LOG_DATE      = 1 << 5,
      LOG_FUNCTION  = 1 << 6,
    };

    const std::string dateToString(const qi::os::timeval date);

    const std::string tidToString();

    const std::string categoryToFixedCategory(const char *category, int size=30);

    int rtrim(const char *msg);

    int categoriesFromContext();
  }
} // namespace qi::detail

#endif	    /* !LOG_P_PP_ */
