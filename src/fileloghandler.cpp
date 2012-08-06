/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/log/fileloghandler.hpp>

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

namespace qi {
  namespace log {
    class PrivateFileLogHandler
    {
    public:
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
      catch (const boost::filesystem::filesystem_error &e)
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

    void FileLogHandler::log(const qi::log::LogLevel verb,
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
        std::string logline = qi::detail::logline(date, category, msg, file, fct, line);
        fprintf(_private->_file, "%s %s", logLevelToString(verb), logline.c_str());
        fflush(_private->_file);
      }
    }
  }
}
