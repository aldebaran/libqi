/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/log/log.hpp>

#include <sstream>
#include <iostream>
#include <cstdarg>
# include <cstdio>
# include <cstring>

namespace qi {
  namespace log {

    static LogFunctionPtr gLogHandler = 0;


    static class LogHandlerInit {
    public:
      LogHandlerInit() {
        setLogHandler(defaultLogHandler);
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
      if (gLogHandler) {
        va_list   vl;
        va_start(vl, fmt);
        gLogHandler(verb, file, fct, line, fmt, vl);
        va_end(vl);
      }
      else {
        va_list   vl;
        va_start(vl, fmt);
        printf("[MISSING Logger]: ");
        vprintf(fmt, vl);
        va_end(vl);
      }
    }

    void defaultLogHandler(const LogLevel    verb,
                           const char       *file,
                           const char       *fct,
                           const int         line,
                           const char       *fmt,
                           va_list           vl)
    {
      static const char *sverb[] = { "",
                                     "FATAL  :",
                                     "ERROR  :",
                                     "WARNING:",
                                     "",
                                     "DEBUG  :"
                                   };
      printf(sverb[verb]);
      printf("%s:%s:%d:", file, fct, line);
      vprintf(fmt, vl);
      //va_end(vl);
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

