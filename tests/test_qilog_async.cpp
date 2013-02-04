/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
#include <gtest/gtest.h>
#include <qi/log.hpp>
#include <cstring>

TEST(log, logasync)
{
  qi::log::init(qi::log::info, 0, false);
  atexit(qi::log::destroy);

   for (int i = 0; i < 1000; i++)
     qiLogFatal("core.log.test1", "%d\n", i);
}


TEST(log, cats)
{
  qiLogCategory("pan");
  qiLogWarningF("canard %s", 12);
  qi::os::msleep(100);
}
