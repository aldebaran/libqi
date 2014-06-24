/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <gtest/gtest.h>

#include <qi/threadpool.hpp>
#include <qi/atomic.hpp>
#include <qi/os.hpp>

#include <boost/bind.hpp>

void f2(qi::Atomic<unsigned int>& number)
{
  ++number;
}

TEST(QiThreadPool, IntenseThreadCreationAndDestruction)
{
  qi::Atomic<unsigned int> number;
  qi::ThreadPool* pool = new qi::ThreadPool(0, 100, 0, 0);
  boost::function<void()> test = boost::bind(&f2, boost::ref(number));

  for (unsigned int i = 0; i < 10; i++)
  {
    pool->setMaxIdleWorkers(100);
    pool->setMinIdleWorkers(100);
    for (int i = 0; i < 1000; i++)
      pool->schedule(test);
    qi::os::sleep(10);
    pool->setMaxIdleWorkers(0);
    pool->setMinIdleWorkers(0);
    qi::os::sleep(10);
  }

  delete pool;
  EXPECT_EQ(*number, 10000U);
}
