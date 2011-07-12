/**
 * fileloghandler.cpp
 * Author(s):
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 */

#include <qi/log/fileloghandler.hpp>

#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <string>
#include <qi/log.hpp>
#include <qi/os.hpp>
#include <cstdio>

#define CATSIZEMAX 16

namespace qi {
  namespace log {
    class PrivateFileLogHandler
    {
    public:
      void cutCat(const char* category, char* res);

      FILE* _file;
    };


    FileLogHandler::FileLogHandler(const std::string& filePath)
      : _private(new PrivateFileLogHandler)
    {
      _private->_file = NULL;
      boost::filesystem::path fPath(filePath);
      // Create the directory!
      try
      {
        if (!boost::filesystem::exists(fPath.make_preferred().parent_path()))
          boost::filesystem::create_directories(fPath.make_preferred().parent_path());
      }
      catch (boost::filesystem::filesystem_error &e)
      {
        qiLogWarning("qi.log.fileloghandler") << e.what() << std::endl;
      }

      // Open the file.
      FILE* file = qi::os::fopen(fPath.make_preferred().string().c_str(), "w+");

      if (file)
        _private->_file = file;
      else
        qiLogWarning("qi.log.fileloghandler") << "Cannot open "
                                              << filePath << std::endl;
    }

    FileLogHandler::~FileLogHandler()
    {
      if (_private->_file != NULL)
        fclose(_private->_file);
    }

    void PrivateFileLogHandler::cutCat(const char* category, char* res)
    {
      int categorySize = strlen(category);
      if (categorySize < CATSIZEMAX)
      {
        memset(res, ' ', CATSIZEMAX);
        memcpy(res, category, strlen(category));
      }
      else
      {
        memset(res, '.', CATSIZEMAX);
        memcpy(res + 3, category + categorySize - CATSIZEMAX + 3, CATSIZEMAX - 3);
      }
      res[CATSIZEMAX] = '\0';
    }


    void FileLogHandler::log(const qi::log::LogLevel verb,
                             const char              *category,
                             const char              *msg,
                             const char              *file,
                             const char              *fct,
                             const int               line)
    {
      if (verb > qi::log::getVerbosity() || _private->_file == NULL)
      {
        return;
      }
      else
      {
        const char* head = logLevelToString(verb);
        char fixedCategory[CATSIZEMAX + 1];
        fixedCategory[CATSIZEMAX] = '\0';
        _private->cutCat(category, fixedCategory);
        if (qi::log::getContext())
        {
          fprintf(_private->_file, "%s %s: %s(%d) %s %s",
                  head, fixedCategory, file, line, fct, msg);
        }
        else
        {
          fprintf(_private->_file,"%s %s: %s", head, fixedCategory, msg);
        }
        fflush(_private->_file);
      }
    }
  }
}
