#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_LOG_HPP_
#define _QI_LOG_HPP_

#include <string>
#include <sstream>
#include <cstdarg>
#include <qi/api.hpp>

#ifdef NO_QI_DEBUG
# define qiDebug(...)
# define qisDebug
#else
# define qiDebug(...)   qi::log::log(qi::log::debug,         __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
# define qisDebug       qi::log::LogStream(qi::log::debug,   __FILE__, __FUNCTION__, __LINE__).self()
#endif

#ifdef NO_QI_INFO
# define qiInfo(...)
# define qisInfo
#else
#define qiInfo(...)    qi::log::log(qi::log::info,          __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define qisInfo        qi::log::LogStream(qi::log::info,    __FILE__, __FUNCTION__, __LINE__).self()
#endif

#ifdef NO_QI_WARNING
# define qiWarning(...)
# define qisWarning
#else
# define qiWarning(...) qi::log::log(qi::log::warning,       __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
# define qisWarning     qi::log::LogStream(qi::log::warning, __FILE__, __FUNCTION__, __LINE__).self()
#endif

#ifdef NO_QI_ERROR
# define qiError(...)
# define qisError
#else
# define qiError(...)   qi::log::log(qi::log::error,         __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
# define qisError       qi::log::LogStream(qi::log::error,   __FILE__, __FUNCTION__, __LINE__).self()
#endif

#ifdef NO_QI_FATAL
# define qiFatal(...)
# define qisFatal
#else
# define qisFatal       qi::log::LogStream(qi::log::fatal, __FILE__, __FUNCTION__, __LINE__).self()
# define qiFatal(...)   qi::log::log(qi::log::fatal,       __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif

namespace qi {
  namespace log {
    /**
     * Log Verbosity
     */
    enum QI_API LogLevel {
      silent = 0,
      fatal,
      error,
      warning,
      info,
      debug,
    };


    /**
     * log handler ptr
     */
    typedef void (*LogFunctionPtr) (qi::log::LogLevel,
                                    const char *,
                                    const char *,
                                    int,
                                    const char *,
                                    va_list,
                                    void *);

    /**
     * call this to make some log
     */
    QI_API void log(const LogLevel    verb,
                    const char       *file,
                    const char       *fct,
                    const int         line,
                    const char       *fmt, ...);

    /**
     * call this to make some log
     */
    QI_API void log(const LogLevel    verb,
                    const char       *file,
                    const char       *fct,
                    const int         line,
                    const char       *fmt,
                    va_list           vl);


    /** our default console log handler */
    QI_API void consoleLogHandler(const LogLevel    verb,
                                  const char       *file,
                                  const char       *fct,
                                  const int         line,
                                  const char       *fmt,
                                  va_list           vl,
                                  void             *data);
    /**
     * set the function called when we need to log something
     */
    QI_API void setLogHandler(LogFunctionPtr p, void *data);

    QI_API const char *logLevelToString(const LogLevel verb);


    class LogStream: public std::stringstream
    {
    public:

      /**
       * LogStream. Will log at object destruction
       * @param pLevel { debug = 5, info = 4, warning = 3, error = 2, fatal = 1, silent = 0 }
       * @param pFile __FILE__
       * @param pFunction __FUNCTION__
       * @param pLine __LINE__
       * @param pLogEntry log stream constructor and destructor (for scoped log)
       */
      LogStream(const LogLevel     level,
                const char        *file,
                const char        *function,
                const int          line) :
        _logLevel(level),
        _file(file),
        _function(function),
        _line(line)
      {
      }

      ~LogStream() {
        log(_logLevel, _file, _function, _line, this->str().c_str());
      }

      // necessary to have sinfo et al. work with an anonymous object
      LogStream& self() {
        return *this;
      }

    private:
      LogLevel    _logLevel;
      const char *_file;
      const char *_function;
      int         _line;
    };

  }
}

#endif  // _QI_LOG_HPP_
