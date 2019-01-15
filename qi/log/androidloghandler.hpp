#pragma once
/*
 * Copyright (c) 2016 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_LOG_ANDROIDLOGHANDLER_HPP_
#define _QI_LOG_ANDROIDLOGHANDLER_HPP_

#include <qi/log.hpp>
#include <boost/function.hpp>

namespace qi
{
namespace log
{
  /**
   * \includename{qi/log/androidloghandler.hpp}
   * @brief create a log handler which redirect logs to android log system.
   *
   * Limitations: timestamps, file name, function name and line number are discarded.
   */
  Handler makeAndroidLogHandler();

} // namespace log
} // namespace qi

#endif // _QI_LOG_ANDROIDLOGHANDLER_HPP_
