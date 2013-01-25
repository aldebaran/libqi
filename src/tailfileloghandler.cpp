/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/log/tailfileloghandler.hpp>

#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>

#include <sstream>
#include <iomanip>
#include <string>
#include <qi/log.hpp>
#include "log_p.hpp"
#include <qi/os.hpp>
#include <cstdio>
#include <boost/thread/mutex.hpp>

#define FILESIZEMAX 1024 * 1024

namespace qi {
  namespace log {
    class PrivateTailFileLogHandler
    {
    public:
      FILE* _file;
      std::string _fileName;
      int   _writeSize;
      boost::mutex mutex_;
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
      delete _private;
    }

    void TailFileLogHandler::log(const qi::log::LogLevel verb,
                                 const qi::os::timeval   date,
                                 const char              *category,
                                 const char              *msg,
                                 const char              *file,
                                 const char              *fct,
                                 const int               line)
    {
      boost::mutex::scoped_lock scopedLock(_private->mutex_);

      if (verb > qi::log::verbosity() || _private->_file == NULL)
      {
        return;
      }
      else
      {
        fseek(_private->_file, 0, SEEK_END);

        std::string logline = qi::detail::logline(date, category, msg, file, fct, line);
        _private->_writeSize += fprintf(_private->_file, "%s %s", logLevelToString(verb), logline.c_str());
        fflush(_private->_file);
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
