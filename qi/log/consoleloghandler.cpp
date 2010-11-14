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

#ifdef _WIN32
# include <io.h>
# define isatty _isatty
#else
# include <unistd.h>
#endif

namespace qi {
  namespace log {

    ConsoleLogHandler::ConsoleLogHandler()
      : _verbosity(qi::log::info),
        _context(0),
        _color(1)
    {
      const char *verbose = std::getenv("VERBOSE");
      const char *context = std::getenv("CONTEXT");
      const char *color   = std::getenv("CLICOLOR");

      if (verbose)
        _verbosity = (LogLevel)atoi(verbose);
      if (context)
        _context = atoi(context);
      if (color)
        _color = atoi(color);
      if (!isatty(1))
        _color = 0;
      printf("Verb: %d, Ctx: %d, Color: %d\n", _verbosity, _context, _color);
    }

    void ConsoleLogHandler::log(const LogLevel    verb,
                                const char       *file,
                                const char       *fct,
                                const int         line,
                                const char       *fmt,
                                va_list           vl)
    {
      if (verb > _verbosity)
        return;
      header(verb, file, fct, line);
      vprintf(fmt, vl);
    }

    void ConsoleLogHandler::textColor(char fg, char bg, char attr) const
    {
      if (!_color)
        return;
      if (attr != -1 && bg != -1)
        printf("%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
      else if (bg != -1)
        printf("%c[%d;%dm", 0x1B, fg + 30, bg + 40);
      else
        printf("%c[%dm", 0x1B, fg + 30);
    }

    void ConsoleLogHandler::textColorBG(char bg) const
    {
      if (!_color)
        return;
      printf("%c[%dm", 0x1B, bg + 40);
    }

    void ConsoleLogHandler::textColorAttr(char attr) const
    {
      if (!_color)
        return;
      printf("%c[%dm", 0x1B, attr);
    }

    void ConsoleLogHandler::header(const LogLevel verb,
                                   const char   *file,
                                   const char   *fct,
                                   const int     line) const
    {
      if (_context)
      {
        textColorAttr(reset);
        printf("%s", file);
        //textColorAttr(dim);
        textColor(blue);
        printf(":");
        textColor(green);
        printf("%s", fct);
        textColor(blue);
        printf(":");
        textColor(magenta);
        printf("%d", line);
        textColor(blue);
        printf(":");
        textColorAttr(reset);
        printf("\n");
      }

      //display log level
      textColorAttr(bright);
      if (verb == fatal || verb == error)
        textColor(red);
      if (verb == warning)
        textColor(yellow);
      if (verb == debug)
        textColorAttr(dim);
      printf("%-7s: ", logLevelToString(verb));
      textColorAttr(reset);
    }

  }
}

