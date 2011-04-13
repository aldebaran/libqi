/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qi/log/log.hpp>
#include <qi/log/consoleloghandler.hpp>

#include <boost/bind.hpp>

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace qi {
  namespace log {

    static LogFunctionPtr    gLogHandler = 0;
    static ConsoleLogHandler gConsoleLogHandler;

    static class LogHandlerInit {
    public:
      LogHandlerInit() {
        //setLogHandler(defaultLogHandler);
        setLogHandler(boost::bind<void>(&ConsoleLogHandler::log, &gConsoleLogHandler, _1, _2, _3, _4, _5, _6));
      }
    } gLogHandlerInit;


    void setLogHandler(LogFunctionPtr p) {
      gLogHandler = p;
    }

    void log(const LogLevel    verb,
             const char       *file,
             const char       *fct,
             const int         line,
             const char       *fmt, ...)
    {
      va_list   vl;
      va_start(vl, fmt);
      log(verb, file, fct, line, fmt, vl);
      va_end(vl);
    }

    void log(const LogLevel    verb,
             const char       *file,
             const char       *fct,
             const int         line,
             const char       *fmt,
             va_list           vl)
    {
      if (gLogHandler) {
        gLogHandler(verb, file, fct, line, fmt, vl);
      }
      else {
        printf("[MISSING Logger]: ");
        vprintf(fmt, vl);
      }
    }

    void defaultLogHandler(const LogLevel    verb,
                           const char       *file,
                           const char       *fct,
                           const int         line,
                           const char       *fmt,
                           va_list           vl)
    {
      printf(logLevelToString(verb));
      printf("%s:%s:%d:", file, fct, line);
      vprintf(fmt, vl);
    }

    const char *logLevelToString(const LogLevel verb) {
      static const char *sverb[] = {
        "[SILENT]", // never shown
        "[FATAL]",
        "[ERROR]",
        "[WARN ]",
        "[INFO ]",
        "[DEBUG]"
      };
      return sverb[verb];
    }

    LogStream::LogStream(const LogLevel     level,
                         const char        *file,
                         const char        *function,
                         int                line)
      : _logLevel(level),
        _file(file),
        _function(function),
        _line(line)
    {
    }

    LogStream::~LogStream()
    {
      log(_logLevel, _file, _function, _line, this->str().c_str());
    }

  }
}

