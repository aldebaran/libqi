#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
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
