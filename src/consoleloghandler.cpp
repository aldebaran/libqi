/*
 *  Author(s):
 *  - Cedric Gestes <gestes@aldebaran-robotics.com>
 *  - Chris  Kilner <ckilner@aldebaran-robotics.com>
 *
 *  Copyright (C) 2010, 2011 Aldebaran Robotics
 */

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/log/consoleloghandler.hpp>

#ifdef _WIN32
# include <windows.h>
# include <io.h>
# define isatty _isatty
#else
# include <unistd.h>
#endif

#define CATSIZEMAX 16

namespace qi {
  namespace log {
    ConsoleLogHandler::ConsoleLogHandler()
      : _color(1)
    {
#ifdef WIN32
      _winScreenHandle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
      const char *verbose = std::getenv("VERBOSE");
      const char *context = std::getenv("CONTEXT");
      const char *color   = std::getenv("CLICOLOR");


      if (verbose)
        qi::log::setVerbosity((LogLevel)atoi(verbose));
      if (context)
        qi::log::setContext(atoi(context) > 0 ? true: false);
      if (color)
        _color = atoi(color) > 0 ? true: false;
      if (!isatty(1))
        _color = 0;
    }

    char* cutCat(const char* category, char* res)
    {
      if (strlen(category) < CATSIZEMAX)
      {
        memset(res, ' ', CATSIZEMAX);
        strncpy(res, category, strlen(category));
      }
      else
      {
        memset(res, '.', CATSIZEMAX);
        strncpy(res, category, CATSIZEMAX - 3);
      }
      res[CATSIZEMAX] = '\0';
    }

    void ConsoleLogHandler::log(const LogLevel    verb,
                                const char       *file,
                                const char       *fct,
                                const char       *category,
                                const int         line,
                                const char       *msg)
    {
      if (verb > qi::log::getVerbosity())
      {
        return;
      }
      else
      {
        header(verb);
        char fixedCategory[CATSIZEMAX + 1];
        cutCat(category, fixedCategory);
        if (qi::log::getContext() != 0)
        {
          printf("%s: %s(%d) %s %s", fixedCategory, file, line, fct, msg);
          fflush (stdout);
        }
        else
        {
          printf("%s: ", fixedCategory);
          if (qi::log::getContext())
          {
            textColorAttr(reset);
            textColor(gray);
            printf("%s(%d) %s ", file, line, fct);
            textColorAttr(reset);
          }
          printf("%s", msg);
          fflush (stdout);
        }
      }
      fflush (stdout);
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

    void ConsoleLogHandler::header(const LogLevel verb) const
    {
      //display log level
      textColorAttr(bright);
      if (verb == fatal)
        textColor(magenta);
      if (verb == error)
        textColor(red);
      if (verb == warning)
        textColor(yellow);
      if (verb == verbose)
        textColor(green);
      if (verb == debug)
        textColorAttr(dim);
      printf("%s ", logLevelToString(verb));
      textColorAttr(reset);
    }

  }
}

