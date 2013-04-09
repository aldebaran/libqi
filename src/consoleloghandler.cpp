/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include "log_p.hpp"
#include <qi/log/consoleloghandler.hpp>
#include <boost/thread.hpp>

#ifdef _WIN32
# include <windows.h>
# include <io.h>
#else
# include <unistd.h>
#endif

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
        blue,
        red,
        magenta,
        yellow,
        white,
        max_color,
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
        gray,
        max_color,
      };
#endif


      void textColorBG(char bg) const;
      void textColorFG(char fg) const;
      void textColorAttr(char attr) const;
      void header(const LogLevel verb) const;
      PrivateConsoleLogHandler::ConsoleColor colorForHeader(const LogLevel verb) const;
      void coloredLog(const LogLevel verb, const qi::os::timeval date,
                      const char *category,
                      const char *msg,
                      const char *file,
                      const char *fct,
                      const int   line);

      bool _color;

#ifdef _WIN32
      void* _winScreenHandle;
#endif
    };


#ifndef _WIN32
    void printAttribute(char attr)
    {
      printf("%c[%dm", 0x1B, attr);
    }

    void PrivateConsoleLogHandler::textColorAttr(char attr) const
    {
      if (!_color)
        return;

      printAttribute(attr);
    }

    void PrivateConsoleLogHandler::textColorBG(char bg) const
    {
      if (!_color)
        return;

      printAttribute(bg+40);
    }

    void PrivateConsoleLogHandler::textColorFG(char fg) const
    {
      if (!_color)
        return;

      printAttribute(fg+30);
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

    PrivateConsoleLogHandler::ConsoleColor PrivateConsoleLogHandler::colorForHeader(const LogLevel verb) const
    {
      if (verb == fatal)
        return magenta;
      if (verb == error)
        return red;
      if (verb == warning)
        return yellow;
      if (verb == info)
        return blue;
      if (verb == verbose)
        return green;
      if (verb == debug)
        return gray;
      return white;
    }

    void PrivateConsoleLogHandler::header(const LogLevel verb) const
    {
      //display log level
      textColorAttr(reset);
      textColorFG(colorForHeader(verb));
      printf("%s ", logLevelToString(verb));
      textColorAttr(reset);
    }

    ConsoleLogHandler::~ConsoleLogHandler()
    {
      delete _private;
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
      if (!qi::os::isatty())
        _private->_color = 0;
    }

    int stringToColor(const char *str)
    {
      int sum = 0;
      int i = 0;
      while (str[i] != '\0') {
        sum += str[i++];
      }
      return sum % qi::log::PrivateConsoleLogHandler::max_color;
    }

    int intToColor(int nbr)
    {
      return nbr % qi::log::PrivateConsoleLogHandler::max_color;
    }

    void PrivateConsoleLogHandler::coloredLog(const LogLevel verb,
                    const qi::os::timeval date,
                    const char *category,
                    const char *msg,
                    const char *file,
                    const char *fct,
                    const int   line)
    {
      int categories = qi::detail::categoriesFromContext();

      static boost::mutex mutex;
      boost::mutex::scoped_lock scopedLock(mutex, boost::defer_lock_t());
      static bool useLock = qi::os::getenv("QI_LOG_NOTUSELOCK").empty();
      if (useLock)
        scopedLock.lock();
      if (categories & qi::detail::LOG_VERBOSITY) {
        header(verb);
      }
      if (categories & qi::detail::LOG_DATE)
        printf("%s ", qi::detail::dateToString(date).c_str());
      if (categories & qi::detail::LOG_TID) {
        textColorBG(intToColor(qi::os::gettid()));
        printf("%s", qi::detail::tidToString().c_str());
        textColorAttr(reset);
        printf(" ");
      }
      if (categories & qi::detail::LOG_CATEGORY) {
        std::string cat = qi::detail::categoryToFixedCategory(category);
        textColorFG(stringToColor(cat.c_str()));
        printf("%s", cat.c_str());
        textColorAttr(qi::log::PrivateConsoleLogHandler::reset);
        printf(": ");
      }
      if (categories & qi::detail::LOG_FILE) {
        printf("%s", file);
        if (line != 0)
          printf("(%i)", line);
        printf(" ");
      }
      if (categories && qi::detail::LOG_FUNCTION)
        printf("%s ", fct);
      std::string ss = msg;
      ss.reserve(qi::detail::rtrim(msg));
      printf("%s\n", ss.c_str());
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
#ifndef _WIN32
        _private->textColorAttr(_private->reset);
        _private->textColorFG(_private->gray);
#endif
        if (_private->_color) {
          _private->coloredLog(verb, date, category, msg, file, fct, line);
          return;
        } else {
          _private->header(verb);

          std::string logline = qi::detail::logline(date, category, msg, file, fct, line);
          printf("%s", logline.c_str());
          fflush(stdout);
        }
      }
      fflush(stdout);
    }
  }
}

