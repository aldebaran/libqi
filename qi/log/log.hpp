/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_LOG_LOG_H
#define QI_LOG_LOG_H

#include <string>
#include <sstream>
#include <cstdarg>
#include <qi/api.hpp>

// log macro
#define qiInfo(x)    qi::log::log(qi::log::info,    x, __FILE__, __FUNCTION__, __LINE__)
#define qiDebug(x)   qi::log::log(qi::log::debug,   x, __FILE__, __FUNCTION__, __LINE__)
#define qiWarning(x) qi::log::log(qi::log::warning, x, __FILE__, __FUNCTION__, __LINE__)
#define qiError(x)   qi::log::log(qi::log::error,   x, __FILE__, __FUNCTION__, __LINE__)
#define qiFatal(x)   qi::log::log(qi::log::fatal,   x, __FILE__, __FUNCTION__, __LINE__)

#define qisInfo      qi::log::LogStream(qi::log::info,    __FILE__,__FUNCTION__, __LINE__).self()
#define qisDebug     qi::log::LogStream(qi::log::debug,   __FILE__,__FUNCTION__, __LINE__).self()
#define qisWarning   qi::log::LogStream(qi::log::warning, __FILE__,__FUNCTION__, __LINE__).self()
#define qisError     qi::log::LogStream(qi::log::error,   __FILE__,__FUNCTION__, __LINE__).self()
#define qisFatal     qi::log::LogStream(qi::log::fatal,   __FILE__,__FUNCTION__, __LINE__).self()

namespace qi {

  namespace log {

  /**
   * Enum of accepted error types
   */
    enum LogLevel {
      silent = 0,
      fatal,
      error,
      warning,
      info,
      debug,
    };


    /*
     * log ptr
     */
    typedef void (*LogFunctionPtr)(const char         *file,
                                   const char         *fct,
                                   const int           line,
                                   const char          verb,
                                   const char         *fmt,
                                   va_list             vl);

    /** call this to make some log
     */
    QIAPI void log(const char       *file,
                   const char       *fct,
                   const int         line,
                   const char        verb,
                   const char       *fmt, ...);


    /** default log handler
     *  output log to console
     */
    QIAPI void defaultLogHandler(const char  *file,
                                 const char  *fct,
                                 const int    line,
                                 const char   verb,
                                 const char  *fmt,
                                 va_list      vl);

    /**
     * set the function called when we need to log something
     */
    QIAPI void setLogHandler(LogFunctionPtr p);

    class QIAPI LogStream: public std::stringstream
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
      LogStream(const LogLevel    &level,
                const char        *file,
                const char        *function,
                const int          line);

      ~LogStream();

      // necessary to have sinfo et al. work with an anonymous object
      LogStream& self() { return *this; }

    private:
      LogLevel    _logLevel;
      const char *_file;
      const char *_function;
      int         _line;
    };

  }
}

#endif

