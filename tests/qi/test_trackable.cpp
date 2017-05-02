#include <atomic>
#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <qi/future.hpp>
#include <qi/trackable.hpp>
#include "test_future.hpp"

qiLogCategory("test");

void _delayValue(int msDelay, qi::Promise<void> p)
{
  qi::os::msleep(msDelay);
  p.setValue(0);
}

qi::Future<void> delayValue(int msDelay)
{
  qi::Promise<void> p;
  if (msDelay >= 0)
    boost::thread(_delayValue, msDelay, p);
  else
    p.setValue(0);
  return p.future();
}

void set_from_future(std::atomic<int>& tgt, qi::Future<void> f)
{
  tgt = f.isFinished()?2:0;
}

TEST(FutureTrack, WeakPtr)
{
  std::atomic<int> v{0};
  boost::shared_ptr<SetValue> s(new SetValue(v));
  boost::weak_ptr<SetValue> w(s);
  delayValue(-1).connect(&SetValue::exchange, w, 1);
  for (int i=0; i<50&&v!=1; ++i)
    qi::os::msleep(10);
  EXPECT_EQ(1, v);
  v=0;
  // check that _1 works in connect
  delayValue(-1).connect(&set_from_future, std::ref(v), _1);
  for (int i=0; i<50&&v!=2; ++i)
    qi::os::msleep(10);
  EXPECT_EQ(2, v);

  delayValue(100).connect(&SetValue::exchange, w, 3);
  for (int i=0; i<50&&v!=3; ++i)
    qi::os::msleep(10);
  EXPECT_EQ(3, v);

  delayValue(100).connect(&SetValue::exchange, w, 4);
  s.reset(); // reset before future finishes, w invalid, cb not called
  qi::os::msleep(100);
  EXPECT_EQ(3, v);
  s = boost::shared_ptr<SetValue>(new SetValue(v));
  w = boost::weak_ptr<SetValue>(s);

  delayValue(-1).connect(&SetValue::delayExchange, w, 200, 10);
  qi::int64_t time = qi::os::ustime();
  // wait for delayExchange to start
  while (s->state != 1)
    qi::os::msleep(10);
  s.reset();
  // let's block until object is gone
  while (w.lock())
    qi::os::msleep(20);
  time = qi::os::ustime() - time;
  EXPECT_GT(time, 160000);
  qi::os::msleep(20); // poor man's memory barrier, v is written from an other thread
  EXPECT_EQ(10, v);
}

TEST(FutureTrack, Trackable)
{
  // copy-paste of weak-ptr, but use a trackable on stack instead of shared_ptr
  std::atomic<int> v{0};
  SetValue2* w = new SetValue2(v);
  delayValue(-1).connect(&SetValue2::exchange, w, 1);
  for (int i=0; i<50&&v!=1; ++i)
    qi::os::msleep(10);
  EXPECT_EQ(1, v);
  v=0;
  // check that _1 works in connect
  delayValue(-1).connect(&set_from_future, std::ref(v), _1);
  for (int i=0; i<50&&v!=2; ++i)
    qi::os::msleep(10);
  EXPECT_EQ(2, v);

  delayValue(100).connect(&SetValue2::exchange, w, 3);
  for (int i=0; i<50&&v!=3; ++i)
    qi::os::msleep(10);
  EXPECT_EQ(3, v);

  delayValue(100).connect(&SetValue2::exchange, w, 4);
  delete w; // reset before future finishes, w invalid, cb not called
  qi::os::msleep(100);
  EXPECT_EQ(3, v);
  qiLogDebug() << "destruction-lock test";
  w = new SetValue2(v);

  delayValue(0).connect(&SetValue::delayExchange, w, 200, 10);
  qi::int64_t time = qi::os::ustime();
  qi::os::msleep(100); // give time for delayExchange to start executing
  qiLogVerbose() << "deleting w";
  delete w; // blocks until delayExchange finishes
  qiLogVerbose() << "w deleted";
  time = qi::os::ustime() - time;
  EXPECT_GT(time, 160000);
  qi::os::msleep(20); // poor man's memory barrier, v is written from an other thread
  EXPECT_EQ(10, v);
}
