/**
 * headfileloghandler.h
 * Author(s):
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 */

#ifndef HEADFILELOGHANDLER_HPP_
# define HEADFILELOGHANDLER_HPP_

# include <qi/log.hpp>
# include <string>

namespace qi {
  namespace log {
    class QI_API HeadFileLogHandler
    {
    public:
      HeadFileLogHandler(const std::string& filePath);
      virtual ~HeadFileLogHandler();


      HeadFileLogHandler(const HeadFileLogHandler &rhs);
      const HeadFileLogHandler &operator=(const HeadFileLogHandler &rhs);

      void log(const qi::log::LogLevel verb,
               const char              *category,
               const char              *msg,
               const char              *file,
               const char              *fct,
               const int               line);

    private:
      void cutCat(const char* category, char* res);
      HeadFileLogHandler() {};

      FILE* fFile;
      int   nbLog;

    }; // !HeadFileLogHandler
  }; // !log
}; // !qi

#endif // !HEADFILELOGHANDLER_HPP_
