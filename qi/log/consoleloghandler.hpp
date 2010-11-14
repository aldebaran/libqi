/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef    QI_LOG_DEFAULTLOGHANDLER_HPP_
# define   QI_LOG_DEFAULTLOGHANDLER_HPP_

#include <cstdarg>
#include <qi/log/log.hpp>

namespace qi {
  namespace log {
    class ConsoleLogHandler {
      ConsoleLogHandler();

      void log(const char       *file,
               const char       *fct,
               const int         line,
               const char        verb,
               const char       *fmt,
               va_list           vl);


    protected:
      qi::log::LogLevel _verbosity;
    };
  }
}


#endif     /* !DEFAULTLOGHANDLER_PP_ */
