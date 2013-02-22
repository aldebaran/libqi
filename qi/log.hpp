/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

/** @file qi/log.hpp
 *  @brief Convenient log macro
 */


#pragma once
#ifndef _LIBQI_QI_LOG_HPP_
#define _LIBQI_QI_LOG_HPP_

# include <string>
# include <iostream>
# include <sstream>
# include <cstdarg>
# include <cstdio>

#include <boost/format.hpp>
#include <boost/function/function_fwd.hpp>

#include <qi/os.hpp>


#  define qiLogCategory(Cat)                                           \
  static ::qi::log::category_type _QI_LOG_CATEGORY_GET() =              \
    ::qi::log::addCategory(Cat)



#if defined(NO_QI_DEBUG) || defined(NDEBUG)
# define qiLogDebug(...) ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogDebugF(Msg, ...)
#else
# define qiLogDebug(...)   _QI_LOG_MESSAGE_STREAM(debug,   Debug ,  __VA_ARGS__)
# define qiLogDebugF(Msg, ...)   _QI_LOG_MESSAGE(debug,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

#if defined(NO_QI_VERBOSE)
# define qiLogVerbose(...) ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogVerboseF(Msg, ...)
#else
# define qiLogVerbose(...) _QI_LOG_MESSAGE_STREAM(verbose, Verbose, __VA_ARGS__)
# define qiLogVerboseF(Msg, ...)   _QI_LOG_MESSAGE(verbose,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

#if defined(NO_QI_INFO)
# define qiLogInfo(...) ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogInfoF(Msg, ...)
#else
# define qiLogInfo(...)    _QI_LOG_MESSAGE_STREAM(info,    Info,    __VA_ARGS__)
# define qiLogInfoF(Msg, ...)   _QI_LOG_MESSAGE(info,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

#if defined(NO_QI_WARNING)
# define qiLogWarning(...) ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogWarningF(Msg, ...)
#else
# define qiLogWarning(...) _QI_LOG_MESSAGE_STREAM(warning, Warning, __VA_ARGS__)
# define qiLogWarningF(Msg, ...)   _QI_LOG_MESSAGE(warning,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

#if defined(NO_QI_ERROR)
# define qiLogError(...)   ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogErrorF(Msg, ...)
#else
# define qiLogError(...)   _QI_LOG_MESSAGE_STREAM(error,   Error,   __VA_ARGS__)
# define qiLogErrorF(Msg, ...)   _QI_LOG_MESSAGE(error,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

#if defined(NO_QI_FATAL)
# define qiLogFatal(...)  ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogFatalF(Msg, ...)
#else
# define qiLogFatal(...)   _QI_LOG_MESSAGE_STREAM(fatal,   Fatal,   __VA_ARGS__)
# define qiLogFatalF(Msg, ...)   _QI_LOG_MESSAGE(fatal,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

namespace qi {
  namespace log {
    enum LogLevel {
        silent = 0,
        fatal,
        error,
        warning,
        info,
        verbose,
        debug
    };
  }
}

namespace qi {
  namespace log {
    namespace detail {
      struct Category;
    }
    typedef detail::Category* category_type;

    typedef unsigned int Subscriber;

    typedef boost::function7<void,
                             const qi::log::LogLevel,
                             const qi::os::timeval,
                             const char*,
                             const char*,
                             const char*,
                             const char*,
                             int> logFuncHandler;

    QI_API void init(qi::log::LogLevel verb = qi::log::info,
                     int ctx = 0,
                     bool synchronous = true);

    QI_API void destroy();

    QI_API void log(const qi::log::LogLevel verb,
                    const char              *category,
                    const char              *msg,
                    const char              *file = "",
                    const char              *fct = "",
                    const int               line = 0);
    QI_API void log(const qi::log::LogLevel verb,
                    category_type           category,
                    const std::string&      msg,
                    const char              *file = "",
                    const char              *fct = "",
                    const int               line = 0);


    QI_API const char* logLevelToString(const qi::log::LogLevel verb);

    QI_API LogLevel stringToLogLevel(const char* verb);

    QI_API void setVerbosity(const qi::log::LogLevel lv);

    QI_API LogLevel verbosity();

    /// @return the list of existing categories
    QI_API std::vector<std::string> categories();

    /** Parse and execute a set of verbosity rules
    *  Semi-colon separated of rules.
    *  Each rule can be:
    *    - (+)?CAT    : enable category CAT
    *    - -CAT       : disable category CAT
    *    - CAT=level  : set category CAT to level
    *
    * Each category can include a '*' for globbing.
    */
    QI_API void setVerbosity(const std::string& env);

    QI_API void setVerbosity(Subscriber s, const qi::log::LogLevel lv);
    /// Add/get a category
    QI_API category_type addCategory(const std::string& name);
    /// Set \param cat to current verbosity level. Globbing is supported.
    QI_API void enableCategory(const std::string& cat);
    /// Set \param cat to silent log level. Globbing is supported.
    QI_API void disableCategory(const std::string& cat);
    /// Set \param cat to level \param level. Globbing is supported.
    QI_API void setCategory(const std::string& cat, LogLevel level);
    /// Set per-subscriber \param cat to level \param level. Globbing is supported.
    QI_API void setCategory(Subscriber sub, const std::string& cat, LogLevel level);
    /// \return true if given combination of category and level is enabled.
    QI_API bool isVisible(category_type category, LogLevel level);
    /// \return true if given combination of category and level is enabled.
    QI_API bool isVisible(const std::string& category, LogLevel level);

    QI_API void setContext(int ctx);

    QI_API int context();

    QI_API void setSynchronousLog(bool sync);

    QI_API Subscriber addLogHandler(const std::string& name,
                              qi::log::logFuncHandler fct);

    QI_API void removeLogHandler(const std::string& name);

    QI_API void flush();

  }
}

#include <qi/details/log.hxx>


#endif  // _LIBQI_QI_LOG_HPP_
