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

# include <map>
# include <string>
# include <iostream>
# include <sstream>
# include <cstdarg>
# include <cstdio>

#include <boost/function/function_fwd.hpp>

#include <qi/config.hpp>
#include <qi/os.hpp>

#if defined(NO_QI_DEBUG) || defined(NDEBUG)
# define qiLogDebug(...)        if (false) qi::log::detail::NullStream(__VA_ARGS__).self()
#elif defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
#   define qiLogDebug(...)      qi::log::LogStream(qi::log::debug, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
#   define qiLogDebug(...)      qi::log::LogStream(qi::log::debug, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#ifdef NO_QI_VERBOSE
# define qiLogVerbose(...)      if (false) qi::log::detail::NullStream(__VA_ARGS__).self()
#elif defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
# define qiLogVerbose(...)      qi::log::LogStream(qi::log::verbose, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define qiLogVerbose(...)      qi::log::LogStream(qi::log::verbose, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#ifdef NO_QI_INFO
# define qiLogInfo(...)         if (false) qi::log::detail::NullStream(__VA_ARGS__).self()
#elif defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
# define qiLogInfo(...)         qi::log::LogStream(qi::log::info, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define qiLogInfo(...)         qi::log::LogStream(qi::log::info, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#ifdef NO_QI_WARNING
# define qiLogWarning(...)      if (false) qi::log::detail::NullStream(__VA_ARGS__).self()
#elif defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
# define qiLogWarning(...)      qi::log::LogStream(qi::log::warning, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define qiLogWarning(...)      qi::log::LogStream(qi::log::warning, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#ifdef NO_QI_ERROR
# define qiLogError(...)        if (false) qi::log::detail::NullStream(__VA_ARGS__).self()
#elif defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
# define qiLogError(...)        qi::log::LogStream(qi::log::error, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define qiLogError(...)        qi::log::LogStream(qi::log::error, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#ifdef NO_QI_FATAL
# define qiLogFatal(...)        if (false) qi::log::detail::NullStream(__VA_ARGS__).self()
#elif defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
# define qiLogFatal(...)        qi::log::LogStream(qi::log::fatal, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define qiLogFatal(...)        qi::log::LogStream(qi::log::fatal, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif


// enum level {
//   silent = 0,
//   fatal,
//   error,
//   warning,
//   info,
//   verbose,
//   debug
// };


namespace qi {
  namespace log {

    namespace detail {

      class NullStream {
      public:
        NullStream(const char *, ...)
        {
        }

        NullStream &self()
        {
          return *this;
        }

        template <typename T>
        NullStream& operator<<(const T &QI_UNUSED(val))
        {
          return self();
        }

        NullStream& operator<<(std::ostream& (*QI_UNUSED(f))(std::ostream&))
        {
          return self();
        }

      };

    };

    enum LogLevel {
        silent = 0,
        fatal,
        error,
        warning,
        info,
        verbose,
        debug
    };

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

    QI_API const char* logLevelToString(const qi::log::LogLevel verb);

    QI_API qi::log::LogLevel stringToLogLevel(const char* verb);

    QI_API void setVerbosity(const qi::log::LogLevel lv);

    QI_API qi::log::LogLevel verbosity();


    QI_API void setContext(int ctx);

    QI_API int context();

    QI_API void setSynchronousLog(bool sync);

    QI_API void addLogHandler(const std::string& name,
                              qi::log::logFuncHandler fct);

    QI_API void removeLogHandler(const std::string& name);

    QI_API void flush();

    class LogStream: public std::stringstream
    {
    public:
      LogStream(const LogLevel    level,
                const char        *file,
                const char        *function,
                const int         line,
                const char        *category)
        : _logLevel(level)
        , _category(category)
        , _file(file)
        , _function(function)
        , _line(line)
      {
      }

      LogStream(const LogLevel    level,
                const char        *file,
                const char        *function,
                const int         line,
                const char        *category,
                const char        *fmt, ...)
        : _logLevel(level)
        , _category(category)
        , _file(file)
        , _function(function)
        , _line(line)
      {
        char buffer[2048];
        va_list vl;
        va_start(vl, fmt);
       #ifdef _MSC_VER
        vsnprintf_s(buffer, 2048, 2047, fmt, vl);
       #else
        vsnprintf(buffer, 2048, fmt, vl);
       #endif
        buffer[2047] = 0;
        va_end(vl);
        *this << buffer;
      }

      ~LogStream()
      {
        qi::log::log(_logLevel, _category, this->str().c_str(), _file, _function, _line);
      }

      LogStream& self() {
        return *this;
      }

    private:
      LogLevel    _logLevel;
      const char *_category;
      const char *_file;
      const char *_function;
      int         _line;

      //avoid copy
      LogStream(const LogStream &rhs);
      LogStream &operator=(const LogStream &rhs);
    };
  }
}

#endif  // _LIBQI_QI_LOG_HPP_
