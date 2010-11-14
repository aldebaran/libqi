/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <qi/log/consoleloghandler.hpp>


namespace qi {
  namespace log {

    ConsoleLogHandler::ConsoleLogHandler()
      : _verbosity(qi::log::info)
    {
      const char *env = std::getenv("VERBOSE");
      if (env)
        _verbosity = (LogLevel)atoi(env);
    }

    void ConsoleLogHandler::log(const char       *file,
                                const char       *fct,
                                const int         line,
                                const char        verb,
                                const char       *fmt,
                                va_list           vl)
    {
      int         len;
      const char *b = file;

      len = strlen(file);
      if (len > 20)
        b = file + (len - 20);
      printf("DEFFF: %.20s:%s:%d: ", b, fct, line);
      vprintf(fmt, vl);
    }

  }
}

