#include <gtest/gtest.h>
#include <qi/periodictask.hpp>
#include "test_future.hpp"

void empty() {}

TEST(TestPeriodicTask, Exception)
{
  {
    qi::PeriodicTask pt;
    EXPECT_ANY_THROW(pt.start());
    EXPECT_ANY_THROW(pt.setUsPeriod(-123));
    pt.setUsPeriod(1000);
    EXPECT_ANY_THROW(pt.start());
  }
  {
    qi::PeriodicTask pt;
    pt.setCallback(&empty);
    EXPECT_ANY_THROW(pt.start()); // interval not set
  }
}

void inc(qi::Atomic<int>& tgt)
{
  ++tgt;
}

TEST(TestPeriodicTask, FutureFuck)
{
  for (unsigned i=0; i<500; ++i)
  {
    qi::Future<void> f;
    qi::Promise<void> p;
    f = p.future();
    p.setValue(0);
    f.wait();
  }
}

TEST(TestPeriodicTask, Basic)
{
  qi::Atomic<int> a;
  qi::PeriodicTask pt;
  pt.setCallback(&inc, std::ref(a));
  pt.setUsPeriod(100000);
  pt.start();
  qi::os::msleep(450);
  EXPECT_GE(2, std::abs(*a - 5)); // be leniant for our overloaded buildslaves
  pt.stop();
  int cur = *a;
  qi::os::msleep(60);
  EXPECT_EQ(cur, *a); // stop means stop
  pt.start();
  qi::os::msleep(150);
  EXPECT_GE(1, std::abs(*a - cur - 2));
}

TEST(TestPeriodicTask, Stop)
{
  qi::PeriodicTask pt;
  pt.setCallback(boost::bind(&qi::os::msleep, 500));
  pt.setUsPeriod(10000000);
  pt.start();
  qi::os::msleep(100); // wait for actual start
  qi::int64_t now = qi::os::ustime();
  pt.stop();
  EXPECT_LE(300000, qi::os::ustime() - now);
}


TEST(TestPeriodicTask, StopFromTask)
{
  qi::PeriodicTask pt;
  pt.setCallback(boost::bind(&qi::PeriodicTask::stop, boost::ref(pt)));
  pt.setUsPeriod(10000000);
  pt.start();
  qi::os::msleep(100); // wait for actual start
  qi::int64_t now = qi::os::ustime();
  pt.stop();
  EXPECT_GE(100000, qi::os::ustime() - now);
}

static void pdelete(qi::PeriodicTask* p)
{
  delete p;
}

TEST(TestPeriodicTask, DeleteFromTask)
{
  qi::PeriodicTask* pt = new qi::PeriodicTask();
  pt->setCallback(boost::bind(pdelete, pt));
  pt->setUsPeriod(10000000);
  pt->start();
  qi::os::msleep(200); // wait for actual start
}

TEST(TestPeriodicTask, DeadLock)
{
  qi::Atomic<int> a;
  for (unsigned i=0; i<500; ++i)
  {
    qi::PeriodicTask pt;
    pt.setCallback(&inc, std::ref(a));
    pt.setUsPeriod(0);
    pt.start();
    qi::os::msleep(i%20);
    pt.stop();
  }
}

TEST(TestPeriodicTask, StartIsNoop)
{
  qi::Promise<void> prom;
  qi::PeriodicTask pt;
  pt.setCallback(&block, 99, prom.future());
  pt.setUsPeriod(100);
  std::vector<qi::Future<void> > futs;
  // multiple start is no-op, this should not deadlock
  for (unsigned i=0; i<20; ++i)
    futs.push_back(qi::async(boost::function<void()>(boost::bind(&qi::PeriodicTask::start, &pt, true))));
  for (unsigned i=0; i<20; ++i)
    futs[i].wait();

  prom.setValue(0);
  pt.stop();
}

static void loopTrigger(qi::PeriodicTask& pt)
{
  for (int i = 0; i < 1000; ++i)
  {
    pt.trigger();
    qi::os::msleep(1);
  }
}

TEST(TestPeriodicTask, Trigger)
{
  // just test that there is no segfault or deadlock
  qi::PeriodicTask pt;
  qi::Atomic<int> a;
  pt.setCallback(&inc, std::ref(a));
  pt.setUsPeriod(1000);
  pt.start();
  std::vector<qi::Future<void> > futures;
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))));
  for (unsigned int i = 0; i < futures.size(); ++i)
    futures[i].wait();
  pt.stop();
}

TEST(TestPeriodicTask, TriggerStartStop)
{
  // just test that there is no segfault or deadlock
  qi::PeriodicTask pt;
  qi::Atomic<int> a;
  pt.setCallback(&inc, std::ref(a));
  pt.setUsPeriod(1000);
  std::vector<qi::Future<void> > futures;
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))));
  while (true)
  {
    bool stop = true;
    for (unsigned int i = 0; i < futures.size(); ++i)
      stop = stop && futures[i].wait(0) == qi::FutureState_FinishedWithValue;
    if (stop)
      break;
    pt.start();
    qi::os::msleep(10);
    pt.stop();
  }
}
