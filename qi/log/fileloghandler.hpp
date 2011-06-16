/*
** fileloghandler.h
** Login : <hcuche@hcuche-de>
** Started on  Mon Jun 13 12:48:36 2011 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
*/

#ifndef FILELOGHANDLER_HPP_
# define FILELOGHANDLER_HPP_

# include <qi/log.hpp>
# include <string>

namespace qi {
  namespace log {
    class QI_API FileLogHandler
    {
    public:
      FileLogHandler(const std::string& filePath);
      virtual ~FileLogHandler();

      void log(const qi::log::LogLevel verb,
               const char              *file,
               const char              *fct,
               const char              *category,
               const int                line,
               const char              *msg);

    private:
      FileLogHandler() {};

      FILE* fFile;
    }; // !FileLogHandler
  }; // !log
}; // !qi

#endif // !FILELOGHANDLER_HPP_
