/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <sstream>
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
#ifndef _WIN32
      enum ConsoleAttr {
        reset      = 0,
        bright,
        dim,
        underline  = 4,
        blink,
        reverse    = 7
      };
#else
      enum ConsoleAttr {
        reset      = 0,
        dim        = 0,
        reverse    = 7,
        bright
      };
#endif

#ifdef _WIN32
      enum ConsoleColor {
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
      };
#else
      enum ConsoleColor {
        black   = 0,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        white,
        gray
      };
#endif


      void textColorBG(char bg) const;
      void textColorFG(char fg) const;
      void textColorAttr(char attr) const;
      void header(const LogLevel verb) const;

      bool _color;

#ifdef _WIN32
      void* _winScreenHandle;
#endif
    };


#ifndef _WIN32
    void PrivateConsoleLogHandler::textColorAttr(char attr) const
    {
      if (!_color)
        return;

      printf("%c[%dm", 0x1B, attr);
    }

    void PrivateConsoleLogHandler::textColorBG(char bg) const
    {
      if (!_color)
        return;

      printf("%c[%dm", 0x1B, bg + 40);
    }

    void PrivateConsoleLogHandler::textColorFG(char fg) const
    {
      if (!_color)
        return;

      printf("%c[%dm", 0x1B, fg + 30);
    }

#else
    void PrivateConsoleLogHandler::textColorBG(char bg) const
    {
      return;
    }

    void PrivateConsoleLogHandler::textColorAttr(char attr) const
    {
      textColorFG(attr);
      return;
    }

    void PrivateConsoleLogHandler::textColorFG(char fg) const
    {
      if (!_color)
        return;

      if (fg == reset)
      {
        SetConsoleTextAttribute(_winScreenHandle, whitegray);
      }
      else
      {
        SetConsoleTextAttribute(_winScreenHandle, fg);
      }
      return;
    }
#endif

    void PrivateConsoleLogHandler::header(const LogLevel verb) const
    {
      //display log level
      textColorAttr(reset);
      if (verb == fatal)
        textColorFG(magenta);
      if (verb == error)
        textColorFG(red);
      if (verb == warning)
        textColorFG(yellow);
      if (verb == info)
        textColorAttr(reset);
      if (verb == verbose)
        textColorAttr(dim);
      if (verb == debug)
        textColorAttr(dim);
      printf("%s ", logLevelToString(verb));
      textColorAttr(reset);
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

    void ConsoleLogHandler::log(const LogLevel        verb,
                                const qi::os::timeval date,
                                const char            *category,
                                const char            *msg,
                                const char            *file,
                                const char            *fct,
                                const int             line)
    {
      if (verb > qi::log::verbosity())
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
        _private->textColorFG(_private->gray);
#endif

        std::stringstream ss;
        ss << date.tv_sec << "." << date.tv_usec;

        int ctx = qi::log::context();
        switch (ctx)
        {
        case 1:
          printf("%s: ", fixedCategory);
          break;
        case 2:
          printf("%s ", ss.str().c_str());
          break;
        case 3:
          printf("%s(%d) ", file, line);
          break;
        case 4:
          printf("%s %s: ", ss.str().c_str(), fixedCategory);
          break;
        case 5:
          printf("%s %s(%d) ", ss.str().c_str(), file, line);
          break;
        case 6:
          printf("%s: %s(%d) ", fixedCategory, file, line);
          break;
        case 7:
          printf("%s %s: %s(%d) %s ", ss.str().c_str(), fixedCategory, file, line, fct);
          break;
        default:
          break;
        }
        printf("%s", msg);
        fflush (stdout);
      }
      fflush (stdout);
    }
  }
}

