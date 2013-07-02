/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/log/headfileloghandler.hpp>

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

namespace qi {
  namespace log {
    class PrivateHeadFileLogHandler
    {
    public:
      FILE* _file;
      int   _count;
      int   _max;
      boost::mutex _mutex;
    };

    HeadFileLogHandler::HeadFileLogHandler(const std::string& filePath,
                                           int length)
      : _p(new PrivateHeadFileLogHandler)
    {
      _p->_max = length;
      _p->_file = NULL;
      _p->_count = _p->_max + 1;

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
        _p->_file = file;
        _p->_count = 0;
      }
      else
      {
        qiLogWarning("qi.log.headfileloghandler") << "Cannot open "
                                                  << filePath << std::endl;
      }
    }


    HeadFileLogHandler::~HeadFileLogHandler()
    {
      if (_p->_file != NULL)
        fclose(_p->_file);
      delete _p;
    }

    void HeadFileLogHandler::log(const qi::LogLevel verb,
                                 const qi::os::timeval   date,
                                 const char              *category,
                                 const char              *msg,
                                 const char              *file,
                                 const char              *fct,
                                 const int               line)
    {
      boost::mutex::scoped_lock scopedLock(_p->_mutex);

      if (_p->_count < _p->_max)
      {
        if (verb > qi::log::verbosity() || _p->_file == NULL)
        {
          return;
        }
        else
        {
          std::string logline = qi::detail::logline(qi::detail::fileLogContext, date, category, msg, file, fct, line, verb);
          fprintf(_p->_file, "%s", logline.c_str());
          fflush(_p->_file);

          _p->_count++;
        }
      }
      else if (_p->_file != NULL)
      {
        fclose(_p->_file);
        _p->_file = NULL;
      }
    }
  }
}
