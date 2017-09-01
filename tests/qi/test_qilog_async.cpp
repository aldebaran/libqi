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

static const std::string testCategory = "core.log.test1";

class BlockyHandler
{
public:
  std::atomic<int> count{0};
  qi::Promise<void> start;
  qi::Promise<void> finished;
  LogHandler handler;
  const unsigned int& id = handler.id;

  explicit BlockyHandler(const std::string& name)
    : handler(name, std::ref(*this))
  {
  }

  void operator()(const qi::LogLevel,
                  const qi::Clock::time_point,
                  const qi::SystemClock::time_point,
                  const std::string category,
                  const char*,
                  const char*,
                  const char*,
                  int)
  {
    start.future().wait();
    if (category == testCategory && ++count == iterations)
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
  qiLogCategory(testCategory);

  BlockyHandler bh("BlockyHandler");

  for (int i = 0; i < iterations; i++)
    qiLogFatal() << i << std::endl;

  bh.start.setValue(0);

  EXPECT_EQ(qi::FutureState_FinishedWithValue, bh.finished.future().wait(defaultTimeout));
}


TEST_F(AsyncLog, cats)
{
  qiLogCategory("pan");
  qiLogWarningF("canard %s", 12);
}
