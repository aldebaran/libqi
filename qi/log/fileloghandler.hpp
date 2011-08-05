/*
 * fileloghandler.h
 * Author(s):
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 */

#pragma once
#ifndef _LIBQI_QI_LOG_FILELOGHANDLER_HPP_
#define _LIBQI_QI_LOG_FILELOGHANDLER_HPP_

# include <qi/log.hpp>
# include <string>

namespace qi {
  namespace log {
    class PrivateFileLogHandler;

    /** \class qi/log/fileloghandler.hpp
     *  \ingroup qilog
     *  Log to file.
     */
    class QI_API FileLogHandler
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
      QI_DISALLOW_COPY_AND_ASSIGN(FileLogHandler);
      PrivateFileLogHandler* _private;
    }; // !FileLogHandler

  }; // !log
}; // !qi

#endif  // _LIBQI_QI_LOG_FILELOGHANDLER_HPP_
