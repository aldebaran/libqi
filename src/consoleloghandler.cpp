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

    class PrivateConsoleLogHandler
    {
    public:
      enum ConsoleAttr {
#ifndef _WIN32
        reset      = 0,
        bright,
        dim,
        blink,
        underline,
        reverse    = 7,
        hidden
#else
        reset      = 0,
        dim        = 0,
        reverse    = 7,
        bright,
        hidden
#endif
      };

      enum ConsoleColor {
#ifdef _WIN32
        black   = 0,
        darkblue,
        green,
        bluegray,
        brown,
        purple,
        whitegray = 7,
        gray,
        whiteblue,
        whitegreen,
        cyan,
        red,
        magenta,
        yellow,
        white
#else
        black   = 0,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        white,
        gray
#endif
      };


      void textColor(char fg, char bg = -1, char attr = -1) const;
      void textColorBG(char bg) const;
      void textColorAttr(char attr) const;
      void header(const LogLevel verb) const;

      bool _color;

#ifdef _WIN32
      void* _winScreenHandle;
#endif
    };

    void PrivateConsoleLogHandler::textColor(char fg, char bg, char attr) const
    {
      if (!_color)
        return;
#ifdef _WIN32
      if (fg == reset)
      {
        SetConsoleTextAttribute(_winScreenHandle, whitegray);
      }
      else
      {
        SetConsoleTextAttribute(_winScreenHandle, fg);
      }
      return;
#endif
      if (attr != -1 && bg != -1)
        printf("%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
      else if (bg != -1)
        printf("%c[%d;%dm", 0x1B, fg + 30, bg + 40);
      else
        printf("%c[%dm", 0x1B, fg + 30);
    }

    void PrivateConsoleLogHandler::textColorBG(char bg) const
    {
      if (!_color)
        return;
#ifdef _WIN32
      return;
#endif
      printf("%c[%dm", 0x1B, bg + 40);
    }

    void PrivateConsoleLogHandler::textColorAttr(char attr) const
    {
      if (!_color)
        return;

#ifdef _WIN32
      if (attr == reset)
      {
        SetConsoleTextAttribute(_winScreenHandle, whitegray);
      }
      else
      {
        SetConsoleTextAttribute(_winScreenHandle, attr);
      }
      return;
#endif
      printf("%c[%dm", 0x1B, attr);
    }

    void PrivateConsoleLogHandler::header(const LogLevel verb) const
    {
      //display log level
      textColorAttr(bright);
      if (verb == fatal)
        textColor(magenta);
      if (verb == error)
        textColor(red);
      if (verb == warning)
        textColor(yellow);
      if (verb == info)
        textColor(white);
      if (verb == verbose)
        textColorAttr(dim);
      if (verb == debug)
        textColorAttr(dim);
      printf("%s ", logLevelToString(verb));
      textColorAttr(reset);
      textColor(reset);
    }

    ConsoleLogHandler::ConsoleLogHandler()
      : _private(new PrivateConsoleLogHandler)
    {
      const char *color   = std::getenv("CLICOLOR");

#ifdef _WIN32
      _private->_winScreenHandle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
      _private->_color = 1;

      if (color)
        _private->_color = atoi(color) > 0 ? true: false;
      if (!isatty(1))
        _private->_color = 0;
    }

    void cutCat(const char* category, char* res)
    {
      int categorySize = strlen(category);
      if (categorySize < CATSIZEMAX)
      {
        memset(res, ' ', CATSIZEMAX);
        memcpy(res, category, strlen(category));
      }
      else
      {
        memset(res, '.', CATSIZEMAX);
        memcpy(res + 3, category + categorySize - CATSIZEMAX + 3, CATSIZEMAX - 3);
      }
      res[CATSIZEMAX] = '\0';
    }

    void ConsoleLogHandler::log(const LogLevel    verb,
                                const char       *category,
                                const char       *msg,
                                const char       *file,
                                const char       *fct,
                                const int         line)
    {
      if (verb > qi::log::getVerbosity())
      {
        return;
      }
      else
      {
        _private->header(verb);
        char fixedCategory[CATSIZEMAX + 1];
        fixedCategory[CATSIZEMAX] = '\0';
        cutCat(category, fixedCategory);
#ifndef WIN32
        _private->textColorAttr(_private->reset);
        _private->textColor(_private->gray);
#endif
        printf("%s: ", fixedCategory);
        if (qi::log::getContext())
        {
          printf("%s(%d) %s ", file, line, fct);
        }
        printf("%s", msg);
        fflush (stdout);
      }
      fflush (stdout);
    }
  }
}

