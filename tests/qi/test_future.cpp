/*
**
** Copyright (C) 2010, 2012, 2013, Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <future>
#include <list>
#include <string>
#include <thread>
#include <chrono>
#include <functional>
#include <boost/thread.hpp>
#include <qi/application.hpp>
#include <qi/future.hpp>
#include <qi/log.hpp>
#include <qi/tag.hpp>
#include <qi/testutils/testutils.hpp>
#include <ka/conceptpredicate.hpp>
#include <ka/src.hpp>
#include <ka/macro.hpp>
#include "test_future.hpp"

qiLogCategory("test");

static const qi::MilliSeconds defaultWaitTimeout{500};

SetValue::SetValue(std::atomic<int>& tgt)
  : target(tgt)
  , state(0)
{
}

int SetValue::exchange(int v)
{
  int old = target;
  target = v;
  return old;
}

int SetValue::delayExchange(qi::MilliSeconds delay, int value)
{
  qiLogDebug("test") << "delayexchange enter";
  state = 1;
  boost::this_thread::sleep_for(delay);
  state = 2;
  qiLogDebug("test") << "delayexchange leave";
  return exchange(value);
}

SetValue2::SetValue2(std::atomic<int>& target)
:SetValue(target)
{}

SetValue2::~SetValue2()
{
  qiLogVerbose("test") << "entering dtor";
  state = 1;
  destroy();
  state = 2;
  qiLogVerbose("test") << "finishing dtor";
}

void SetValue2::delayExchangeP(qi::MilliSeconds delay, int value, qi::Promise<int> result)
{
  result.setValue(delayExchange(delay, value));
}

int block(int /*i*/, qi::Future<void> f)
{
  f.wait();
  return 99;
}

int get42() { return 42; }


class FutureFixture : public ::testing::Test
{
public:
  static int         gGlobalI;
  static std::string gGlobalS;
  static std::string gGlobalE;
  static qi::Atomic<int>   gSuccess;

protected:
  static void SetUpTestCase()
  {
    gGlobalI = 0;
    gGlobalS = "";
    gGlobalE = "";
    gSuccess = 0;
  }
};

int         FutureFixture::gGlobalI;
std::string FutureFixture::gGlobalS;
std::string FutureFixture::gGlobalE;
qi::Atomic<int> FutureFixture::gSuccess;

class FutureFixtureI {
public:
  FutureFixtureI(int &gGlobalI, std::string &gGlobalE)
    : gGlobalI(gGlobalI),
      gGlobalE(gGlobalE)
  {}

  void onFutureFinished(qi::Future<int> future) {
    if (future.hasError())
      gGlobalE = future.error();
    else
      gGlobalI = future.value();
  }

  int         &gGlobalI;
  std::string &gGlobalE;
};

class FutureFixtureS {
public:
  FutureFixtureS(std::string &gGlobalS, std::string &gGlobalE)
    : gGlobalS(gGlobalS),
      gGlobalE(gGlobalE)
  {}

  void onFutureFinished(qi::Future<std::string> future) {
    if (future.hasError())
      gGlobalE = future.error();
    else
      gGlobalS = future.value();
  }

  std::string &gGlobalS;
  std::string &gGlobalE;
};

TEST_F(FutureFixture, Validity) {
  qi::Future<void> a;
  EXPECT_FALSE(a.isValid());

  qi::Promise<void> p;
  EXPECT_TRUE(p.future().isValid());

  qi::Future<void> b = p.future();
  EXPECT_TRUE(b.isValid());

  a = b;
  EXPECT_TRUE(a.isValid());

  a = qi::Future<void>();
  EXPECT_FALSE(a.isValid());
}

static void donothingcb()
{}

TEST_F(FutureFixture, Invalid) {
  qi::Future<void> a;
  EXPECT_EQ(a.wait(), qi::FutureState_None);
  EXPECT_THROW(a.connect(boost::bind(donothingcb)), qi::FutureException);
  EXPECT_THROW(a.value(), qi::FutureException);
  EXPECT_THROW(a.error(), qi::FutureException);
  EXPECT_FALSE(a.isValid());
  EXPECT_FALSE(a.hasValue());
  EXPECT_FALSE(a.hasError());
  EXPECT_FALSE(a.isCanceled());
  EXPECT_FALSE(a.isFinished());
  EXPECT_FALSE(a.isRunning());
}

TEST_F(FutureFixture, SimpleType) {
  FutureFixtureI tf(gGlobalI, gGlobalE);

  qi::Promise<int> pro(qi::FutureCallbackType_Sync);

  qi::Future<int>  fut = pro.future();

  fut.connect(boost::bind(&FutureFixtureI::onFutureFinished, tf, _1));

  EXPECT_EQ(0, gGlobalI);
  EXPECT_FALSE(fut.isFinished());
  EXPECT_TRUE(fut.isRunning());
  EXPECT_FALSE(fut.isCanceled());
  pro.setValue(42);
  fut.wait(1000);
  EXPECT_TRUE(fut.isFinished());
  EXPECT_EQ(42, fut.value());
  EXPECT_EQ(42, gGlobalI);
}

TEST_F(FutureFixture, ComplexType) {
  FutureFixtureS tf(gGlobalS, gGlobalE);

  qi::Promise<std::string> pro(qi::FutureCallbackType_Sync);

  qi::Future<std::string>  fut = pro.future();

  fut.connect(boost::bind(&FutureFixtureS::onFutureFinished, tf, _1));

  EXPECT_STREQ("", gGlobalS.c_str());
  EXPECT_FALSE(fut.isFinished());
  pro.setValue("42");
  EXPECT_TRUE(fut.isFinished());
  EXPECT_STREQ("42", fut.value().c_str());
  EXPECT_STREQ("42", gGlobalS.c_str());
}

TEST_F(FutureFixture, PromiseSetWhileWaitingOnFuture)
{
  qi::Promise<void> p;
  auto f = p.future();
  auto setValue = std::async(std::launch::async, [&]{ p.setValue(nullptr); });
  ASSERT_EQ(qi::FutureState_FinishedWithValue, f.waitFor(qi::MilliSeconds{300}));
  setValue.get();
}

void producer(qi::Promise<int> pro) {
  std::this_thread::sleep_for(std::chrono::milliseconds{100});
  pro.setValue(42);
}

