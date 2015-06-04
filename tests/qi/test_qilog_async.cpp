/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
#include <gtest/gtest.h>
#include <qi/log.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <qi/atomic.hpp>
#include <qi/future.hpp>
#include <cstring>

static const int MAX = 1000;

class BlockyHandler {
public:
  qi::Atomic<int> count;
  qi::Promise<void> start;
  qi::Promise<void> finished;
  void log(const qi::LogLevel,
           const qi::Clock::time_point,
           const qi::SystemClock::time_point,
           const char*,
           const char*,
           const char*,
           const char*,
           int) {
    start.future().wait();
    if (++count == MAX)
      finished.setValue(0);
  }
};

TEST(log, logasync)
{
  qiLogCategory("core.log.test1");

  qi::log::init(qi::LogLevel_Info, 0, false);
  atexit(qi::log::destroy);

  BlockyHandler bh;
  qi::log::addHandler("BlockyHandler",
      boost::bind(&BlockyHandler::log, &bh,
                  _1, _2, _3, _4, _5, _6, _7, _8));

  for (int i = 0; i < MAX; i++)
    qiLogFatal() << i;

  bh.start.setValue(0);

  bh.finished.future().wait();

  qi::log::removeHandler("BlockyHandler");
}


TEST(log, cats)
{
  qiLogCategory("pan");
  qiLogWarningF("canard %s", 12);
  qi::os::msleep(100);
}
