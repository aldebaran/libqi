/**
 * headfileloghandler.cpp
 * Author(s):
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 */

#include <qi/log/headfileloghandler.hpp>

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

    class PrivateHeadFileLogHandler
    {
    public:
      void cutCat(const char* category, char* res);

      FILE* _file;
      int   _count;
      int   _max;
    };

    HeadFileLogHandler::HeadFileLogHandler(const std::string& filePath,
                                           int length)
      : _private(new PrivateHeadFileLogHandler)
    {
      _private->_max = length;
      _private->_file = NULL;
      _private->_count = _private->_max + 1;

      boost::filesystem::path fPath(filePath);
      // Create the directory!
      try
      {
        if (!boost::filesystem::exists(fPath.make_preferred().parent_path()))
          boost::filesystem::create_directories(fPath.make_preferred().parent_path());
      }
      catch (boost::filesystem::filesystem_error &e)
      {
        qiLogWarning("qi.log.headfileloghandler") << e.what() << std::endl;
      }
      // Open the file.
      FILE* file = qi::os::fopen(fPath.make_preferred().string().c_str(), "w+");

      if (file)
      {
        _private->_file = file;
        _private->_count = 0;
      }
      else
      {
        qiLogWarning("qi.log.headfileloghandler") << "Cannot open "
                                                  << filePath << std::endl;
      }
    }


    HeadFileLogHandler::~HeadFileLogHandler()
    {
      if (_private->_file != NULL)
        fclose(_private->_file);
    }

    void PrivateHeadFileLogHandler::cutCat(const char* category, char* res)
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


    void HeadFileLogHandler::log(const qi::log::LogLevel verb,
                                 const char              *category,
                                 const char              *msg,
                                 const char              *file,
                                 const char              *fct,
                                 const int               line)
    {
      if (_private->_count < _private->_max)
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
        }
        _private->_count++;
        fflush(_private->_file);
      }
      else if (_private->_file != NULL)
      {
        fclose(_private->_file);
        _private->_file = NULL;
      }
    }
  }
}
