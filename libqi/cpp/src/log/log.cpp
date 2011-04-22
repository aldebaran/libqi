/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include "src/log/consoleloghandler.hpp"

#include <boost/bind.hpp>

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace qi {
  namespace log {

    static LogFunctionPtr    gLogHandler     = 0;
    static void             *gLogHandlerData = 0;
    static ConsoleLogHandler gConsoleLogHandler;


    void consoleLogHandler(const LogLevel    verb,
                           const char       *file,
                           const char       *fct,
                           const int         line,
                           const char       *fmt,
                           va_list           vl,
                           void             *data) {
      gConsoleLogHandler.log(verb, file, fct, line, fmt, vl);
    }

    static class LogHandlerInit {
    public:
      LogHandlerInit() {
        setLogHandler(consoleLogHandler, 0);
        //setLogHandler(boost::bind<void>(&ConsoleLogHandler::log, &gConsoleLogHandler, _1, _2, _3, _4, _5, _6));
      }
    } gLogHandlerInit;


    void setLogHandler(LogFunctionPtr p, void *data) {
      gLogHandler     = p;
      gLogHandlerData = data;
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
        gLogHandler(verb, file, fct, line, fmt, vl, gLogHandlerData);
      }
      else {
        printf("[MISSING Logger]: ");
        vprintf(fmt, vl);
      }
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

    class PrivateLogStream {
      LogLevel    _logLevel;
      const char *_file;
      const char *_function;
      int         _line;
    };


  }
}

