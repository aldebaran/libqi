/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <gtest/gtest.h>

#include <qi/atomic.hpp>
#include <limits>
#include <qi/types.hpp>

template<template <typename> class A, typename T>
void sub_test_type()
{
  T max = std::numeric_limits<T>::max();
  T min = std::numeric_limits<T>::min();

  A<T> n(max);
  EXPECT_EQ(*n, max);
  ++n;
  EXPECT_EQ(*n, min);

  A<T> m(min);
  EXPECT_EQ(*m, min);
  --m;
  EXPECT_EQ(*m, max);

  m = 42;
  EXPECT_EQ(42, *m);
}

template <typename T>
void test_type()
{
  // new API
  sub_test_type<qi::Atomic, T>();
  // legacy API
  sub_test_type<qi::atomic, T>();
}

TEST(QiAtomic, tas)
{
  long lock = 0;
  EXPECT_EQ(1, qi::testAndSet(&lock));
  EXPECT_EQ(0, qi::testAndSet(&lock));
  lock = 0;
  EXPECT_EQ(1, qi::testAndSet(&lock));
}

#ifndef _MSC_VER
TEST(QiAtomic, qiuint8)
{
  test_type<qi::uint8_t>();
}

TEST(QiAtomic, qiint8)
{
  test_type<qi::int8_t>();
}
#endif

TEST(QiAtomic, qiuint16)
{
  test_type<qi::uint16_t>();
}

TEST(QiAtomic, qiint16)
{
  test_type<qi::int16_t>();
}

TEST(QiAtomic, qiuint32)
{
  test_type<qi::uint32_t>();
}

TEST(QiAtomic, qiint32)
{
  test_type<qi::int32_t>();
}
