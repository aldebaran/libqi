/*
**
** Author(s):
**  -  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#include <stdarg.h>
#include <qi/log.h>
#include <qi/log.hpp>

/**
 * call this to make some log
 */
void qi_log(const QiLogLevel  verb,
            const char       *file,
            const char       *fct,
            const int         line,
            const char       *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  qi::log::log(static_cast<qi::log::LogLevel>(verb), file, fct, line, fmt, vl);
  va_end(vl);
}

