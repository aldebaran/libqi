/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <gtest/gtest.h>

#include <boost/noncopyable.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/thread.hpp>
#include <qi/atomic.hpp>
#include <qi/os.hpp>
#include <qi/preproc.hpp>
#include <qi/traits.hpp>


class CloneOk
{
public:
  int i;
};

class CloneKo : private boost::noncopyable
{
public:
  int i;
};

class CloneKo2: boost::noncopyable
{
  public:
  int i;
};
class CloneKo3: public boost::noncopyable
{
  public:
  int i;
};

TEST(Macro, clonable)
{
  EXPECT_TRUE(qi::isClonable<CloneOk>());
  EXPECT_TRUE(qi::isClonable((CloneOk*)0));
  EXPECT_TRUE(!qi::isClonable<CloneKo>());
  EXPECT_TRUE(!qi::isClonable((CloneKo*)0));
  EXPECT_TRUE(!qi::isClonable<CloneKo2>());
  EXPECT_TRUE(!qi::isClonable((CloneKo2*)0));
  EXPECT_TRUE(!qi::isClonable<CloneKo3>());
  EXPECT_TRUE(!qi::isClonable((CloneKo3*)0));
}

qi::Atomic<int> counter;

class CCount
{
public:
  CCount()  { ++counter;};
};



/* To be able to perform multiple tests of a static init, use template
 * functions. We will pass a different int template value each time.
*/

template<int i> class StaticCounter1
{
public:
  static void init()
  {
    static CCount* cc1 = 0;
    QI_THREADSAFE_NEW(cc1);
  }
};
template<int i> inline
void staticCounter2()
{
  static CCount* cc1=0, *cc2=0;
  QI_THREADSAFE_NEW(cc1, cc2);
}

template<int k> inline
int staticCounterControlled(qi::Atomic<int>* start, int count, int delay)
{
  static CCount* cc1 = 0;
  int res = k;
  // mark thread started
  ++(*start);
  // wait for other threads to start
  while (**start != count)
    ;
  // delay as asked
  for (int i=0; i<delay; ++i)
    res = res + i;
  // GO!
  QI_THREADSAFE_NEW(cc1);
  return res;
}

#define EXPECTCOUNTER(v)                                 \
qi::os::msleep(1);                                        \
  for (unsigned t=0; t<200 && *counter - prev != v; ++t)  \
    qi::os::msleep(5);                                   \
  EXPECT_EQ(v, *counter - prev);                         \
  prev = *counter


TEST(Macro, ThreadSafeNew)
{
  int prev = 0;
  EXPECTCOUNTER(0); // check the macro just in case
  for (unsigned i=0; i<10; ++i)
    boost::thread t(boost::bind(&staticCounter2<0>));
  EXPECTCOUNTER(2);
  EXPECTCOUNTER(0);
  for (unsigned i=0; i<10; ++i)
    boost::thread t(&StaticCounter1<1>::init);
  EXPECTCOUNTER(1);
  for (unsigned i=0; i<2; ++i)
    boost::thread t(&StaticCounter1<2>::init);
  EXPECTCOUNTER(1);
  for (unsigned i=0; i<3; ++i)
    boost::thread t(&StaticCounter1<3>::init);
  EXPECTCOUNTER(1);
  for (unsigned i=0; i<4; ++i)
    boost::thread t(&StaticCounter1<4>::init);
  EXPECTCOUNTER(1);
  for (unsigned i=0; i<5; ++i)
    boost::thread t(&StaticCounter1<5>::init);
  EXPECTCOUNTER(1);
  for (unsigned i=0; i<6; ++i)
    boost::thread t(&StaticCounter1<6>::init);
  EXPECTCOUNTER(1);

#define DOOM(nthread, delay0, delayDelta)                                           \
  {                                                                                           \
    qi::Atomic<int> a;                                                                        \
    for (unsigned i=0; i<nthread; ++i)                                                        \
      boost::thread t(&staticCounterControlled<__LINE__>, &a, nthread, delay0 + i*delayDelta);  \
    EXPECTCOUNTER(1);                                                                         \
  }
  // Do not put multiple dooms on the same line or it will not work.
  DOOM(1, 0, 0);
  DOOM(1, 0, 0);
  DOOM(2, 0, 0);
  DOOM(2, 0, 0);
  DOOM(8, 0, 0);
  DOOM(8, 0, 0);
  DOOM(8, 0, 0);
  DOOM(8, 0, 0);
  DOOM(8, 0, 0);
  DOOM(8, 0, 0);
  DOOM(8, 0, 1);
  DOOM(8, 0, 1);
  DOOM(8, 0, 1);
  DOOM(8, 8, -1);
  DOOM(8, 8, -1);
  DOOM(8, 8, -1);
  DOOM(8, 8, -1);
  DOOM(8, 0, 2);
  DOOM(8, 0, 2);
  DOOM(8, 0, 2);
  DOOM(8, 0, 2);
  DOOM(8, 16, -2);
  DOOM(8, 16, -2);
  DOOM(8, 16, -2);
  DOOM(8, 16, -2);
  DOOM(8, 0, 3);
  DOOM(8, 0, 3);
  DOOM(8, 0, 3);
  DOOM(8, 0, 3);
  DOOM(8, 24, -3);
  DOOM(8, 24, -3);
  DOOM(8, 24, -3);
  DOOM(8, 24, -3);
  DOOM(8, 0, 4);
  DOOM(8, 0, 4);
  DOOM(8, 0, 4);
  DOOM(8, 0, 4);
  DOOM(8, 32, -4);
  DOOM(8, 32, -4);
  DOOM(8, 32, -4);
  DOOM(8, 32, -4);
  DOOM(8, 0,   5);
  DOOM(8, 0,   5);
  DOOM(8, 0,   5);
  DOOM(8, 0,   5);
  DOOM(8, 40, -5);
  DOOM(8, 40, -5);
  DOOM(8, 40, -5);
  DOOM(8, 40, -5);
  DOOM(8, 0,   6);
  DOOM(8, 0,   6);
  DOOM(8, 0,   6);
  DOOM(8, 0,   6);
  DOOM(8, 48, -6);
  DOOM(8, 48, -6);
  DOOM(8, 48, -6);
  DOOM(8, 48, -6);
  DOOM(8, 0,   7);
  DOOM(8, 0,   7);
  DOOM(8, 0,   7);
  DOOM(8, 0,   7);
  DOOM(8, 56, -7);
  DOOM(8, 56, -7);
  DOOM(8, 56, -7);
  DOOM(8, 56, -7);
  DOOM(8, 0,   8);
  DOOM(8, 0,   8);
  DOOM(8, 0,   8);
  DOOM(8, 0,   8);
  DOOM(8, 64, -8);
  DOOM(8, 64, -8);
  DOOM(8, 64, -8);
  DOOM(8, 64, -8);
  DOOM(8, 0,   9);
  DOOM(8, 0,   9);
  DOOM(8, 0,   9);
  DOOM(8, 0,   9);
  DOOM(8, 72, -9);
  DOOM(8, 72, -9);
  DOOM(8, 72, -9);
  DOOM(8, 72, -9);
  DOOM(8, 0,   10);
  DOOM(8, 0,   10);
  DOOM(8, 0,   10);
  DOOM(8, 0,   10);
  DOOM(8, 80, -10);
  DOOM(8, 80, -10);
  DOOM(8, 80, -10);
  DOOM(8, 80, -10);
}