void consumer(qi::Atomic<int> &gSuccess, qi::Future<int> fut) {
  //wont block thread on error
  ASSERT_TRUE(fut.wait(1000) == qi::FutureState_FinishedWithValue);
  EXPECT_EQ(42, fut.value());
  ++gSuccess;
}

TEST_F(FutureFixture, Threaded) {
  qi::Promise<int> pro;
  EXPECT_EQ(0, gSuccess.load());
  boost::thread_group tg;

  tg.create_thread(boost::bind(&consumer, std::ref(gSuccess), pro.future()));
  tg.create_thread(boost::bind(&consumer, std::ref(gSuccess), pro.future()));
  tg.create_thread(boost::bind(&consumer, std::ref(gSuccess), pro.future()));
  tg.create_thread(boost::bind(&producer, pro));
  tg.join_all();
  EXPECT_EQ(3, gSuccess.load());
}

TEST_F(FutureFixture, TestTimeout) {
  qi::Promise<int> pro;
  qi::Future<int>  fut = pro.future();

  qi::SteadyClock::time_point start = qi::SteadyClock::now();
  EXPECT_EQ(qi::FutureState_Running, fut.wait(100));
  EXPECT_FALSE(fut.isFinished());
  EXPECT_GT(qi::SteadyClock::now(), start + qi::MilliSeconds(100));

  start = qi::SteadyClock::now();
  EXPECT_EQ(qi::FutureState_Running, fut.wait(qi::MicroSeconds(50000)));
  EXPECT_FALSE(fut.isFinished());
  EXPECT_GT(qi::SteadyClock::now(), start + qi::MilliSeconds(50));

  start = qi::SteadyClock::now();
  EXPECT_EQ(qi::FutureState_Running, fut.wait(qi::SteadyClock::now() + qi::MilliSeconds(10)));
  EXPECT_FALSE(fut.isFinished());
  EXPECT_GT(qi::SteadyClock::now(), start + qi::MilliSeconds(10));

  start = qi::SteadyClock::now();
  qi::asyncDelay(boost::function<void()>(boost::bind(&qi::Promise<int>::setValue, pro, 42)), qi::MicroSeconds(20000));
  EXPECT_EQ(qi::FutureState_FinishedWithValue, fut.wait(qi::SteadyClock::now() + qi::MilliSeconds(40)));
  auto end = qi::SteadyClock::now();
  EXPECT_TRUE(fut.isFinished());
  EXPECT_GT(end, start + qi::MilliSeconds(20));
  EXPECT_LT(end, start + qi::MilliSeconds(50))
      << "Timeout took " << boost::chrono::duration_cast<qi::MilliSeconds>(end - start).count() << "ms, which is too much!";
}

TEST_F(FutureFixture, TestError) {
  FutureFixtureI tf(gGlobalI, gGlobalE);

  qi::Promise<int> pro(qi::FutureCallbackType_Sync);

  qi::Future<int>  fut = pro.future();
  fut.connect(boost::bind(&FutureFixtureI::onFutureFinished, tf, _1));

  EXPECT_STREQ("", gGlobalE.c_str());
  EXPECT_FALSE(fut.isFinished());
  pro.setError("chiche");
  fut.wait();
  EXPECT_STREQ("chiche", gGlobalE.c_str());
  EXPECT_TRUE(fut.isFinished());
  EXPECT_TRUE(fut.hasError());
  EXPECT_THROW(fut.value(), qi::FutureUserException);
  std::string err;
  try {
    fut.value();
  } catch (qi::FutureUserException &fue) {
    err = fue.what();
  }
  EXPECT_EQ(err, fut.error());
}

TEST_F(FutureFixture, TestStateNone)
{
  qi::Future<int> f;
  ASSERT_FALSE(f.isRunning());
  ASSERT_FALSE(f.isCanceled());
  ASSERT_FALSE(f.isFinished());
  ASSERT_TRUE(f.wait(qi::FutureTimeout_None) == qi::FutureState_None);

  qi::Promise<int> p;
  f = p.future();
  ASSERT_TRUE(f.isRunning());
  ASSERT_FALSE(f.isCanceled());
  ASSERT_FALSE(f.isFinished());
  ASSERT_TRUE(f.wait(qi::FutureTimeout_None) == qi::FutureState_Running);
}

TEST(Future, TestBroken)
{
  qi::Future<int> f;
  {
    qi::Promise<int> p;
    f = p.future();
  }
  EXPECT_TRUE(f.isFinished());
  EXPECT_TRUE(f.hasError());
  EXPECT_FALSE(f.isCanceled());
  EXPECT_THROW(f.value(), qi::FutureException);
}

struct DelCheck {
  void foo() {}
};

TEST_F(FutureFixture, TestPromiseAdapter)
{
  boost::weak_ptr<DelCheck> wdc;
  {
    qi::Promise<int> p;
    qi::Promise<int> p2;

    // Prepare for test for leakage
    {
      boost::shared_ptr<DelCheck> dc = boost::make_shared<DelCheck>();
      wdc = dc;
      p.future().connect(std::bind(&DelCheck::foo, dc));
      p2.future().connect(std::bind(&DelCheck::foo, dc));
    }
    EXPECT_FALSE(wdc.expired());

    qi::adaptFuture(p.future(), p2);
    p.setValue(42);
    // wait for the result to be forwarded
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_EQ(42, p2.value());
  }
  // Test for leakage
  EXPECT_TRUE(wdc.expired());
}

void justThrow()
{
  throw std::runtime_error("Expected error.");
}

TEST(Future, AsyncErrorOnTaskThrow)
{
  qi::Future<void> f = qi::async(&justThrow);
  EXPECT_TRUE(f.hasError());
}

void unlock(qi::Promise<int> prom, std::atomic<bool>& tag)
{
  tag = true;
  prom.setValue(1);
}

