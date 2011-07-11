/**
 * headfileloghandler.h
 * Author(s):
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 */

#ifndef HEADFILELOGHANDLER_HPP_
# define HEADFILELOGHANDLER_HPP_

# include <qi/log.hpp>
# include <qi/noncopyable.hpp>
# include <string>

namespace qi {
  namespace log {
    class QI_API HeadFileLogHandler : qi::noncopyable
    {
    public:
      HeadFileLogHandler(const std::string& filePath, int length = 2000);
      virtual ~HeadFileLogHandler();

      void log(const qi::log::LogLevel verb,
               const char              *category,
               const char              *msg,
               const char              *file,
               const char              *fct,
               const int               line);
    private:
      void cutCat(const char* category, char* res);
      HeadFileLogHandler() {};

      FILE* _file;
      int   _count;
      int   _max;

    }; // !HeadFileLogHandler
  }; // !log
}; // !qi

#endif // !HEADFILELOGHANDLER_HPP_
