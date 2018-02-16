/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include "test_qilog.hpp"
#include <gtest/gtest.h>
#include <qi/future.hpp>
#include <qi/log.hpp>
#include <qi/testutils/testutils.hpp>
#include <atomic>

namespace
{

static const int iterations = 1000;
static const auto testCategory = "core.log.test1";

class BlockyHandler
{
public:
  std::atomic<int> count{0};
  qi::Promise<void> start;
  qi::Promise<void> finished;
  LogHandler handler;
  const unsigned int& id = handler.id;

  explicit BlockyHandler(const std::string& name)
    : handler(name, std::ref(*this), qi::LogLevel_Verbose)
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
  BlockyHandler bh("BlockyHandler");
  qiLogCategory(testCategory);
  for (int i = 0; i < iterations; i++)
    qiLogVerbose() << "Iteration " << i;

  bh.start.setValue(0);
  ASSERT_TRUE(test::finishesWithValue(bh.finished.future()));
}


TEST_F(AsyncLog, cats)
{
  qiLogCategory("pan");
  qiLogWarningF("canard %s", 12);
}