TEST(Future, SyncBasic)
{
  qi::EventLoop* eventLoop = qi::getEventLoop();
  ASSERT_FALSE(eventLoop->isInThisContext());

  {
    qi::FutureSync<int> fs;
    ASSERT_TRUE(!fs.isFinished());
  } // unbound futuresync should not block

  std::atomic<bool> tag;
  tag = false;
  {
    qi::FutureSync<int> fs;
    qi::Promise<int> p;
    fs = p.future();
    eventLoop->asyncDelay(boost::bind(unlock, p, std::ref(tag)), qi::MicroSeconds(50000));
  }
  ASSERT_TRUE(tag); // fs should block at end of scope, so we reach here after unlock

  tag = false;
  { // This scope is a synchro point to avoid tests running in //
    qi::Promise<int> p;
    qi::FutureSync<int> syncPoint = p.future();
    {
      qi::FutureSync<int> fs;
      fs = p.future();
      fs.async();
      eventLoop->asyncDelay(boost::bind(unlock, p, std::ref(tag)), qi::MicroSeconds(50000));
    }
    ASSERT_FALSE(tag); // fs is async: we exit immediately
  }
  ASSERT_TRUE(tag); // validate our synchro point worked

  tag = false;
  { // This scope is a synchro point to avoid tests running in //
    qi::Promise<int> p;
    qi::FutureSync<int> syncPoint = p.future();
    {
      qi::FutureSync<int> fs;
      fs = p.future();
      qi::Future<int> fa = fs;
      eventLoop->asyncDelay(boost::bind(unlock, p, std::ref(tag)), qi::MicroSeconds(50000));
    }
    ASSERT_FALSE(tag); // fs was copied: blocking disabled
  }
  ASSERT_TRUE(tag); // validate our synchro point worked

  ASSERT_TRUE(true);
}

qi::FutureSync<int> getSync(std::atomic<bool>& tag)
{
  qi::EventLoop* el = qi::getEventLoop();
  qi::Promise<int> promise;
  el->asyncDelay(boost::bind(unlock, promise, std::ref(tag)), qi::MicroSeconds(50000));
  return promise.future();
}

qi::FutureSync<int> getSync2(std::atomic<bool>& tag)
{
  qi::EventLoop* el = qi::getEventLoop();
  qi::Promise<int> promise;
  el->asyncDelay(boost::bind(unlock, promise, std::ref(tag)), qi::MicroSeconds(50000));
  return promise.future().sync();
}

qi::FutureSync<int> getGetSync(std::atomic<bool>& tag)
{
  return getSync(tag);
}

qi::FutureSync<int> getGetSync2(std::atomic<bool>& tag)
{
  return getSync2(tag);
}

TEST(FutureTestSync, InSitu)
{
  /* Check that whatever we do, a function returning a FutureSync is not
  * stuck if we take the sync, and blocks if we ignore it
  */
  std::atomic<bool> tag;
  tag = false;
  {
    qi::FutureSync<int> fs = getSync(std::ref(tag));
    ASSERT_FALSE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
  {
    qi::FutureSync<int> fs = getSync2(std::ref(tag));
    ASSERT_FALSE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
  {
    qi::FutureSync<int> fs = getGetSync(std::ref(tag));
    ASSERT_FALSE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
  {
    qi::FutureSync<int> fs = getGetSync2(std::ref(tag));
    ASSERT_FALSE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
  {
    getSync(std::ref(tag));
    ASSERT_TRUE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
  {
    getSync2(std::ref(tag));
    ASSERT_TRUE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
}

TEST(FutureTestSync, ThrowOnDestroy) {
  qi::Promise<int> prom;

  prom.setError("touctouc");
  EXPECT_THROW(qi::FutureSync<int>(prom.future()), std::runtime_error);
  EXPECT_THROW(qi::FutureSync<int>(prom.future()), std::runtime_error); //multiple futuresync on the same future should fail.
  EXPECT_THROW(qi::FutureSync<int>(prom.future()).value(), std::runtime_error); //value should fail.
}

TEST(FutureTestSync, NoThrow) {
  qi::Promise<int> prom;

  prom.setError("touctouc");
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future()).wait(qi::FutureTimeout_None));
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future()).hasError());
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future()).hasValue());
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future()).isRunning());
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future()).isCanceled());
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future()).isFinished());
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future()).error());
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future()).cancel());
}

void do_nothing(FutureFixtureI*) {}

TEST(FutureTestError, MultipleSetValue)
{
  qi::Promise<int> p;
  qi::Future<int> f = p.future();
  p.setValue(0);
  p = qi::Promise<int>();
  p.setError("");
  p = qi::Promise<int>();
  p.setValue(1);
  EXPECT_ANY_THROW({ p.setValue(0);});
}

TEST(FutureTestError, ValueOnError)
{
  qi::Promise<int> p;
  qi::Future<int> f = p.future();
  p.setError("foo");
  EXPECT_ANY_THROW({ f.value();});
}


TEST(FutureTestCancel, AsyncCallCanceleable)
{
  static const auto EXECUTION_DELAY = qi::MilliSeconds{ 100 };

  bool b = false;
  qi::Future<void> f = qi::getEventLoop()->asyncDelay(
    [&] { b = true; }, EXECUTION_DELAY);
  f.cancel();
  // f is going to cancel asynchronously, so it can already be canceled, or not
  f.wait();
  ASSERT_TRUE(!b);
  ASSERT_TRUE(f.isFinished());
  ASSERT_FALSE(f.hasError());
  ASSERT_TRUE(f.isCanceled());
}

static void doCancel(qi::Promise<int> promise)  { promise.setCanceled(); }
static void doError(qi::Promise<int> promise)   { promise.setError("paf");}
static void doValue(qi::Promise<int> promise)   { promise.setValue(42); }
static void doNothing(qi::Promise<int> /*promise*/) { ; }

TEST(FutureTestCancel, CancelRequest)
{
  qi::Promise<int> promise;

  ASSERT_FALSE(promise.isCancelRequested());

  qi::Future<int> future = promise.future();

  ASSERT_TRUE(future.isRunning());

  ASSERT_NO_THROW(future.cancel());

  ASSERT_TRUE(promise.isCancelRequested());

  promise.setCanceled();

  ASSERT_TRUE(future.isCanceled());
}

