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

    void log(const char       *file,
             const char       *fct,
             const int         line,
             const char        verb,
             const char       *fmt, ...)
    {
      if (gLogHandler) {
        va_list   vl;
        va_start(vl, fmt);
        gLogHandler(file, fct, line, verb, fmt, vl);
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

    void defaultLogHandler(const char       *file,
                           const char       *fct,
                           const int         line,
                           const char        verb,
                           const char       *fmt,
                           va_list           vl)
    {
      //va_list     vl;
      int         len;
      const char *b = file;

      //va_start(vl, fmt);
      len = strlen(file);
      if (len > 20)
        b = file + (len - 20);
      printf("%.20s:%s:%d: ", b, fct, line);
      vprintf(fmt, vl);
      //va_end(vl);
    }

    LogStream::LogStream(const LogLevel    &level,
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
      log(_file, _function, _line, _logLevel, this->str().c_str());
    }

  }
}

