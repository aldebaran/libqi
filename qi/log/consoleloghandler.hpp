#pragma once
/*
 *  Author(s):
 *  - Cedric Gestes <gestes@aldebaran-robotics.com>
 *  - Chris  Kilner <ckilner@aldebaran-robotics.com>
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 *
 *  Copyright (C) 2010, 2011 Aldebaran Robotics
 */


#ifndef CONSOLELOGHANDLER_HPP_
# define CONSOLELOGHANDLER_HPP_

# include <cstdarg>
# include <qi/log.hpp>

namespace qi {
  namespace log {


    /// <summary> Can print colored logs to the console </summary>
    class QI_API ConsoleLogHandler {
    public:
      ConsoleLogHandler();

      void log(const LogLevel    verb,
               const char       *file,
               const char       *fct,
               const char       *category,
               const int         line,
               const char       *msg);

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

    protected:
      bool              _color;

#ifdef _WIN32
      void* _winScreenHandle;
#endif
    };
  }
}


#endif  // !CONSOLELOGHANDLER_HPP_