TEST(FutureTestCancel, Canceleable)
{
  qi::Promise<int> p(&doCancel);
  qi::Future<int> f = p.future();

  ASSERT_FALSE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  f.cancel();
  ASSERT_TRUE(f.isFinished());
  ASSERT_TRUE(f.isCanceled());
  ASSERT_FALSE(f.hasError(qi::FutureTimeout_None));
  ASSERT_FALSE(f.hasValue(qi::FutureTimeout_None));


  p = qi::Promise<int>(&doError);
  f = p.future();

  ASSERT_FALSE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  f.cancel();
  ASSERT_TRUE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  ASSERT_TRUE(f.hasError(qi::FutureTimeout_None));
  ASSERT_FALSE(f.hasValue(qi::FutureTimeout_None));

  p = qi::Promise<int>(&doValue);
  f = p.future();

  ASSERT_FALSE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  f.cancel();
  ASSERT_TRUE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  ASSERT_FALSE(f.hasError(qi::FutureTimeout_None));
  ASSERT_TRUE(f.hasValue(qi::FutureTimeout_None));

  p = qi::Promise<int>(&doNothing);
  f = p.future();

  ASSERT_FALSE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  f.cancel();
  ASSERT_FALSE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  EXPECT_THROW(f.hasError(qi::FutureTimeout_None), qi::FutureException);
  EXPECT_THROW(f.hasValue(qi::FutureTimeout_None), qi::FutureException);
}

TEST(FutureTestCancel, CancelCannotThrow) // If this test fails, it can result in aborting the whole program.
{
  qi::Promise<void> promise([](qi::Promise<void>&) { throw 42; });
  auto future = promise.future();
  EXPECT_NO_THROW(future.cancel());
}

TEST(TestFutureCancel, CanceledDelayed)
{
  // We can't guarantee when it will be cancelled but we can guarantee it will be
  // cancelled if cancel is called before delayed execution.
  bool b = false;
  qi::Future<void> f = qi::getEventLoop()->asyncDelay(
    [&]{ b = true; }, qi::Hours(999999));
  f.cancel();
  f.wait();
  ASSERT_TRUE(!b);
  ASSERT_TRUE(f.isFinished());
  ASSERT_FALSE(f.hasError());
  ASSERT_TRUE(f.isCanceled());
}

int assinc(const qi::Future<int>& f, int exp)
{
  int val = 0;
  EXPECT_NO_THROW(val = f.value());
  EXPECT_EQ(exp, val);
  return val+1;
}

TEST(FutureTestThen, ThenR)
{
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
  qi::Future<int> f = qi::async(&get42);
  qi::Future<int> ff = f.thenR<int>(&assinc, _1, 42);
  qi::Future<int> fff = ff.thenR<int>(&assinc, _1, 43);
KA_WARNING_POP()

  ASSERT_EQ(44, fff.value());
}

TEST(FutureTestThen, Then)
{
  qi::Future<int> f = qi::async(&get42);
  qi::Future<int> ff = f.then(qi::bind(&assinc, _1, 42));
  qi::Future<int> fff = ff.then(qi::bind(&assinc, _1, 43));

  ASSERT_EQ(44, fff.value());
}

TEST(FutureTestThen, ThenCancel)
{
  qi::Promise<int> p;
  qi::Future<void> f = p.future().then(qi::FutureCallbackType_Sync, [](qi::Future<int> f) {
      ASSERT_TRUE(f.isCanceled());
      });

  f.cancel();
  ASSERT_TRUE(p.isCancelRequested());
  p.setCanceled();
  ASSERT_TRUE(f.hasValue());
}

int fail(int)
{
  throw std::runtime_error("fail");
}

int call(bool& b)
{
  b = true;
  return 42;
}

TEST(FutureTestThen, AndThenR)
{
  bool called = false;
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
  qi::Future<int> f = qi::async(&get42);
  qi::Future<int> ff = f.andThenR<int>(boost::bind(&fail, _1));
  qi::Future<int> fff = ff.andThenR<int>(boost::bind(&call, std::ref(called)));
KA_WARNING_POP()

  fff.wait();

  EXPECT_FALSE(called);
  ASSERT_TRUE(fff.hasError());
  EXPECT_EQ(fff.error(), "fail");
}

TEST(FutureTestThen, AndThen)
{
  bool called = false;
  qi::Future<int> f = qi::async(&get42);
  qi::Future<int> ff = f.andThen(boost::bind(&fail, _1));
  qi::Future<int> fff = ff.andThen(boost::bind(&call, std::ref(called)));

  fff.wait();

  EXPECT_FALSE(called);
  ASSERT_TRUE(fff.hasError());
  EXPECT_EQ(fff.error(), "fail");
}

TEST(FutureTestThen, AndThenRVoid)
{
  bool called = false;
  qi::Promise<void> p;
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
  qi::Future<void> ff = p.future().andThenR<void>(boost::bind(&call, std::ref(called)));
KA_WARNING_POP()
  p.setValue(0);

  ff.wait();

  EXPECT_TRUE(called);
  ASSERT_TRUE(ff.hasValue());
}

TEST(FutureTestThen, AndThenRCancel)
{
  qi::Promise<void> blockProm;

  bool called = false;
  qi::Future<int> f = qi::Future<int>(42);
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
  qi::Future<int> ff = f.andThenR<int>(boost::bind(&block, _1, blockProm.future()));
  qi::Future<int> fff = ff.andThenR<int>(boost::bind(&call, std::ref(called)));
KA_WARNING_POP()

  fff.cancel();
  blockProm.setValue(0);
  fff.wait();

  EXPECT_FALSE(called);
  EXPECT_TRUE(fff.isCanceled());
}

TEST(FutureTestThen, ErrorForwardingAndThenThen)
{
  qi::Promise<void> p;
  std::atomic<bool> ok;
  std::string error = "zut";
  ok = false;
  auto f0 = p.future();
  auto f1 = f0.andThen([](void*){});
  auto f2 = f1.then([&ok](qi::Future<void>){ ok = true; });
  p.setError(error);
  auto f0State = f0.waitFor(qi::MilliSeconds{100});
  auto f1State = f1.waitFor(qi::MilliSeconds{100});
  auto f2State = f2.waitFor(qi::MilliSeconds{100});
  ASSERT_EQ(qi::FutureState_FinishedWithError, f0State);
  ASSERT_EQ(qi::FutureState_FinishedWithError, f1State);
  ASSERT_EQ(qi::FutureState_FinishedWithValue, f2State);
  ASSERT_TRUE(ok.load());
}

TEST(FutureTestUnwrap, Unwrap)
{
  qi::Promise<qi::Future<int> > prom;
  qi::Promise<int> prom2;
  qi::Future<int> future = prom.future().unwrap();

  ASSERT_TRUE(future.isRunning());

  prom.setValue(prom2.future());

  ASSERT_TRUE(future.isRunning());

  prom2.setValue(42);

  ASSERT_TRUE(test::finishesWithValue(future));
  ASSERT_EQ(42, future.value());
}

