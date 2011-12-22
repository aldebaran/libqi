/*
 * Copyright (c) 2011, Aldebaran Robotics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Aldebaran Robotics nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <qi/log/tailfileloghandler.hpp>

#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>

#include <sstream>
#include <string>
#include <qi/log.hpp>
#include <qi/os.hpp>
#include <cstdio>

#define CATSIZEMAX 16
#define FILESIZEMAX 1024 * 1024

namespace qi {
  namespace log {
    class PrivateTailFileLogHandler
    {
    public:
      void cutCat(const char* category, char* res);

      FILE* _file;
      std::string _fileName;
      int   _writeSize;
    };

    TailFileLogHandler::TailFileLogHandler(const std::string& filePath)
      : _private(new PrivateTailFileLogHandler)
    {
      _private->_file = NULL;
      _private->_writeSize = 0;
      _private->_fileName = filePath;

      boost::filesystem::path fPath(_private->_fileName);
      // Create the directory!
      try
      {
        if (!boost::filesystem::exists(fPath.make_preferred().parent_path()))
          boost::filesystem::create_directories(fPath.make_preferred().parent_path());
      }
      catch (boost::filesystem::filesystem_error &e)
      {
        qiLogWarning("qi.log.tailfileloghandler") << e.what() << std::endl;
      }

      // Open the file.
      FILE* file = qi::os::fopen(fPath.make_preferred().string().c_str(), "w+");

      if (file)
        _private->_file = file;
      else
        qiLogWarning("qi.log.tailfileloghandler") << "Cannot open "
                                                  << filePath << std::endl;
    }


    TailFileLogHandler::~TailFileLogHandler()
    {
      if (_private->_file != NULL)
        fclose(_private->_file);
    }

    void PrivateTailFileLogHandler::cutCat(const char* category, char* res)
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


    void TailFileLogHandler::log(const qi::log::LogLevel verb,
                                 const qi::os::timeval   date,
                                 const char              *category,
                                 const char              *msg,
                                 const char              *file,
                                 const char              *fct,
                                 const int               line)
    {
        if (verb > qi::log::verbosity() || _private->_file == NULL)
        {
          return;
        }
        else
        {
          const char* head = logLevelToString(verb);
          char fixedCategory[CATSIZEMAX + 1];
          fixedCategory[CATSIZEMAX] = '\0';
          _private->cutCat(category, fixedCategory);

          std::stringstream l;
          std::stringstream ss;
          ss << date.tv_sec << "." << date.tv_usec;

          l << head << " ";
          int ctx = qi::log::context();
          switch (ctx)
          {
          case 1:
            l << fixedCategory << ": ";
            break;
          case 2:
            l << ss.str() << " ";
            break;
          case 3:
            l << file << "(" << line << ") ";
            break;
          case 4:
            l << ss.str() << " " << fixedCategory << ": ";
            break;
          case 5:
            l << ss.str() << " " << file << "(" << line << ") ";
            break;
          case 6:
            l << fixedCategory << ": " << file << "(" << line << ") ";
            break;
          case 7:
            l << ss.str() << " " << fixedCategory << ": " << file << "(" << line << ") " << fct;
            break;
          default:
            break;
          }
          l << msg;

          fseek(_private->_file, 0, SEEK_END);
          fprintf(_private->_file, "%s", l.str().c_str());
          fflush(_private->_file);

          _private->_writeSize += l.str().size();
       }

      if (_private->_writeSize > FILESIZEMAX)
      {
        fclose(_private->_file);
        boost::filesystem::path filePath(_private->_fileName);
        boost::filesystem::path oldFilePath(_private->_fileName + ".old");

        boost::filesystem::copy_file(filePath,
                                     oldFilePath,
                                     boost::filesystem::copy_option::overwrite_if_exists);

        FILE* file = qi::os::fopen(filePath.make_preferred().string().c_str(), "w+");

        _private->_file = file;
        _private->_writeSize = 0;
      }
    }
  }
}
