#pragma once
/*
** Author(s):
**  - Cedric Gestes <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#ifndef         _C_QI_LOG_H_
# define        _C_QI_LOG_H_

#include <qi/api.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum e_qi_log_level {
  QI_SILENT = 0,
  QI_FATAL,
  QI_ERROR,
  QI_WARNING,
  QI_INFO,
  QI_DEBUG,
} QiLogLevel;

#ifdef NO_QI_DEBUG
# define qi_debug(...)
#else
# define qi_debug(...)   qi_log(QI_DEBUG,   __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif

#ifdef NO_QI_INFO
# define qi_info(...)
#else
#define qi_info(...)     qi_log(QI_INFO,    __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif

#ifdef NO_QI_WARNING
# define qi_warning(...)
#else
# define qi_warning(...) qi_log(QI_WARNING, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif

#ifdef NO_QI_ERROR
# define qi_error(...)
#else
# define qi_error(...)   qi_log(QI_ERROR,   __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif

#ifdef NO_QI_FATAL
# define qi_fatal(...)
#else
# define qi_fatal(...)   qi_log(QI_FATAL,   __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif


/**
 * call this to make some log
 */
QI_API void qi_log(const QiLogLevel  verb,
                   const char       *file,
                   const char       *fct,
                   const int         line,
                   const char       *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif  /* !LOG_H_ */