TEST(FutureTestUnwrap, UnwrapError)
{
  qi::Promise<qi::Future<int> > prom;
  qi::Promise<int> prom2;
  qi::Future<int> future = prom.future().unwrap();

  ASSERT_TRUE(future.isRunning());

  prom.setError("fail");

  ASSERT_TRUE(test::finishesWithError(future));
  ASSERT_EQ("fail", future.error());
}

TEST(FutureTestUnwrap, UnwrapError2)
{
  qi::Promise<qi::Future<int> > prom;
  qi::Promise<int> prom2;
  qi::Future<int> future = prom.future().unwrap();

  ASSERT_TRUE(future.isRunning());

  prom.setValue(prom2.future());

  ASSERT_TRUE(future.isRunning());

  prom2.setError("fail");

  ASSERT_TRUE(test::finishesWithError(future));
  ASSERT_EQ("fail", future.error());
}

template <typename T>
void setTrue(qi::Promise<T>& p, bool& b)
{
  EXPECT_FALSE(b);
  b = true;

  p.setCanceled();
}

TEST(FutureTestUnwrap, UnwrapCancel)
{
  bool canceled = false;

  qi::Promise<qi::Future<int> > prom(boost::bind(setTrue<qi::Future<int> >, _1, std::ref(canceled)));
  qi::Promise<int> prom2;
  qi::Future<int> future = prom.future().unwrap();

  ASSERT_TRUE(future.isRunning());
  ASSERT_FALSE(canceled);

  future.cancel();

  EXPECT_TRUE(prom.isCancelRequested());
  ASSERT_TRUE(test::finishesAsCanceled(future));
  EXPECT_TRUE(canceled);
}

TEST(FutureTestUnwrap, UnwrapCancel2)
{
  bool canceled = false;
  bool canceled2 = false;

  qi::Promise<qi::Future<int> > prom(boost::bind(setTrue<qi::Future<int> >, _1, std::ref(canceled)));
  qi::Promise<int> prom2(boost::bind(setTrue<int>, _1, std::ref(canceled2)));
  qi::Future<int> future = prom.future().unwrap();

  ASSERT_TRUE(future.isRunning());
  ASSERT_FALSE(canceled);

  prom.setValue(prom2.future());

  ASSERT_TRUE(future.isRunning());
  ASSERT_FALSE(canceled);
  ASSERT_FALSE(canceled2);

  future.cancel();

  EXPECT_FALSE(prom.isCancelRequested());
  EXPECT_TRUE(prom2.isCancelRequested());
  ASSERT_TRUE(test::finishesAsCanceled(future));
  EXPECT_TRUE(future.isCanceled());
  EXPECT_FALSE(canceled);
  EXPECT_TRUE(canceled2);
}

TEST(FutureTestUnwrap, TryUnwrapOnSimpleFuture)
{
  qi::Future<void> truc;
  auto machin = qi::detail::tryUnwrap(truc);
  static_assert(
        std::is_same<decltype(truc), decltype(machin)>::value,
        "Try unwrap messes up a simple future!");
}

TEST(FutureTestUnwrap, TryUnwrapOnFutureOfFutureDirect)
{
  auto machin = qi::detail::tryUnwrap(
        qi::Future<qi::Future<void>>{qi::Future<void>{nullptr}});
  static_assert(
        std::is_same<decltype(machin), qi::Future<void>>::value,
        "Try unwrap does not unwrap future of futures!");
}

TEST(FutureTestUnwrap, TryUnwrapOnFutureOfFutureIntermediateVariable)
{
  auto truc = qi::Future<qi::Future<void>>{qi::Future<void>{nullptr}};
  auto machin = qi::detail::tryUnwrap(truc);
  static_assert(
        std::is_same<decltype(machin), qi::Future<void>>::value,
        "Try unwrap does not unwrap future of futures!");
}

TEST(FutureTestUnwrap, TryUnwrapOnFutureOfFutureDeprecatedSignature)
{
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
  auto machin = qi::detail::tryUnwrap(
        qi::Future<qi::Future<void>>{qi::Future<void>{nullptr}}, 0);
  static_assert(
        std::is_same<decltype(machin), qi::Future<void>>::value,
        "Try unwrap does not unwrap future of futures!");
KA_WARNING_POP()
}

TEST(FutureTestUnwrap, TryUnwrapOnValue)
{
  ASSERT_EQ(42, qi::detail::tryUnwrap(42));
}

TEST(FutureTestWeakCanceler, Cancel)
{
  qi::Promise<void> prom;
  prom.future().makeCanceler()();
  ASSERT_TRUE(prom.isCancelRequested());
}

TEST(FutureTestWeakCanceler, IsWeak)
{
  boost::weak_ptr<int> wptr;
  boost::function<void()> canceler;
  {
    qi::Promise<boost::shared_ptr<int> > prom;
    canceler = prom.future().makeCanceler();
    prom.setValue(boost::make_shared<int>(42));
    wptr = prom.future().value();
  }
  ASSERT_FALSE(wptr.lock());
}

// ===== FutureBarrier =========================================================
#define BARRIER_N 10

static void addMe(qi::Atomic<int>* a) {
  ++(*a);
}

static void mult42(qi::Promise<int> prom, int number) {
  prom.setValue(42 * number);
}

static void checkBarrier(qi::Promise<void> prom,
                         qi::Future<std::vector< qi::Future<int>>> futResults)
{
  const auto results = futResults.value();
  for (int it = 0; it < BARRIER_N; ++it) {
    qi::Future<int> fut = results[it];

    ASSERT_FALSE(fut.hasError());
    ASSERT_EQ(it * 42, fut.value());
  }
  prom.setValue(0);
}

TEST(FutureBarrier, SimpleBarrier)
{
  int it;
  qi::Atomic<int> a;
  qi::FutureBarrier<void> barrier;
  qi::Promise<void> call;

  // Loading the barrier.
  for (it = 0; it < BARRIER_N; ++it) {
    // function call in reality here.
    call.future().connect(boost::bind<void>(&addMe, &a));
    barrier.addFuture(call.future());
  }

  // Launch all addMe (just think of it like function calls returned)
  call.setValue(0);

  // We wait for all futures of the for loop.
  barrier.future().wait();
  std::this_thread::sleep_for(std::chrono::milliseconds{10});
  ASSERT_EQ(it, a.load());
}

