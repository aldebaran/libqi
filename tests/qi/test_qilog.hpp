/*
**  Copyright (C) 2017 SoftBank Robotics Europe
**  See COPYING for the license
*/

#ifndef QI_TEST_QILOG_HPP
#define QI_TEST_QILOG_HPP

#pragma once

#include <boost/utility/string_ref_fwd.hpp>
#include <gmock/gmock.h>
#include <qi/log.hpp>

class LogHandler final
{
public:
  LogHandler(const std::string& name, qi::log::Handler handler);

  virtual ~LogHandler();

  std::string name;
  const unsigned int id;
};

class MockLogHandler
{
  LogHandler handler;
public:
  explicit MockLogHandler(const std::string& name);

  MOCK_METHOD1(log, void(const char*));

  void operator()(qi::LogLevel,
                  qi::Clock::time_point,
                  qi::SystemClock::time_point,
                  const char* category,
                  const char* message,
                  const char*,
                  const char*,
                  int);

  const unsigned int& id = handler.id;

};

inline void dummyHandler(qi::LogLevel,
                         qi::Clock::time_point,
                         qi::SystemClock::time_point,
                         const char*,
                         const char*,
                         const char*,
                         const char*,
                         int)
{
  // do nothing
}

#endif // QI_TEST_QILOG_HPP
