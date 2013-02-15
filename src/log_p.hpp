/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef   LOG_P_HPP_
# define  LOG_P_HPP_

#include <string>
#include <qi/os.hpp>

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
                        const int                line);
  }
} // namespace qi::detail

#endif	    /* !LOG_P_PP_ */