TEST(FutureBarrier, Cancel)
{
  qi::Future<std::vector<qi::Future<int> > > fut;
  {
    qi::FutureBarrier<int> barrier;
    std::vector<qi::Promise<int> > promises;

    // Loading the barrier.
    for (int it = 0; it < BARRIER_N; ++it) {
      qi::Promise<int> prom(doCancel);
      promises.push_back(prom);
      barrier.addFuture(prom.future());
    }

    fut = barrier.future();
  }

  fut.cancel();
  // We wait for all futures of the for loop.
  fut.value();
}

TEST(FutureBarrier, ClosedBarrier)
{
  qi::FutureBarrier<void> barrier;

  // Can add a future to the barrier because is not yet closed.
  qi::Promise<void> prom;
  ASSERT_NO_THROW(barrier.addFuture(prom.future()));
  prom.setValue(0);

  barrier.future().wait();

  // Invalid promise, because FutureBarrier is closed.
  qi::Promise<void> prom2;
  ASSERT_ANY_THROW(barrier.addFuture(prom2.future()));
}

TEST(FutureBarrier, CompleteExample)
{
  qi::Promise<void> end;

  {
    qi::Promise<void> call;
    qi::FutureBarrier<int> barrier;

    // Load data in the barrier.
    for (int it = 0; it < BARRIER_N; ++it) {
      qi::Promise<int> prom;
      call.future().connect(boost::bind<void>(&mult42, prom, it));
      barrier.addFuture(prom.future());
    }
    call.setValue(0);

    // Bind something to do after everything is computed.
    barrier.future().connect(boost::bind(&checkBarrier, end, _1));
  }

  // Wait for the end of the check.
  end.future().wait();
}

TEST(FutureBarrier, errorAmongstSuccesses)
{
  qi::FutureBarrier<void> barrier;
  barrier.addFuture(qi::Future<void>{0});
  barrier.addFuture(qi::makeFutureError<void>("holy moses"));
  auto futures = barrier.future().value(200);
  int errorCount = 0;
  for (auto& future: futures)
  {
    if (future.hasError())
      ++errorCount;
  }
  ASSERT_EQ(1, errorCount);
}

qi::Future<int> emulateSet(int it, bool error = false) {
  qi::Promise<int> prom;

  if (error) {
    prom.setError("ERROR");
  } else {
    prom.setValue(it);
  }
  return prom.future();
}

TEST(FutureTestWaitForAll, SimpleTest) {
  std::vector< qi::Future<int> > vect;

  for (int it = 0; it < BARRIER_N; ++it) {
    vect.push_back(emulateSet(it));
  }
  qi::waitForAll<int>(vect);
  for (int it = 0; it < BARRIER_N; ++it) {
    ASSERT_FALSE(vect.at(it).hasError());
    ASSERT_EQ(vect.at(it).value(), it);
  }
}

TEST(FutureTestWaitForFirst, SuccessfulTest) {
  std::vector< qi::Future<int> > vect;

  for (int it = 0; it < BARRIER_N; ++it) {
    vect.push_back(emulateSet(it, it != 3));
  }
  qi::Future<int> a = qi::waitForFirst<int>(vect).value();
  ASSERT_FALSE(a.hasError());
  ASSERT_EQ(a.value(), 3);
}

TEST(FutureTestWaitForFirst, FailingTest) {
  std::vector< qi::Future<int> > vect;

  for (int it = 0; it < BARRIER_N; ++it) {
    vect.push_back(emulateSet(it, true));
  }
  qi::Future<int> a = qi::waitForFirst<int>(vect).value();
  ASSERT_TRUE(a.hasError());
}

namespace
{
  struct SetCanceled
  {
    void operator()(qi::Promise<void>& p) const
    {
      p.setCanceled();
    }
  };
}

TEST(TestCancelOnTimeout, TimeoutCauseCancel)
{
  using namespace qi;
  Promise<void> p{SetCanceled{}};
  auto fut = p.future();
  cancelOnTimeout(fut, MilliSeconds{1});
  ASSERT_EQ(FutureState_Canceled, fut.wait(defaultWaitTimeout));
}

TEST(TestCancelOnTimeout, AlreadyFinishedWithValue)
{
  using namespace qi;
  Promise<void> p{SetCanceled{}};
  auto fut = p.future();
  p.setValue(nullptr);
  cancelOnTimeout(fut, MilliSeconds{1});
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultWaitTimeout));
}

TEST(TestCancelOnTimeout, AlreadyFinishedWithError)
{
  using namespace qi;
  Promise<void> p{SetCanceled{}};
  auto fut = p.future();
  p.setError("");
  cancelOnTimeout(fut, MilliSeconds{1});
  ASSERT_EQ(FutureState_FinishedWithError, fut.wait(defaultWaitTimeout));
}

TEST(TestCancelOnTimeout, TimeoutCauseCancelAsync)
{
  using namespace qi;
  auto fut = async([]() {
    std::this_thread::sleep_for(std::chrono::seconds{3});
  });
  cancelOnTimeout(fut, MilliSeconds{1});

  // No cancel callback was specified, so the future is still running.
  ASSERT_EQ(FutureState_Running, fut.wait(defaultWaitTimeout));
}

TEST(FutureTestAdaptFuture, WithVoid) {
  qi::Promise<void> prom1;
  qi::Promise<void> prom2;
  prom1.setError("foo");

  qi::adaptFuture(prom1.future(), prom2);
  while(prom2.future().isRunning())
    std::this_thread::sleep_for(std::chrono::milliseconds{5});
  ASSERT_TRUE(prom2.future().hasError());
  ASSERT_FALSE(prom2.future().hasValue());
  ASSERT_STREQ("foo", prom2.future().error().c_str());
}

TEST(FutureTestAdaptFuture, WithInt) {
  qi::Promise<int> prom1;
  qi::Promise<int> prom2;
  prom1.setValue(1);

  qi::adaptFuture(prom1.future(), prom2);
  while(prom2.future().isRunning())
    std::this_thread::sleep_for(std::chrono::milliseconds{5});
  ASSERT_TRUE(prom2.future().hasValue());
  ASSERT_FALSE(prom2.future().hasError());
  ASSERT_EQ(1, prom2.future().value());
}

