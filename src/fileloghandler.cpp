/*
** fileloghandler.cpp
** Login : <hcuche@hcuche-de>
** Started on  Mon Jun 13 12:48:36 2011 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
*/

#include <qi/log/fileloghandler.hpp>

#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <string>
#include <qi/log.hpp>
#include <qi/os.hpp>
#include <cstdio>

namespace qi {
  namespace log {
    FileLogHandler::FileLogHandler(const std::string& filePath)
    {
      boost::filesystem::path fPath(filePath);
      // Create the directory!
      boost::filesystem::create_directories(fPath.make_preferred().parent_path());
      // Open the file.
      FILE* file = qi::os::fopen(fPath.make_preferred().string().c_str(), "w");

      if(file)
      {
        fFile = file;
        qi::log::addLogHandler(boost::bind(&FileLogHandler::log, this, _1, _2, _3, _4, _5, _6), "fileloghandler");
      }
    }


    FileLogHandler::FileLogHandler(const FileLogHandler &rhs)
      : fFile(new FILE)
    {
      *fFile = *rhs.fFile;
    }

    const FileLogHandler & FileLogHandler::operator=(const FileLogHandler &rhs)
    {
      *fFile = *rhs.fFile;
      return *this;
    }

    FileLogHandler::~FileLogHandler()
    {
      if (fFile != NULL)
        fclose(fFile);

      qi::log::removeLogHandler("fileloghandler");
    }

    void FileLogHandler::log(const qi::log::LogLevel verb,
                             const char              *category,
                             const char              *msg,
                             const char              *file,
                             const char              *fct,
                             const int               line)
    {
      if (verb > qi::log::getVerbosity() || fFile == NULL)
      {
        return;
      }
      else
      {
        const char* head = logLevelToString(verb);
        if (verb == qi::log::debug || qi::log::getContext())
        {
          fprintf(fFile, "%s %s: %s(%d) %s %s", head, category, file, line, fct, msg);
        }
        else
        {
          fprintf(fFile,"%s %s: %s", head, category, msg);
        }
      }
    }
  }
}
