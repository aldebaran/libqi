/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#ifndef   	FILESYSTEM_HPP_
# define   	FILESYSTEM_HPP_

#include <string>

namespace qi
{
  namespace detail
  {
    std::string normalizePath(const std::string& path);
  }
} // namespace qi::detail

#endif	    /* !FILESYSTEM_PP_ */
