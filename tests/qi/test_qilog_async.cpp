/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include "test_qilog.hpp"
#include <gtest/gtest.h>
#include <qi/future.hpp>
#include <qi/log.hpp>
#include <atomic>

namespace
{

const int iterations = 1000;
const auto defaultTimeout = qi::Seconds(2);

class BlockyHandler : public LogHandler
{
public:
  std::atomic<int> count{ 0 };
  qi::Promise<void> start;
  qi::Promise<void> finished;

  explicit BlockyHandler(const std::string& name)
    : LogHandler(name, std::ref(*this))
  {
  }

  void operator()(const qi::LogLevel,
                  const qi::Clock::time_point,
                  const qi::SystemClock::time_point,
                  const char*,
                  const char*,
                  const char*,
                  const char*,
                  int)
  {
    start.future().wait();
    if (++count == iterations)
      finished.setValue(0);
  }
};

}

class AsyncLog : public ::testing::Test
{
protected:
  void SetUp() override
  {
    qi::log::setSynchronousLog(false);
  }

  void TearDown() override
  {
    qi::log::flush();
  }
};

TEST_F(AsyncLog, logasync)
{
  qiLogCategory("core.log.test1");

  BlockyHandler bh("BlockyHandler");

  for (int i = 0; i < iterations; i++)
    qiLogFatal() << i;

  bh.start.setValue(0);

  EXPECT_EQ(bh.finished.future().wait(defaultTimeout), qi::FutureState_FinishedWithValue);
}


TEST_F(AsyncLog, cats)
{
  qiLogCategory("pan");
  qiLogWarningF("canard %s", 12);
}
