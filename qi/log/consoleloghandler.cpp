#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <cstdlib>
#include <cstring>
#include <cstdio>


#include <qi/log/consoleloghandler.hpp>

#ifdef _WIN32
# include <windows.h>
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
#ifdef WIN32
    _winScreenHandle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
      const char *verbose = std::getenv("VERBOSE");
      const char *context = std::getenv("CONTEXT");
      const char *color   = std::getenv("CLICOLOR");

      if (verbose)
        _verbosity = (LogLevel)atoi(verbose);
      if (context)
        _context = atoi(context) > 0 ? true: false;
      if (color)
        _color = atoi(color)> 0 ? true: false;
      if (!isatty(1))
        _color = 0;
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
      {
        boost::mutex::scoped_lock scopedLock(_mutex);
        header(verb, file, fct, line);
        vprintf(fmt, vl);
      }
    }

    void ConsoleLogHandler::textColor(char fg, char bg, char attr) const
    {
      if (!_color)
        return;
#ifdef _WIN32
      SetConsoleTextAttribute(_winScreenHandle, fg);
      return;
#endif
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
#ifdef _WIN32
      return;
#endif
      printf("%c[%dm", 0x1B, bg + 40);
    }

    void ConsoleLogHandler::textColorAttr(char attr) const
    {
      if (!_color)
        return;

#ifdef _WIN32
      if (attr == reset) {
        SetConsoleTextAttribute(_winScreenHandle, white);
      }
      return;
#endif
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
        textColor(gray);
        printf("%s(%d) %s\n", file, line, fct);
        textColorAttr(reset);
      }

      //display log level
      textColorAttr(bright);
      if (verb == fatal)
        textColor(magenta);
      if (verb == error)
        textColor(red);
      if (verb == warning)
        textColor(yellow);
      if (verb == debug)
        textColorAttr(dim);
      printf("%s ", logLevelToString(verb));
      textColorAttr(reset);
    }

  }
}

