#pragma once
/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef QI_GETENV_HPP
#define QI_GETENV_HPP

#include <qi/os.hpp>
#include <boost/lexical_cast.hpp>

namespace qi
{
namespace os
{

template <typename T>
T getEnvDefault(const char* name, T defaultVal)
{
  std::string sval = qi::os::getenv(name);
  if (sval.empty())
    return defaultVal;
  else
    return boost::lexical_cast<T>(sval);
}

}
}

#endif
