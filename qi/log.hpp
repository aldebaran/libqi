#pragma once
/*
 *  Author(s):
 *  - Chris  Kilner <ckilner@aldebaran-robotics.com>
 *  - Cedric Gestes <gestes@aldebaran-robotics.com>
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 *
 *  Copyright (C) 2010, 2011 Aldebaran Robotics
 */

/** @file
 *  @brief convenient log macro
 */


#ifndef LOG_HPP_
# define LOG_HPP_

# include <map>
# include <string>
# include <iostream>
# include <sstream>
# include <cstdarg>
# include <cstdio>

#include <boost/function.hpp>

#include <qi/api.hpp>

//should not be compiled in release. Not useful for user.
#ifdef NO_QI_DEBUG
# define qiLogDebug(...)
#else
# define qiLogDebug(...)        qi::log::LogStream(qi::log::debug, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#ifdef NO_QI_VERBOSE
# define qiLogVerbose(...)
#else
# define qiLogVerbose(...)      qi::log::LogStream(qi::log::verbose, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#ifdef NO_QI_INFO
# define qiLogInfo(...)
#else
# define qiLogInfo(...)         qi::log::LogStream(qi::log::info, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#ifdef NO_QI_WARNING
# define qiLogWarning(...)
#else
# define qiLogWarning(...)      qi::log::LogStream(qi::log::warning, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#ifdef NO_QI_ERROR
# define qiLogError(...)
#else
# define qiLogError(...)        qi::log::LogStream(qi::log::error, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#ifdef NO_QI_FATAL
# define qiLogFatal(...)
#else
# define qiLogFatal(...)        qi::log::LogStream(qi::log::fatal, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

namespace qi {
  namespace log {
    class LogStream;

    /**
     * Log Verbosity
     */
    enum QI_API LogLevel {
        silent = 0,
        fatal,
        error,
        warning,
        info,
        verbose,
        debug,
        };



    typedef boost::function6<void,
                             const qi::log::LogLevel,
                             const char*,
                             const char*,
                             const char*,
                             int,
                             const char*> logFuncHandler;

    /**
     * call this to make some log
     */
    QI_API void log(const LogLevel    verb,
                    const char       *file,
                    const char       *fct,
                    const char       *category,
                    const int         line,
                    const char       *msg);


    QI_API const char* logLevelToString(const LogLevel verb);
    QI_API const LogLevel stringToLogLevel(const char* verb);


    static LogLevel _glVerbosity = qi::log::info;
    QI_API void setVerbosity(const LogLevel lv);
    QI_API LogLevel getVerbosity();

    static bool _glContext = 0;
    QI_API void setContext(bool ctx);
    QI_API bool getContext();

    QI_API void addLogHandler(logFuncHandler fct, const std::string& name);
    QI_API void removeLogHandler(const std::string& name);

    // HEADER Only. (no pimpl)
    class LogStream: public std::stringstream
    {
    public:

      /**
       * LogStream. Will log at object destruction
       * @param level { debug = 6, verbose=5, info = 4, warning = 3, error = 2, fatal = 1, silent = 0 }
       * @param file __FILE__
       * @param function __FUNCTION__
       * @param line __LINE__
       * @param category log category
       */
      LogStream(const LogLevel     level,
                const char        *file,
                const char        *function,
                const int          line,
                const char        *category)
        : _logLevel(level)
        , _file(file)
        , _function(function)
        , _line(line)
        , _category(category)
      {
      }

      LogStream(const LogStream &rhs)
        : _logLevel(rhs._logLevel)
        , _category(rhs._category)
        , _file(rhs._file)
        , _function(rhs._function)
        , _line(rhs._line)
      {
      }

      const LogStream &operator=(const LogStream &rhs)
      {
        _logLevel = rhs._logLevel;
        _file     = rhs._file;
        _function = rhs._function;
        _line     = rhs._line;
        _category = rhs._category;
      }

      LogStream(const LogLevel     level,
                const char        *file,
                const char        *function,
                const int          line,
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
        qi::log::log(_logLevel, _file, _function, _category, _line, this->str().c_str());
      }

      // necessary to have sinfo et al. work with an anonymous object
      LogStream& self() {
        return *this;
      }

    private:
      LogLevel    _logLevel;
      const char *_category;
      const char *_file;
      const char *_function;
      int         _line;
    };
  }
}

#endif // !LOG_HPP_
