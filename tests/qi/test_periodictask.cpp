#include <gtest/gtest.h>
#include <qi/periodictask.hpp>
#include "test_future.hpp"
#include <qi/testutils/testutils.hpp>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>

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
  static const std::chrono::milliseconds sleepDuration { 100 }; // big enough to not block the periodic task too much
  static const qi::MilliSeconds period{ 10 };
  static const int assertCount = 20;
  std::vector<bool> assertions;
  {
    qi::SteadyClockTimePoint last;
    qi::Strand strand;
    qi::PeriodicTask pt;
    pt.setPeriod(period);
    pt.setCallback([&] {
      strand.async([&] {
        const auto now = qi::SteadyClock::now();
        const auto success = now - last >= period;
        assertions.push_back(success);
        last = now;
      }).value();
    });
    pt.start(false); // false = not immediately
    while (strand.async([&]{ return assertions.size(); }).value() < assertCount)
      std::this_thread::sleep_for(sleepDuration);
    pt.stop();
  }
  EXPECT_TRUE(boost::algorithm::all_of(assertions, ka::id_transfo_t{}))
      << boost::join(assertions | boost::adaptors::transformed(
                                   [](bool v) -> std::string { return v ? "true" : "false"; }),
                     ", ");
}

TEST(TestPeriodicTask, Stop)
{
  qi::PeriodicTask pt;
  pt.setCallback([]{ std::this_thread::sleep_for(std::chrono::milliseconds{500}); });
  pt.setUsPeriod(10000000);
  pt.start();
  std::this_thread::sleep_for(std::chrono::milliseconds{100}); // wait for actual start
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
  std::this_thread::sleep_for(std::chrono::milliseconds{100}); // wait for actual start
  qi::int64_t now = qi::os::ustime();
  pt.stop();
  EXPECT_GE(100000, qi::os::ustime() - now);
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
    std::this_thread::sleep_for(std::chrono::milliseconds{i % 20}); // wait for actual start
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
    std::this_thread::sleep_for(std::chrono::milliseconds{1});
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
  qi::Future<void> futures[] {
    qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))),
    qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))),
    qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))),
    qi::getEventLoop()->async(boost::bind(&loopTrigger, std::ref(pt))),
  };
  while(true)
  {
    if (boost::algorithm::all_of(futures, [](qi::Future<void> fut) {
          return fut.wait(0) == qi::FutureState_FinishedWithValue;
        }))
    {
        break;
    }
    pt.start();
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
    pt.stop();
  }
}

TEST(TestPeriodicTask, DoesNotDeadlockWhenStartedWithStrandInsideStrandContext)
{
  qi::Strand strand;
  qi::PeriodicTask pt;
  pt.setStrand(&strand);
  pt.setCallback([]{ /* dummy callback */ });
  pt.setUsPeriod(100000);
  auto futStart = strand.async([&]{ pt.start(); }); // call start from inside the start
  ASSERT_TRUE(test::finishesWithValue(futStart));
  strand.join(); // join it before PeriodicTask is destroyed
}
