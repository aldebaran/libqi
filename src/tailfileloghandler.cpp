/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/log/tailfileloghandler.hpp>

#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>

#include <iomanip>
#include "log_p.hpp"
#include "qi/macro.hpp"
#include <qi/os.hpp>
#include <cstdio>
#include <boost/thread/mutex.hpp>

#define FILESIZEMAX 1024 * 1024

qiLogCategory("qi.log.tailfileloghandler");

namespace qi
{
namespace log
{
  struct PrivateTailFileLogHandler
  {
    FILE* _file;
    std::string _fileName;
    int _writeSize;
    boost::mutex mutex_;
  };

  TailFileLogHandler::TailFileLogHandler(const std::string& filePath)
    : _p(new PrivateTailFileLogHandler)
  {
    _p->_file = NULL;
    _p->_writeSize = 0;
    _p->_fileName = filePath;

    boost::filesystem::path fPath(_p->_fileName);
    // Create the directory!
    try
    {
      if (!boost::filesystem::exists(fPath.make_preferred().parent_path()))
        boost::filesystem::create_directories(fPath.make_preferred().parent_path());
    }
    catch (const boost::filesystem::filesystem_error& e)
    {
      qiLogWarning() << e.what();
    }

    // Open the file.
    FILE* file = qi::os::fopen(fPath.make_preferred().string().c_str(), "w+");

    if (file)
      _p->_file = file;
    else
      qiLogWarning() << "Cannot open " << filePath;
  }

  TailFileLogHandler::~TailFileLogHandler()
  {
    if (_p->_file != NULL)
      fclose(_p->_file);
    delete _p;
  }
  void TailFileLogHandler::log(const qi::LogLevel verb,
                               const qi::Clock::time_point date,
                               const qi::SystemClock::time_point systemDate,
                               const char* category,
                               const char* msg,
                               const char* file,
                               const char* fct,
                               const int line)

  {
    boost::mutex::scoped_lock scopedLock(_p->mutex_);

    if (verb > qi::log::logLevel() || _p->_file == NULL)
    {
      return;
    }
    else
    {
      fseek(_p->_file, 0, SEEK_END);

      std::string logline =
          qi::detail::logline(qi::log::context(), date, systemDate, category, msg, file, fct, line, verb);
      _p->_writeSize += fprintf(_p->_file, "%s", logline.c_str());
      fflush(_p->_file);
    }

    if (_p->_writeSize > FILESIZEMAX)
    {
      fclose(_p->_file);
      boost::filesystem::path filePath(_p->_fileName);
      boost::filesystem::path oldFilePath(_p->_fileName + ".old");

      QI_ASSERT_TRUE(boost::filesystem::is_regular_file(filePath));
      QI_ASSERT_TRUE(!boost::filesystem::exists(oldFilePath) || boost::filesystem::is_regular_file(oldFilePath));
      const auto copySuccess =
        boost::filesystem::copy_file(filePath, oldFilePath,
                                     boost::filesystem::copy_options::overwrite_existing);
      QI_IGNORE_UNUSED(copySuccess);
      QI_ASSERT_TRUE(copySuccess);

      FILE* pfile = qi::os::fopen(filePath.make_preferred().string().c_str(), "w+");

      _p->_file = pfile;
      _p->_writeSize = 0;
    }
  }
}
}
