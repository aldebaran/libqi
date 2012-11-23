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
}

template <typename T>
void test_type()
{
  // new API
  sub_test_type<qi::Atomic, T>();
  // legacy API
  sub_test_type<qi::atomic, T>();
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


/*
 * These tests should run since cmpxchg8b instruction is available on x86 CPUs.
 * But it might not work with:
 * - Another CPU
 * - An outdated compiler on a 32 bit OS
 */
#if defined __LP64__ || WINVER > 0x502 // Windows Server 2003
TEST(QiAtomic, qiuint64)
{
  test_type<qi::uint64_t>();
}

TEST(QiAtomic, qiint64)
{
  test_type<qi::int64_t>();
}
#endif