TEST(FutureTestAdaptFuture, WithIntVoid) {
  qi::Promise<int> prom1;
  qi::Promise<void> prom2;
  prom1.setValue(1);

  qi::adaptFuture(prom1.future(), prom2);
  while(prom2.future().isRunning())
    std::this_thread::sleep_for(std::chrono::milliseconds{5});
  ASSERT_TRUE(prom2.future().hasValue());
  ASSERT_FALSE(prom2.future().hasError());
  ASSERT_EQ(NULL, prom2.future().value());
}

TEST(FutureTestAdaptFuture, PromiseCanceled) {
  qi::Promise<void> prom1;
  qi::Promise<void> prom2;
  prom1.setCanceled();

  qi::adaptFuture(prom1.future(), prom2);
  while(prom2.future().isRunning())
    std::this_thread::sleep_for(std::chrono::milliseconds{5});
  ASSERT_TRUE(prom2.future().isCanceled());
  ASSERT_FALSE(prom2.future().hasValue());
  ASSERT_FALSE(prom2.future().hasError());
}

void handleCancel(qi::Promise<void> p)
{
  p.setCanceled();
}

TEST(FutureTestAdaptFuture, PromiseCancel) {
  qi::Promise<void> prom1(handleCancel);
  qi::Promise<void> prom2;

  qi::adaptFuture(prom1.future(), prom2);
  prom2.future().cancel();
  while(prom2.future().isRunning())
    std::this_thread::sleep_for(std::chrono::milliseconds{5});
  ASSERT_TRUE(prom2.future().isCanceled());
  ASSERT_FALSE(prom2.future().hasValue());
  ASSERT_FALSE(prom2.future().hasError());
}

TEST(FutureTestUnitFuture, Regular)
{
  using namespace qi;
  UnitFuture unit; // There is only one value.
  ASSERT_TRUE(ka::is_regular({unit}));
}

TEST(FutureTestUnitFuture, BasicNonVoid)
{
  using namespace qi;
  UnitFuture unit; // There is only one value.
  const int i = 3;
  Future<int> fut = unit(i);
  ASSERT_EQ(i, fut.value());
}

TEST(FutureTestUnitFuture, BasicVoid)
{
  using namespace qi;
  UnitFuture unit; // There is only one value.
  Future<void> fut = unit();
  ASSERT_EQ(nullptr, fut.value());
}

TEST(FutureTestFuturized, returnVoidNoArgument)
{
  using namespace qi;
  bool wasCalled = false;
  auto k = futurizeOutput([&] { wasCalled = true; });
  static_assert(std::is_same<qi::Future<void>, decltype(k())>::value, "");
  ASSERT_FALSE(wasCalled);
  k();
  ASSERT_TRUE(wasCalled);
}

TEST(FutureTestFuturized, returnValueNoArgument)
{
  using namespace qi;
  bool wasCalled = false;
  auto k = futurizeOutput([&] { wasCalled = true; return 42; });
  static_assert(std::is_same<qi::Future<int>, decltype(k())>::value, "");
  ASSERT_FALSE(wasCalled);
  int x = k().value();
  ASSERT_TRUE(wasCalled);
  ASSERT_EQ(42, x);
}

TEST(FutureTestFuturized, returnVoidWithArgument)
{
  using namespace qi;
  bool wasCalled = false;
  auto k = futurizeOutput([&](int, int) { wasCalled = true; });
  static_assert(std::is_same<qi::Future<void>, decltype(k(1, 2))>::value, "");
  ASSERT_FALSE(wasCalled);
  k(1, 2);
  ASSERT_TRUE(wasCalled);
}

TEST(TestFuturized, returnValueWithArgument)
{
  using namespace qi;
  bool wasCalled = false;
  auto k = futurizeOutput([&](int x, int y) { wasCalled = true; return x + y; });
  static const int a = 1;
  static const int b = 2;
  static_assert(std::is_same<qi::Future<int>, decltype(k(a, b))>::value, "");
  ASSERT_FALSE(wasCalled);
  int x = k(a, b).value();
  ASSERT_TRUE(wasCalled);
  ASSERT_EQ(a+b, x);
}

TEST(FutureSrc, SrcFuture)
{
  using namespace qi;
  SrcFuture src;
  const int i = 5;
  ASSERT_EQ(i, src(Future<int>{i}));
}

TEST(FutureSrc, SrcFutureUnitFuture)
{
  using namespace qi;
  SrcFuture src;
  UnitFuture unit;
  const int i = 5;
  ASSERT_EQ(i, src(unit(i)));
  Future<int> f{i};
  ASSERT_EQ(f.value(), unit(src(f)).value());
}

TEST(FutureSrc, SrcFutureUnitFutureCompose)
{
// VS2013 does not allow to do composition simplifications.
#if !KA_COMPILER_VS2013_OR_BELOW
  using namespace qi;
  using namespace ka;
  using namespace ka::functional_ops;
  SrcFuture src;
  UnitFuture unit;
  static_assert(Equal<decltype(src * unit), id_transfo_t>::value, "");
  static_assert(Equal<decltype(unit * src), id_transfo_t>::value, "");
  id_transfo_t _1;
  ASSERT_EQ(src * unit, _1);
  ASSERT_EQ(unit * src, _1);
#endif
}

namespace
{
  // Makes destruction visible from outside, by inserting the address of
  // destroyed objects into a persistent set.
  // The user is responsible for the set's lifetime and synchronization.
  struct X
  {
    std::set<const X*>* destroyed;
    X(std::set<const X*>* destroyed = nullptr) : destroyed(destroyed)
    {
    }
    ~X()
    {
      if (destroyed) destroyed->insert(this);
    }
  };
}

namespace
{
  struct UnitFutureSync
  {
    // Constructor added because of an error with Clang on Mac.
    // TODO: Remove this constructor when Clang is upgraded.
    UnitFutureSync()
    {
    }

    template<typename T>
    qi::FutureSync<ka::Decay<T>> operator()(T&& t) const
    {
      return qi::FutureSync<ka::Decay<T>>{std::forward<T>(t)};
    }

    qi::FutureSync<void> operator()() const
    {
      return qi::FutureSync<void>{nullptr};
    }
  };

  static const qi::MilliSeconds longTimeout = qi::Seconds{100};
}

