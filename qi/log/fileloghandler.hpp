/*
 * fileloghandler.h
 * Author(s):
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 */

#ifndef FILELOGHANDLER_HPP_
# define FILELOGHANDLER_HPP_

# include <qi/log.hpp>
# include <qi/noncopyable.hpp>
# include <string>

namespace qi {
  namespace log {
    class PrivateFileLogHandler;

    class QI_API FileLogHandler : qi::noncopyable
    {
    public:
      explicit FileLogHandler(const std::string& filePath);
      virtual ~FileLogHandler();

      void log(const qi::log::LogLevel verb,
               const qi::os::timeval   date,
               const char              *category,
               const char              *msg,
               const char              *file,
               const char              *fct,
               const int               line);

    private:
      FileLogHandler() {};

      PrivateFileLogHandler* _private;
    }; // !FileLogHandler
  }; // !log
}; // !qi

#endif // !FILELOGHANDLER_HPP_
