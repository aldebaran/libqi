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

#define HEAD_SIZE 200
#define CATSIZEMAX 16

namespace qi {
  namespace log {
    HeadFileLogHandler::HeadFileLogHandler(const std::string& filePath)
    {
      fFile = NULL;
      nbLog = HEAD_SIZE + 1;
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
      FILE* file = qi::os::fopen(fPath.make_preferred().string().c_str(), "w");

      if(file)
      {
        fFile = file;
        nbLog = 0;
      }
      else
      {
        qiLogWarning("qi.log.headfileloghandler") << "Cannot open " << filePath << std::endl;
      }
    }


    HeadFileLogHandler::HeadFileLogHandler(const HeadFileLogHandler &rhs)
      : fFile(new FILE)
    {
      *fFile = *rhs.fFile;
    }

    const HeadFileLogHandler& HeadFileLogHandler::operator=(const HeadFileLogHandler &rhs)
    {
      *fFile = *rhs.fFile;
      return *this;
    }

    HeadFileLogHandler::~HeadFileLogHandler()
    {
      if (fFile != NULL)
        fclose(fFile);
    }

    void HeadFileLogHandler::cutCat(const char* category, char* res)
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
      if (nbLog < HEAD_SIZE)
      {
        if (verb > qi::log::getVerbosity() || fFile == NULL)
        {
          return;
        }
        else
        {
          const char* head = logLevelToString(verb);
          char fixedCategory[CATSIZEMAX + 1];
          fixedCategory[CATSIZEMAX] = '\0';
          cutCat(category, fixedCategory);
          if (qi::log::getContext())
          {
            fprintf(fFile, "%s %s: %s(%d) %s %s", head, fixedCategory, file, line, fct, msg);
          }
          else
          {
            fprintf(fFile,"%s %s: %s", head, fixedCategory, msg);
          }
        }
        nbLog++;
        fflush(fFile);
      }
      else if (fFile != NULL)
      {
        fclose(fFile);
        fFile = NULL;
      }
    }
  }
}
