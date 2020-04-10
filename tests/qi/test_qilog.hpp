/*
**  Copyright (C) 2017 SoftBank Robotics Europe
**  See COPYING for the license
*/

#ifndef QI_TEST_QILOG_HPP
#define QI_TEST_QILOG_HPP

#pragma once

#include <boost/utility/string_ref_fwd.hpp>
#include <gmock/gmock.h>
#include <ka/unit.hpp>
#include <qi/log.hpp>

class LogHandler final
{
public:
  LogHandler(const std::string& name,
             qi::log::Handler handler,
             qi::LogLevel level = qi::LogLevel_Info);

  virtual ~LogHandler();

  std::string name;
  const unsigned int id;
};

class MockLogHandler
{
  LogHandler handler;
public:
  explicit MockLogHandler(const std::string& name);

  // TODO: Replace unit_t by void once we upgrade GoogleMock with the fix on mock methods returning
  // void crashing in optimized compilation on recent compilers.
  // See: https://github.com/google/googletest/issues/705
  MOCK_METHOD3(log, ka::unit_t(qi::LogLevel, const char* /*category*/, const char* /*msg*/));

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

class SyncLog : public ::testing::Test
{
protected:
  void SetUp() override
  {
    qi::log::setSynchronousLog(true);
  }

  void TearDown() override
  {
    qi::log::flush();
  }
};

#endif // QI_TEST_QILOG_HPP
