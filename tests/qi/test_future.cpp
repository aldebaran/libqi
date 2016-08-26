/*
**
** Copyright (C) 2010, 2012, 2013, Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <qi/application.hpp>
#include <qi/future.hpp>
#include <qi/log.hpp>
#include "test_future.hpp"

qiLogCategory("test");

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

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

int SetValue::delayExchange(int msDelay, int value)
{
  qiLogDebug("test") << "delayexchange enter";
  state = 1;
  qi::os::msleep(msDelay);
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

void SetValue2::delayExchangeP(int msDelay, int value, qi::Promise<int> result)
{
  result.setValue(delayExchange(msDelay, value));
}

qi::Future<int> SetValue2::asyncDelayExchange(int msDelay, int value)
{
  qi::Promise<int> promise;
  std::thread(&SetValue2::delayExchangeP, this, msDelay, value, promise);
  return promise.future();
}

int block(int i, qi::Future<void> f)
{
  f.wait();
  return 99;
}

int get42() { return 42; }


class TestFuture : public ::testing::Test
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

int         TestFuture::gGlobalI;
std::string TestFuture::gGlobalS;
std::string TestFuture::gGlobalE;
qi::Atomic<int> TestFuture::gSuccess;

class TestFutureI {
public:
  TestFutureI(int &gGlobalI, std::string &gGlobalE)
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

class TestFutureS {
public:
  TestFutureS(std::string &gGlobalS, std::string &gGlobalE)
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

TEST_F(TestFuture, Validity) {
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

TEST_F(TestFuture, Invalid) {
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

TEST_F(TestFuture, SimpleType) {
  TestFutureI tf(gGlobalI, gGlobalE);

  qi::Promise<int> pro(qi::FutureCallbackType_Sync);

  qi::Future<int>  fut = pro.future();

  fut.connect(boost::bind(&TestFutureI::onFutureFinished, tf, _1));

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

TEST_F(TestFuture, ComplexType) {
  TestFutureS tf(gGlobalS, gGlobalE);

  qi::Promise<std::string> pro(qi::FutureCallbackType_Sync);

  qi::Future<std::string>  fut = pro.future();

  fut.connect(boost::bind(&TestFutureS::onFutureFinished, tf, _1));

  EXPECT_STREQ("", gGlobalS.c_str());
  EXPECT_FALSE(fut.isFinished());
  pro.setValue("42");
  EXPECT_TRUE(fut.isFinished());
  EXPECT_STREQ("42", fut.value().c_str());
  EXPECT_STREQ("42", gGlobalS.c_str());
}

void producer(qi::Promise<int> pro) {
  qi::os::msleep(100);
  pro.setValue(42);
}

void consumer(qi::Atomic<int> &gSuccess, qi::Future<int> fut) {
  //wont block thread on error
  ASSERT_TRUE(fut.wait(1000) == qi::FutureState_FinishedWithValue);
  EXPECT_EQ(42, fut.value());
  ++gSuccess;
}

TEST_F(TestFuture, Threaded) {
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

TEST_F(TestFuture, TestTimeout) {
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
  EXPECT_LT(end, start + qi::MilliSeconds(40))
      << "Timeout took " << boost::chrono::duration_cast<qi::MilliSeconds>(end - start).count() << "ms, which is too much!";
}

TEST_F(TestFuture, TestError) {
  TestFutureI tf(gGlobalI, gGlobalE);

  qi::Promise<int> pro(qi::FutureCallbackType_Sync);

  qi::Future<int>  fut = pro.future();
  fut.connect(boost::bind(&TestFutureI::onFutureFinished, tf, _1));

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

TEST_F(TestFuture, TestStateNone)
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

TEST(TestFuture2, TestBroken)
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

TEST_F(TestFuture, TestPromiseAdapter)
{
  boost::weak_ptr<DelCheck> wdc;
  {
    qi::Promise<int> p;
    qi::Promise<int> p2;

    // Prepare for test for leakage
    {
      boost::shared_ptr<DelCheck> dc = boost::make_shared<DelCheck>();
      wdc = dc;
      p.future().connect(&DelCheck::foo, dc);
      p2.future().connect(&DelCheck::foo, dc);
    }
    EXPECT_FALSE(wdc.expired());

    qi::adaptFuture(p.future(), p2);
    p.setValue(42);
    // wait for the result to be forwarded
    qi::os::msleep(100);
    EXPECT_EQ(42, p2.value());
  }
  // Test for leakage
  EXPECT_TRUE(wdc.expired());
}

void justThrow()
{
  throw std::runtime_error("Expected error.");
}

TEST(AsyncAndFuture, errorOnTaskThrow)
{
  qi::Future<void> f = qi::async(&justThrow);
  EXPECT_TRUE(f.hasError());
}

void unlock(qi::Promise<int> prom, std::atomic<bool>& tag)
{
  tag = true;
  prom.setValue(1);
}

TEST(TestFutureSync, Basic)
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

TEST(TestFutureSync, InSitu)
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

TEST(TestFutureSync, ThrowOnDestroy) {
  qi::Promise<int> prom;

  prom.setError("touctouc");
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future()));
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future())); //multiple futuresync on the same future should fail.
  EXPECT_THROW(qi::FutureSync<int>(prom.future()).value(), std::runtime_error); //value should fail.
}

TEST(TestFutureSync, NoThrow) {
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

void do_nothing(TestFutureI*) {}

TEST(TestFutureError, MultipleSetValue)
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

TEST(TestFutureError, ValueOnError)
{
  qi::Promise<int> p;
  qi::Future<int> f = p.future();
  p.setError("foo");
  EXPECT_ANY_THROW({ f.value();});
}


TEST(TestFutureCancel, AsyncCallCanceleable)
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
static void doNothing(qi::Promise<int> promise) { ; }

TEST(TestFutureCancel, CancelRequest)
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

TEST(TestFutureCancel, Canceleable)
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

TEST(TestFutureThen, ThenR)
{
  qi::Future<int> f = qi::async(&get42);
  qi::Future<int> ff = f.thenR<int>(&assinc, _1, 42);
  qi::Future<int> fff = ff.thenR<int>(&assinc, _1, 43);

  ASSERT_EQ(44, fff.value());
}

TEST(TestFutureThen, Then)
{
  qi::Future<int> f = qi::async(&get42);
  qi::Future<int> ff = f.then(qi::bind(&assinc, _1, 42));
  qi::Future<int> fff = ff.then(qi::bind(&assinc, _1, 43));

  ASSERT_EQ(44, fff.value());
}

TEST(TestFutureThen, ThenCancel)
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

int fail(int f)
{
  throw std::runtime_error("fail");
}

int call(bool& b)
{
  b = true;
  return 42;
}

TEST(TestFutureThen, AndThenR)
{
  bool called = false;
  qi::Future<int> f = qi::async(&get42);
  qi::Future<int> ff = f.andThenR<int>(boost::bind(&fail, _1));
  qi::Future<int> fff = ff.andThenR<int>(boost::bind(&call, std::ref(called)));

  fff.wait();

  EXPECT_FALSE(called);
  ASSERT_TRUE(fff.hasError());
  EXPECT_EQ(fff.error(), "fail");
}

TEST(TestFutureThen, AndThen)
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

TEST(TestFutureThen, AndThenRVoid)
{
  bool called = false;
  qi::Promise<void> p;
  qi::Future<void> ff = p.future().andThenR<void>(boost::bind(&call, std::ref(called)));
  p.setValue(0);

  ff.wait();

  EXPECT_TRUE(called);
  ASSERT_TRUE(ff.hasValue());
}

TEST(TestFutureThen, AndThenRCancel)
{
  qi::Promise<void> blockProm;

  bool called = false;
  qi::Future<int> f = qi::Future<int>(42);
  qi::Future<int> ff = f.andThenR<int>(boost::bind(&block, _1, blockProm.future()));
  qi::Future<int> fff = ff.andThenR<int>(boost::bind(&call, std::ref(called)));

  fff.cancel();
  blockProm.setValue(0);
  fff.wait();

  EXPECT_FALSE(called);
  EXPECT_TRUE(fff.isCanceled());
}

TEST(TestFutureUnwrap, Unwrap)
{
  qi::Promise<qi::Future<int> > prom;
  qi::Promise<int> prom2;
  qi::Future<int> future = prom.future().unwrap();

  ASSERT_TRUE(future.isRunning());

  prom.setValue(prom2.future());

  ASSERT_TRUE(future.isRunning());

  prom2.setValue(42);

  // TODO remove this when we have synchronous callbacks
  qi::os::msleep(50);

  ASSERT_TRUE(future.isFinished());
  ASSERT_EQ(42, future.value());
}

TEST(TestFutureUnwrap, UnwrapError)
{
  qi::Promise<qi::Future<int> > prom;
  qi::Promise<int> prom2;
  qi::Future<int> future = prom.future().unwrap();

  ASSERT_TRUE(future.isRunning());

  prom.setError("fail");

  // TODO remove this when we have synchronous callbacks
  qi::os::msleep(50);

  ASSERT_TRUE(future.hasError());
  ASSERT_EQ("fail", future.error());
}

TEST(TestFutureUnwrap, UnwrapError2)
{
  qi::Promise<qi::Future<int> > prom;
  qi::Promise<int> prom2;
  qi::Future<int> future = prom.future().unwrap();

  ASSERT_TRUE(future.isRunning());

  prom.setValue(prom2.future());

  ASSERT_TRUE(future.isRunning());

  prom2.setError("fail");

  // TODO remove this when we have synchronous callbacks
  qi::os::msleep(50);

  ASSERT_TRUE(future.hasError());
  ASSERT_EQ("fail", future.error());
}

template <typename T>
void setTrue(qi::Promise<T>& p, bool& b)
{
  ASSERT_FALSE(b);
  b = true;

  p.setCanceled();
}

TEST(TestFutureUnwrap, UnwrapCancel)
{
  bool canceled = false;

  qi::Promise<qi::Future<int> > prom(boost::bind(setTrue<qi::Future<int> >, _1, std::ref(canceled)));
  qi::Promise<int> prom2;
  qi::Future<int> future = prom.future().unwrap();

  ASSERT_TRUE(future.isRunning());
  ASSERT_FALSE(canceled);

  future.cancel();

  // TODO remove this when we have synchronous callbacks
  qi::os::msleep(50);

  EXPECT_TRUE(prom.isCancelRequested());
  EXPECT_TRUE(future.isCanceled());
  EXPECT_TRUE(canceled);
}

TEST(TestFutureUnwrap, UnwrapCancel2)
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

  // TODO remove this when we have synchronous callbacks
  qi::os::msleep(50);

  EXPECT_FALSE(prom.isCancelRequested());
  EXPECT_TRUE(prom2.isCancelRequested());
  EXPECT_TRUE(future.isCanceled());
  EXPECT_FALSE(canceled);
  EXPECT_TRUE(canceled2);
}

TEST(TestFutureUnwrap, TryUnwrapOnSimpleFuture)
{
  qi::Future<void> truc;
  auto machin = qi::detail::tryUnwrap(truc);
  static_assert(
        std::is_same<decltype(truc), decltype(machin)>::value,
        "Try unwrap messes up a simple future!");
}

TEST(TestFutureUnwrap, TryUnwrapOnFutureOfFutureDirect)
{
  auto machin = qi::detail::tryUnwrap(
        qi::Future<qi::Future<void>>{qi::Future<void>{nullptr}});
  static_assert(
        std::is_same<decltype(machin), qi::Future<void>>::value,
        "Try unwrap does not unwrap future of futures!");
}

TEST(TestFutureUnwrap, TryUnwrapOnFutureOfFutureIntermediateVariable)
{
  auto truc = qi::Future<qi::Future<void>>{qi::Future<void>{nullptr}};
  auto machin = qi::detail::tryUnwrap(truc);
  static_assert(
        std::is_same<decltype(machin), qi::Future<void>>::value,
        "Try unwrap does not unwrap future of futures!");
}

TEST(TestFutureUnwrap, TryUnwrapOnFutureOfFutureDeprecatedSignature)
{
  auto machin = qi::detail::tryUnwrap(
        qi::Future<qi::Future<void>>{qi::Future<void>{nullptr}}, 0);
  static_assert(
        std::is_same<decltype(machin), qi::Future<void>>::value,
        "Try unwrap does not unwrap future of futures!");
}

TEST(TestFutureUnwrap, TryUnwrapOnValue)
{
  ASSERT_EQ(42, qi::detail::tryUnwrap(42));
}

TEST(TestFutureWeakCanceler, Cancel)
{
  qi::Promise<void> prom;
  prom.future().makeCanceler()();
  ASSERT_TRUE(prom.isCancelRequested());
}

TEST(TestFutureWeakCanceler, IsWeak)
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
                         std::vector< qi::Future<int> > results)
{
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
  qi::os::msleep(10);
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

TEST(TestWaitForAll, SimpleTest) {
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

TEST(TestWaitForFirst, SuccessfulTest) {
  std::vector< qi::Future<int> > vect;

  for (int it = 0; it < BARRIER_N; ++it) {
    vect.push_back(emulateSet(it, it != 3));
  }
  qi::Future<int> a = qi::waitForFirst<int>(vect);
  ASSERT_FALSE(a.hasError());
  ASSERT_EQ(a.value(), 3);
}

TEST(TestWaitForFirst, FailingTest) {
  std::vector< qi::Future<int> > vect;

  for (int it = 0; it < BARRIER_N; ++it) {
    vect.push_back(emulateSet(it, true));
  }
  qi::Future<int> a = qi::waitForFirst<int>(vect);
  ASSERT_TRUE(a.hasError());
}

TEST(TestAdaptFuture, WithVoid) {
  qi::Promise<void> prom1;
  qi::Promise<void> prom2;
  prom1.setError("foo");

  qi::adaptFuture(prom1.future(), prom2);
  while(prom2.future().isRunning())
    qi::os::msleep(5);
  ASSERT_TRUE(prom2.future().hasError());
  ASSERT_FALSE(prom2.future().hasValue());
  ASSERT_STREQ("foo", prom2.future().error().c_str());
}

TEST(TestAdaptFuture, WithInt) {
  qi::Promise<int> prom1;
  qi::Promise<int> prom2;
  prom1.setValue(1);

  qi::adaptFuture(prom1.future(), prom2);
  while(prom2.future().isRunning())
    qi::os::msleep(5);
  ASSERT_TRUE(prom2.future().hasValue());
  ASSERT_FALSE(prom2.future().hasError());
  ASSERT_EQ(1, prom2.future().value());
}

TEST(TestAdaptFuture, WithIntVoid) {
  qi::Promise<int> prom1;
  qi::Promise<void> prom2;
  prom1.setValue(1);

  qi::adaptFuture(prom1.future(), prom2);
  while(prom2.future().isRunning())
    qi::os::msleep(5);
  ASSERT_TRUE(prom2.future().hasValue());
  ASSERT_FALSE(prom2.future().hasError());
  ASSERT_EQ(NULL, prom2.future().value());
}

TEST(TestAdaptFuture, PromiseCanceled) {
  qi::Promise<void> prom1;
  qi::Promise<void> prom2;
  prom1.setCanceled();

  qi::adaptFuture(prom1.future(), prom2);
  while(prom2.future().isRunning())
    qi::os::msleep(5);
  ASSERT_TRUE(prom2.future().isCanceled());
  ASSERT_FALSE(prom2.future().hasValue());
  ASSERT_FALSE(prom2.future().hasError());
}

void handleCancel(qi::Promise<void> p)
{
  p.setCanceled();
}

TEST(TestAdaptFuture, PromiseCancel) {
  qi::Promise<void> prom1(handleCancel);
  qi::Promise<void> prom2;

  qi::adaptFuture(prom1.future(), prom2);
  prom2.future().cancel();
  while(prom2.future().isRunning())
    qi::os::msleep(5);
  ASSERT_TRUE(prom2.future().isCanceled());
  ASSERT_FALSE(prom2.future().hasValue());
  ASSERT_FALSE(prom2.future().hasError());
}