TEST(Future, ValueThrowsOnTimeoutAsMilliSeconds)
{
  using namespace qi;
  using namespace std::chrono;
  const int ms = 100;
  Promise<int> prom;

  const auto begin = steady_clock::now();
  EXPECT_THROW(prom.future().value(MilliSeconds{ms}), FutureException);
  const auto end = steady_clock::now();

  // Tests that we have waited at least the required duration.
  const auto delta = duration_cast<milliseconds>(end - begin).count();
  ASSERT_GE(delta, ms);
}

TEST(Future, ValueTimeoutAsMilliSeconds)
{
  using namespace qi;
  const int i = 232143;
  Promise<int> prom;
  std::thread t{[=]() mutable {
    std::this_thread::sleep_for(std::chrono::seconds{1});
    prom.setValue(i);
  }};
  EXPECT_EQ(i, prom.future().value(longTimeout));
  t.join();
}

TEST(Future, ValueTimeoutAsInfinity)
{
  using namespace qi;
  const int i = 232143;
  Promise<int> prom;
  std::thread t{[=]() mutable {
    std::this_thread::sleep_for(std::chrono::seconds{1});
    prom.setValue(i);
  }};
  EXPECT_EQ(i, prom.future().value(Infinity{}));
  t.join();
}

template<typename T>
struct FutureValue : testing::Test {
};

using unit_types = testing::Types<qi::UnitFuture, UnitFutureSync>;

// Make `FutureValue` tests with `Future` and `FutureSync`.
TYPED_TEST_SUITE(FutureValue, unit_types);

namespace
{
  template<typename T>
  T* underlyingAddress(T& x)
  {
    return &x;
  }

  template<typename T>
  T* underlyingAddress(T* x)
  {
    return x;
  }

  template<typename T>
  T* underlyingAddress(const boost::shared_ptr<T>& x)
  {
    return x.get();
  }
} // namespace

// Illustrates a case where we get a dangling reference on Future's value.
// See `FutureValue.SafeValue` for a safe alternative.
TYPED_TEST(FutureValue, UnsafeValue)
{
  using UnitFuture = TypeParam;
  using namespace qi;
  std::set<const X*> destroyed;
  const X a(&destroyed);
  const UnitFuture future;

  // Creates a Future from `a` and returns its value *by constant reference*.
  const X& b = future(a).value();

  // Here the future has been destroyed, and so is its underlying `X` value.
  // Therefore, `b` is dangling reference.
  const bool bIsDestroyed = destroyed.find(underlyingAddress(b)) != destroyed.end();
  EXPECT_TRUE(bIsDestroyed);
}

namespace
{
  /// Function<Readable<T> (qi::Future<T>)> F0,
  /// Function<qi::Future<T> (T)> F1
  template<typename F0, typename F1>
  void testSafeValue(F0 safeValue, F1 future)
  {
    using namespace qi;
    using ka::src;
    std::set<const X*> destroyed;
    const X a(&destroyed);

    // Creates a Future from `a` and returns its value in a safe way (by copy,
    // by `shared_ptr`, etc.).
    const auto& b = safeValue(future(a));

    // Here the future has been destroyed, but `safeValue` offers a guarantee
    // that `b` is still valid.
    const bool bIsDestroyed = destroyed.find(underlyingAddress(b)) != destroyed.end();
    EXPECT_FALSE(bIsDestroyed);
  }
}

// Safe alternative to `FutureValue.UnsafeValue`. The only difference is the
// use of `valueCopy()` instead of `value()`.
TYPED_TEST(FutureValue, SafeValue)
{
  using UnitFuture = TypeParam;
  using namespace qi;
  SCOPED_TRACE("SafeValue");
  testSafeValue([](const Future<X>& f) {return f.valueCopy();}, UnitFuture{});
  testSafeValue([](const Future<X>& f) {return f.valueCopy(longTimeout);}, UnitFuture{});
  testSafeValue([](const Future<X>& f) {return f.valueCopy(Infinity{});}, UnitFuture{});
}

// `*` behaves as `valueCopy`.
TYPED_TEST(FutureValue, OperatorStar)
{
  using UnitFuture = TypeParam;
  using namespace qi;
  SCOPED_TRACE("OperatorStar");
  testSafeValue([](const Future<X>& f) {return *f;}, UnitFuture{});
}

// Second safe alternative to `FutureValue.UnsafeValue`. The only difference is
// the use of `valueSharedPtr()` instead of `value()`: the returned `shared_ptr`
// extends the lifetime of the `Future`.
TYPED_TEST(FutureValue, SafeValuePtr)
{
  using UnitFuture = TypeParam;
  using namespace qi;
  SCOPED_TRACE("SafeValuePtr");
  testSafeValue([](const Future<X>& f) {return f.valueSharedPtr();}, UnitFuture{});
  testSafeValue([](const Future<X>& f) {return f.valueSharedPtr(longTimeout);}, UnitFuture{});
  testSafeValue([](const Future<X>& f) {return f.valueSharedPtr(Infinity{});}, UnitFuture{});
}

TEST(FutureErrorFromException, WithIdentity)
{
  auto res = ka::invoke_catch(
    qi::futureErrorFromException<int>(),
    []() -> qi::Future<int> {
      throw std::runtime_error("an exception");
    }
  );

  std::string err;
  EXPECT_TRUE(test::finishesWithError(res, test::willAssignError(err)));
  EXPECT_NE(std::string::npos, err.find("an exception"));
}

TEST(FutureErrorFromException, WithTransfo)
{
  const std::string prefix = "test - ";
  auto addPrefix = [&](const std::string& msg){
    return prefix + msg;
  };
  auto res = ka::invoke_catch(
    qi::futureErrorFromException<int>(addPrefix),
    []() -> qi::Future<int> {
      throw std::runtime_error("an exception");
    }
  );

  std::string err;
  EXPECT_TRUE(test::finishesWithError(res, test::willAssignError(err)));
  EXPECT_EQ(0u, err.find(prefix)) << err;
  EXPECT_NE(std::string::npos, err.find("an exception"));
}

TEST(ToAnyValueFuture, Basic)
{
  using namespace qi;
  Promise<int> prom;
  auto fut = toAnyValueFuture(prom.future());
  prom.setValue(42);
  EXPECT_EQ(AnyValue::from(42), fut.value());
}

TEST(ToAnyValueFuture, Void)
{
  using namespace qi;
  Promise<void> prom;
  auto fut = toAnyValueFuture(prom.future());
  prom.setValue(nullptr);
  EXPECT_EQ(AnyValue::make<void>(), fut.value());
}

