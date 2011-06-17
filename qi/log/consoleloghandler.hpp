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

    class PrivateConsoleLogHandler;

    /// <summary> Can print colored logs to the console </summary>
    class QI_API ConsoleLogHandler
    {
    public:
      ConsoleLogHandler();

      ConsoleLogHandler(const ConsoleLogHandler &rhs);
      const ConsoleLogHandler &operator=(const ConsoleLogHandler &rhs);

      void log(const LogLevel    verb,
               const char       *category,
               const char       *msg,
               const char       *file,
               const char       *fct,
               const int         line);


    protected:
      PrivateConsoleLogHandler* _private;
    };
  }
}


#endif  // !CONSOLELOGHANDLER_HPP_
