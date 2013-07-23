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
        max_color,
      };
#endif
      static int InvertConsoleColor[max_color];

      PrivateConsoleLogHandler();
      void textColorBG(char bg) const;
      void textColorFG(char fg) const;
      void textColorAttr(char attr) const;
      void header(const qi::LogLevel verb, bool verbose = true) const;
      PrivateConsoleLogHandler::ConsoleColor colorForHeader(const qi::LogLevel verb) const;
      void coloredLog(const qi::LogLevel verb, const qi::os::timeval date,
                      const char *category,
                      const char *msg,
                      const char *file,
                      const char *fct,
                      const int   line);

      bool _color;
      bool _useLock;
      boost::mutex _mutex;

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

    PrivateConsoleLogHandler::ConsoleColor PrivateConsoleLogHandler::colorForHeader(const qi::LogLevel verb) const
    {
      if (verb == LogLevel_Fatal)
        return magenta;
      if (verb == LogLevel_Error)
        return red;
      if (verb == LogLevel_Warning)
        return yellow;
      if (verb == LogLevel_Info)
        return blue;
      if (verb == LogLevel_Verbose)
        return green;
      if (verb == LogLevel_Debug)
        return white;
      return white;
    }

    void PrivateConsoleLogHandler::header(const qi::LogLevel verb, bool verbose) const
    {
      //display log level
      textColorAttr(reset);
      textColorFG(colorForHeader(verb));
      printf("%s ", logLevelToString(verb, verbose));
      textColorAttr(reset);
    }

    PrivateConsoleLogHandler::PrivateConsoleLogHandler()
      : _color(true)
      , _useLock(qi::os::getenv("QI_LOG_NOLOCK").empty())
#ifdef _WIN32
      , _winScreenHandle(GetStdHandle(STD_OUTPUT_HANDLE))
#endif
    {
    }

    ConsoleLogHandler::~ConsoleLogHandler()
    {
      delete _p;
    }

    ConsoleLogHandler::ConsoleLogHandler()
      : _p(new PrivateConsoleLogHandler)
    {
      updateColor();
    }

    void ConsoleLogHandler::updateColor()
    {
      const char *color   = std::getenv("CLICOLOR");

      if (color && atoi(color) == 0) {
        _p->_color = 0;
        return;
      }
      if (qi::log::color() == LogColor_Never)
        _p->_color = 0;
      if (qi::log::color() == LogColor_Auto) {
        if (qi::os::isatty())
          _p->_color = 1;
        else
          _p->_color = 0;
      }
      if (qi::log::color() == LogColor_Always)
        _p->_color = 1;
    }

    int stringToColor(const char *str)
    {
      int sum = 0;
      int i = 0;
      while (str[i] != '\0') {
        sum += str[i++];
      }
      return sum % (qi::log::PrivateConsoleLogHandler::max_color - 1) + 1;
    }

    int intToColor(int nbr)
    {
      return nbr % qi::log::PrivateConsoleLogHandler::max_color;
    }

    void PrivateConsoleLogHandler::coloredLog(const qi::LogLevel verb,
                    const qi::os::timeval date,
                    const char *category,
                    const char *msg,
                    const char *file,
                    const char *fct,
                    const int   line)
    {
      int context = qi::log::context();

      boost::mutex::scoped_lock scopedLock(_mutex, boost::defer_lock_t());
      if (_useLock)
        scopedLock.lock();

      if (context & qi::LogContextAttr_Verbosity) {
        header(verb);
      }
      if (context & qi::LogContextAttr_ShortVerbosity) {
        header(verb, false);
      }

      if (context & qi::LogContextAttr_Tid) {
        int tidColor = intToColor(qi::os::gettid());
        textColorBG(tidColor);
        textColorFG(InvertConsoleColor[tidColor]);
        printf("%s", qi::detail::tidToString().c_str());
        textColorAttr(reset);
        printf(" ");
      }

      if (context & qi::LogContextAttr_Date)
        printf("%s ", qi::detail::dateToString(date).c_str());

      if (context & qi::LogContextAttr_Category) {
        textColorFG(stringToColor(category));
        printf("%s: ", category);
        textColorAttr(qi::log::PrivateConsoleLogHandler::reset);
      }
      if (context & qi::LogContextAttr_File) {
        printf("%s", file);
        if (line != 0)
          printf("(%i)", line);
        printf(" ");
      }
      if (context & qi::LogContextAttr_Function)
        printf("%s() ", fct);
      if (context & qi::LogContextAttr_Return)
        printf("\n");
      std::string ss = msg;
      ss.reserve(qi::detail::rtrim(msg));
      printf("%s\n", ss.c_str());
    }

    void ConsoleLogHandler::log(const qi::LogLevel    verb,
                                const qi::os::timeval date,
                                const char            *category,
                                const char            *msg,
                                const char            *file,
                                const char            *fct,
                                const int             line)
    {
#ifndef _WIN32
      _p->textColorAttr(_p->reset);
      _p->textColorFG(_p->white);
#endif
      if (_p->_color) {
        _p->coloredLog(verb, date, category, msg, file, fct, line);
        return;
      }

      std::string logline = qi::detail::logline(qi::log::context(), date, category, msg, file, fct, line, verb);
      printf("%s", logline.c_str());
      fflush(stdout);
    }

  }
}

#ifdef _WIN32
int qi::log::PrivateConsoleLogHandler::InvertConsoleColor[] = {
        white, // black   = 0,
        white, // darkblue,
        black, // green,
        white, // bluegray,
        black, // brown,
        black, // purple,
        black, // NONE
        black, // whitegray = 7,
        black, // gray,
        black, // whiteblue,
        black, // whitegreen,
        white, // blue,
        black, // red,
        black, // magenta,
        black, // yellow,
        black // white,
        // max_color,
};
#else
int qi::log::PrivateConsoleLogHandler::InvertConsoleColor[] = {
        white, //black   = 0,
        green, //red,
        black, //green,
        black, //yellow,
        white, //blue,
        yellow, //magenta,
        black, //cyan,
        black, //white,
        //max_color,
};
#endif
