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


void f0(unsigned int& number2)
{
  number2 = 42;
}

void f1(void)
{
  qi::os::sleep(2);
}

void f2(qi::atomic<unsigned int>& number)
{
  ++number;
}

TEST(QiThreadPool, CreateAndDelete)
{
  qi::ThreadPool pool(2, 2);

  qi::os::sleep(1);
  EXPECT_EQ(pool.size(), 2U);
}

TEST(QiThreadPool, OneTask)
{
  qi::ThreadPool* pool = new qi::ThreadPool(2, 2);
  unsigned int number2 = 0;

  qi::os::sleep(1);
  EXPECT_EQ(pool->size(), 2U);
  boost::function<void()> test = boost::bind(&f0, boost::ref(number2));
  pool->schedule(test);
  pool->waitForAll();
  delete pool;
  EXPECT_EQ(number2, 42U);
}

TEST(QiThreadPool, IncreaseAndDecreaseSize)
{
  qi::ThreadPool pool(1, 2, 1, 1);

  qi::os::sleep(1);
  EXPECT_EQ(pool.size(), 1U);
  boost::function<void()> test = boost::bind(&f1);
  pool.schedule(test);
  qi::os::sleep(1);
  pool.schedule(test);
  qi::os::sleep(1);
  EXPECT_EQ(pool.size(), 2U);
  qi::os::sleep(3);
  EXPECT_EQ(pool.size(), 1U);
}

TEST(QiThreadPool, MultipleTasks)
{
  qi::atomic<unsigned int> number;
  qi::ThreadPool* pool = new qi::ThreadPool(4, 8);

  qi::os::sleep(1);
  EXPECT_EQ(pool->size(), 4U);
  boost::function<void()> test = boost::bind(&f2, boost::ref(number));

  for (int i = 0; i < 100; i++)
    pool->schedule(test);

  pool->waitForAll();
  delete pool;
  EXPECT_EQ(*number, 100U);
}

TEST(QiThreadPool, MultipleTasksWithResize)
{
  qi::atomic<unsigned int> number;
  qi::ThreadPool* pool = new qi::ThreadPool(1, 1, 1, 1);

  qi::os::sleep(1);
  EXPECT_EQ(pool->size(), 1U);
  boost::function<void()> test = boost::bind(&f2, boost::ref(number));

  for (int i = 0; i < 500; i++)
    pool->schedule(test);
  pool->setMaxWorkers(4);
  for (int i = 0; i < 500; i++)
    pool->schedule(test);

  pool->waitForAll();
  delete pool;
  EXPECT_EQ(*number, 1000U);
}
