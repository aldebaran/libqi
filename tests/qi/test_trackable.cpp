#include <atomic>
#include <gtest/gtest.h>
#include <boost/thread.hpp>

#include <qi/actor.hpp>
#include <qi/future.hpp>
#include <qi/trackable.hpp>
#include <ka/macro.hpp>
#include "test_future.hpp"

qiLogCategory("test");

void _delayValue(std::chrono::milliseconds delay, qi::Promise<void> p)
{
  std::this_thread::sleep_for(delay);
  p.setValue(nullptr);
}

qi::Future<void> delayValue(std::chrono::milliseconds delay)
{
  qi::Promise<void> p;
  if (delay >= std::chrono::milliseconds::zero())
    boost::thread(_delayValue, delay, p);
  else
    p.setValue(nullptr);
  return p.future();
}

void set_from_future(std::atomic<int>& tgt, qi::Future<void> f)
{
  tgt = f.isFinished()?2:0;
}

/// A wrapper around the deprecated `Future::connect` functions so that we don't have to repeat the
/// warning disabling macro calls everytime it's used.
// TODO: Remove this once the deprecated `Future::connect` functions are removed.
template<typename T, typename... Args>
void deprecatedConnect(qi::Future<T> fut, Args&&... args)
{
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
  fut.connect(ka::fwd<Args>(args)...);
KA_WARNING_POP()
}

TEST(FutureTrack, WeakPtr)
{
  std::atomic<int> v{0};
  boost::shared_ptr<SetValue> s(new SetValue(v));
  boost::weak_ptr<SetValue> w(s);
  deprecatedConnect(delayValue(std::chrono::milliseconds{ -1 }), &SetValue::exchange, w, 1);
  for (int i=0; i<50&&v!=1; ++i)
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  EXPECT_EQ(1, v);
  v=0;
  // check that _1 works in connect
  deprecatedConnect(delayValue(std::chrono::milliseconds{ -1 }), &set_from_future, std::ref(v), _1);
  for (int i=0; i<50&&v!=2; ++i)
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  EXPECT_EQ(2, v);

  deprecatedConnect(delayValue(std::chrono::milliseconds{100}), &SetValue::exchange, w, 3);
  for (int i=0; i<50&&v!=3; ++i)
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  EXPECT_EQ(3, v);

  deprecatedConnect(delayValue(std::chrono::milliseconds{100}), &SetValue::exchange, w, 4);
  s.reset(); // reset before future finishes, w invalid, cb not called
  std::this_thread::sleep_for(std::chrono::milliseconds{100});
  EXPECT_EQ(3, v);
  s = boost::shared_ptr<SetValue>(new SetValue(v));
  w = boost::weak_ptr<SetValue>(s);

  deprecatedConnect(delayValue(std::chrono::milliseconds{ -1 }),
                    &SetValue::delayExchange, w, qi::MilliSeconds{200}, 10);
  qi::int64_t time = qi::os::ustime();
  // wait for delayExchange to start
  while (s->state != 1)
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  s.reset();
  // let's block until object is gone
  while (w.lock())
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
  time = qi::os::ustime() - time;
  EXPECT_GT(time, 160000);
  std::this_thread::sleep_for(std::chrono::milliseconds{20}); // poor man's memory barrier, v is written from an other thread
  EXPECT_EQ(10, v);
}

TEST(FutureTrack, Trackable)
{
  // copy-paste of weak-ptr, but use a trackable on stack instead of shared_ptr
  std::atomic<int> v{0};
  SetValue2* w = new SetValue2(v);
  deprecatedConnect(delayValue(std::chrono::milliseconds{ -1 }), &SetValue2::exchange, w, 1);
  for (int i=0; i<50&&v!=1; ++i)
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  EXPECT_EQ(1, v);
  v=0;
  // check that _1 works in connect
  deprecatedConnect(delayValue(std::chrono::milliseconds{ -1 }), &set_from_future, std::ref(v), _1);
  for (int i=0; i<50&&v!=2; ++i)
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  EXPECT_EQ(2, v);

  deprecatedConnect(delayValue(std::chrono::milliseconds{100}), &SetValue2::exchange, w, 3);
  for (int i=0; i<50&&v!=3; ++i)
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  EXPECT_EQ(3, v);

  deprecatedConnect(delayValue(std::chrono::milliseconds{100}), &SetValue2::exchange, w, 4);
  delete w; // reset before future finishes, w invalid, cb not called
  std::this_thread::sleep_for(std::chrono::milliseconds{100});
  EXPECT_EQ(3, v);
  qiLogDebug() << "destruction-lock test";
  w = new SetValue2(v);

  deprecatedConnect(delayValue(std::chrono::milliseconds{ 0 }),
                    &SetValue::delayExchange, w, qi::MilliSeconds{ 200 }, 10);
  qi::int64_t time = qi::os::ustime();
  std::this_thread::sleep_for(std::chrono::milliseconds{100}); // give time for delayExchange to start executing
  qiLogVerbose() << "deleting w";
  delete w; // blocks until delayExchange finishes
  qiLogVerbose() << "w deleted";
  time = qi::os::ustime() - time;
  EXPECT_GT(time, 160000);
  std::this_thread::sleep_for(std::chrono::milliseconds{20}); // poor man's memory barrier, v is written from an other thread
  EXPECT_EQ(10, v);
}

TEST(FutureTrack, track_actor_build)
{
  qi::Actor stuff;

  // rvalue
  auto f1 = qi::track([](int){}, &stuff);
  (void) f1;

  // lvalue
  auto func = [](int){};
  auto f2 = qi::track(func, &stuff);
  (void) f2;
}

TEST(Trackable, CanTrackFunctionWithExplicitDefaultCtorReturnType)
{
  struct WithExplicitDefaultCtor { explicit WithExplicitDefaultCtor() = default; };
  struct Tracker : qi::Trackable<Tracker> { using Trackable::destroy; };
  Tracker tracker;
  qi::track([]{ return WithExplicitDefaultCtor(); }, &tracker);
  SUCCEED();
}
