#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_LOG_CONSOLELOGHANDLER_HPP_
#define _QI_LOG_CONSOLELOGHANDLER_HPP_

#include <cstdarg>
#include <qi/log/log.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {
  namespace log {

    /// <summary> Can print colored logs to the console </summary>
    class ConsoleLogHandler {
    public:
      ConsoleLogHandler();

      void log(const LogLevel    verb,
               const char       *file,
               const char       *fct,
               const int         line,
               const char       *fmt,
               va_list           vl);
    protected:

      boost::mutex _mutex;

      enum ConsoleAttr {
        reset      = 0,
        bright,
        dim,
        underline,
        blink,
        reverse    = 7,
        hidden
      };

      enum ConsoleColor {
#ifdef _WIN32
        black      = 0,
        blue = 9,
        green = 10,
        cyan = 11,
        red = 12,
        magenta =13,
        yellow = 14,
        white = 15,
        gray = 8
#else
        black      = 0,
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
      void header(const LogLevel verb,
                  const char   *file,
                  const char   *fct,
                  const int     line) const;

    protected:
      qi::log::LogLevel _verbosity;
      bool              _context;
      bool              _color;

#ifdef _WIN32
      void* _winScreenHandle;
#endif
    };
  }
}


#endif  // _QI_LOG_CONSOLELOGHANDLER_HPP_
