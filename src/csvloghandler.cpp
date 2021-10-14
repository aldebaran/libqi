
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/log/csvloghandler.hpp>

#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>

#include <sstream>
#include <boost/filesystem/fstream.hpp>
#include <iomanip>
#include <string>
#include <qi/log.hpp>
#include "log_p.hpp"
#include <qi/os.hpp>

qiLogCategory("qi.log.Csvloghandler");

namespace qi
{
namespace log
{
  struct PrivateCsvLogHandler
  {
    boost::filesystem::ofstream _file;
  };

  CsvLogHandler::CsvLogHandler(const std::string& filePath)
  {
    boost::filesystem::path fPath(filePath);
    _p.reset(new PrivateCsvLogHandler());

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
    _p->_file.open(fPath.make_preferred(), std::ios_base::app | std::ios_base::out | std::ios_base::binary);

    if (_p->_file.is_open())
      _p->_file << qi::detail::csvheader() << std::flush;
    else
      qiLogWarning() << "Cannot open " << filePath;
  }

  CsvLogHandler::~CsvLogHandler() = default;

  void CsvLogHandler::log(const qi::LogLevel verb,
                          const qi::Clock::time_point date,
                          const qi::SystemClock::time_point systemDate,
                          const char* category,
                          const char* msg,
                          const char* file,
                          const char* fct,
                          const int line)
  {
    if (verb <= qi::log::logLevel() && _p->_file.is_open())
      _p->_file << qi::detail::csvline(date, systemDate, category, msg, file, fct, line, verb) << std::flush;
    else
      return;
  }
}
}
