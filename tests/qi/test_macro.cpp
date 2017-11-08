/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <thread>
#include <gtest/gtest.h>

#include <boost/noncopyable.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/thread.hpp>
#include <qi/atomic.hpp>
#include <qi/os.hpp>
#include <qi/preproc.hpp>

/* IMPORTANT NOTE:
   The tests related to QI_THREAD_SAFE_NEW macro will leak and cannot
   work in "repeat" test mode because this macro generate static markers
   to know if the 'new' call have already been done since the start
   of the current process.
   We still tried to avoid false-positive leak detection by
   deleting the static objects.
*/

std::atomic<int> counter{0};

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
  static CCount* cc1;
  static void init()
  {
    QI_THREADSAFE_NEW(cc1);
  }

  static void clear()
  {
    delete cc1;
    cc1 = nullptr;
  }
};

template<int i> CCount* StaticCounter1<i>::cc1 = nullptr;

template<int i>
struct staticCounter2
{
  static CCount* cc1;
  static CCount* cc2;

  void operator()()
  {
    QI_THREADSAFE_NEW(cc1, cc2);
  }

  static void clear()
  {
    delete cc1;
    cc1 = nullptr;

    delete cc2;
    cc2 = nullptr;
  }

};

template<int i> CCount* staticCounter2<i>::cc1 = nullptr;
template<int i> CCount* staticCounter2<i>::cc2 = nullptr;

template<int k>
struct staticCounterControlled
{
  static CCount* cc1;

  int operator()(std::atomic<int>& start, int count, std::atomic<int>& delay, int delayDelta)
  {
    int res = k;
    // mark thread started
    ++start;
    // wait for other threads to start
    while (start.load() != count)
      ;
    // delay as asked
    delay += delayDelta;
    const int myDelay = delay;
    for (int i = 0; i < myDelay; ++i)
      res = res + i;
    // GO!
    QI_THREADSAFE_NEW(cc1);
    return res;
  }

  static void clear()
  {
    delete cc1;
    cc1 = nullptr;
  }
};

template<int k> CCount* staticCounterControlled<k>::cc1 = nullptr;

template<class Callable, class... Args>
void runThreads(int threadCount, Callable&& callable, Args&&... args)
{
  std::vector<std::thread> threads;

  for (int i = 0; i < threadCount; ++i)
    threads.emplace_back(std::forward<Callable>(callable), std::forward<Args>(args)...);

  for (auto& thread : threads)
    thread.join();
}

#define EXPECTCOUNTER(v)                                  \
  std::this_thread::sleep_for(std::chrono::milliseconds{1}); \
  for (unsigned t=0; t<200 && counter.load() - prev != v; ++t)  \
    std::this_thread::sleep_for(std::chrono::milliseconds{5}); \
  EXPECT_EQ(v, counter.load() - prev);                    \
  prev = counter.load()


TEST(Macro, ThreadSafeNew)
{
  counter = 0;
  int prev = 0;

  EXPECTCOUNTER(0); // check the macro just in case
  runThreads(10, staticCounter2<0>{});
  EXPECTCOUNTER(2);
  staticCounter2<0>::clear();

  EXPECTCOUNTER(0);
  runThreads(10, &StaticCounter1<1>::init);
  EXPECTCOUNTER(1);
  StaticCounter1<1>::clear();

  runThreads(2, &StaticCounter1<2>::init);
  EXPECTCOUNTER(1);
  StaticCounter1<2>::clear();

  runThreads(3, &StaticCounter1<3>::init);
  EXPECTCOUNTER(1);
  StaticCounter1<3>::clear();

  runThreads(4, &StaticCounter1<4>::init);
  EXPECTCOUNTER(1);
  StaticCounter1<4>::clear();

  runThreads(5, &StaticCounter1<5>::init);
  EXPECTCOUNTER(1);
  StaticCounter1<5>::clear();

  runThreads(6, &StaticCounter1<6>::init);
  EXPECTCOUNTER(1);
  StaticCounter1<6>::clear();

#define DOOM(nthread, delay0, delayDelta)                                           \
  {                                                                                \
    std::atomic<int> a {0};                                                         \
    std::atomic<int> delay{ delay0 };                                               \
    runThreads(nthread, staticCounterControlled<__LINE__>{}, std::ref(a), nthread, std::ref(delay), delayDelta);   \
    EXPECTCOUNTER(1);                                                              \
    staticCounterControlled<__LINE__>::clear();                                     \
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

QI_CREATE_ENUM_WITH_STRING_CONVERSION(toStr, Color, (Blue)(Green)(Red)(Yellow));
TEST(Macro, CreateEnumTostring)
{
  Color color = Red;
  EXPECT_STREQ("Red", toStr(color));
  color = Blue;
  EXPECT_STREQ("Blue", toStr(color));
  color = Green;
  EXPECT_STREQ("Green", toStr(color));
  color = Yellow;
  EXPECT_STREQ("Yellow", toStr(color));
}

