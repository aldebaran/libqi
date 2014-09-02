/*
**
** Copyright (C) 2010, 2012, 2013, Aldebaran Robotics
*/

#include <map>
#include <string>
#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/atomic.hpp>
#include <qi/application.hpp>
#include <qi/future.hpp>
#include <qi/eventloop.hpp>
#include <qi/trackable.hpp>
#include <qi/periodictask.hpp>
#include <qi/strand.hpp>

#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4355 )
#endif

qiLogCategory("test");
class SetValue: private boost::noncopyable
{
public:
  SetValue(int& tgt)
    : target(tgt)
    , state(0)
  {
  }

  int exchange(int v)
  {
    int old = target;
    target = v;
    return old;
  }

  int delayExchange(int msDelay, int value)
  {
    qiLogDebug() << "delayexchange enter";
    state = 1;
    qi::os::msleep(msDelay);
    state = 2;
    qiLogDebug() << "delayexchange leave";
    return exchange(value);
  }

  int& target;
  int state;
private:

};
int exchange(int& target, int value)
{
  int old = target;
  target = value;
  return old;
}

TEST(TestBind, Simple)
{
  int v = 0;
  qi::bind<void(int)>(&exchange, boost::ref(v), _1)(15);
  EXPECT_EQ(15, v);
  qi::bind<void(void)>(&exchange, boost::ref(v), 16)();
  EXPECT_EQ(16, v);
}

TEST(TestBind, MemFun)
{
  int v = 0;
  SetValue s1(v);
  qi::bind<void(int)>(&SetValue::exchange, &s1, _1)(1);
  EXPECT_EQ(1, v);
  qi::bind<void(void)>(&SetValue::exchange, &s1, 2)();
  EXPECT_EQ(2, v);
  qi::bind<void(int)>(&SetValue::exchange, boost::ref(s1), _1)(3);
  EXPECT_EQ(3, v);
  qi::bind<void(void)>(&SetValue::exchange, boost::ref(s1), 4)();
  EXPECT_EQ(4, v);
}

TEST(TestBind, SharedPtr)
{
  int v = 0;
  boost::shared_ptr<SetValue> s(new SetValue(v));
  qi::bind<void(int)>(&SetValue::exchange, s, _1)(1);
  EXPECT_EQ(1, v);
  qi::bind<void(void)>(&SetValue::exchange, s, 2)();
  EXPECT_EQ(2, v);

  boost::function<void(void)> f =  qi::bind<void(void)>(&SetValue::exchange, s, 3);
  s.reset();
  f();
  EXPECT_EQ(3, v);
}

TEST(TestBind, WeakPtr)
{
  int v = 0;
  boost::shared_ptr<SetValue> s(new SetValue(v));
  boost::weak_ptr<SetValue> w(s);
  qi::bind<void(int)>(&SetValue::exchange, w, _1)(1);
  EXPECT_EQ(1, v);
  qi::bind<void(void)>(&SetValue::exchange, w, 2)();
  EXPECT_EQ(2, v);

  boost::function<void(void)> f =  qi::bind<void(void)>(&SetValue::exchange, w, 3);
  s.reset();
  EXPECT_ANY_THROW(f());
  EXPECT_EQ(2, v);
}

class SetValue2: public SetValue, public qi::Trackable<SetValue2>
{
public:
  SetValue2(int& target)
  :SetValue(target)
  , qi::Trackable<SetValue2>(this)
  {}
  ~SetValue2()
  {
    qiLogVerbose() << "entering dtor";
    state = 1;
    destroy();
    state = 2;
    qiLogVerbose() << "finishing dtor";
  }
  void delayExchangeP(int msDelay, int value, qi::Promise<int> result)
  {
    result.setValue(delayExchange(msDelay, value));
  }
  qi::Future<int> asyncDelayExchange(int msDelay, int value)
  {
    qi::Promise<int> promise;
    boost::thread(&SetValue2::delayExchangeP, this, msDelay, value, promise);
    return promise.future();
  }
};

// wrap a call, waiting before, and notifying state
void wrap(boost::function<void()> op, int msDelay, int* notify)
{
  *notify = 1;
  if (msDelay)
    qi::os::msleep(msDelay);
  *notify = 2;
  try {
    op();
  }
  catch (const qi::PointerLockException&)
  {}
  *notify = 3;
}

TEST(TestBind, Trackable)
{
  int v = 0;
  {
    SetValue2 s1(v);
    qi::bind<void(int)>(&SetValue2::exchange, &s1, _1)(1);
    EXPECT_EQ(1, v);
    qi::bind<void(void)>(&SetValue2::exchange, &s1, 2)();
    EXPECT_EQ(2, v);
    qi::bind<void(int)>(&SetValue2::exchange, boost::ref(s1), _1)(3);
    EXPECT_EQ(3, v);
    qi::bind<void(void)>(&SetValue2::exchange, boost::ref(s1), 4)();
    EXPECT_EQ(4, v);
  }

  boost::function<void(void)> f;
  {
    SetValue2 s1(v);
    f = qi::bind<void(void)>(&SetValue2::exchange, &s1, 5);
  }
  EXPECT_ANY_THROW(f()); // s1 is trackable, bound and deleted: callback not executed
  EXPECT_EQ(4, v);

  // check waiting behavior of destroy
  int notify = 0;
  qi::int64_t time;
  {
    SetValue2 s1(v);
    boost::thread(wrap,
      qi::bind<void(void)>(&SetValue2::delayExchange, &s1, 100, 10),
      0,
      &notify);
    time = qi::os::ustime();
    // wait enough for operation to start
    while (!notify)
      qi::os::msleep(10);
    qi::os::msleep(20);
    time = qi::os::ustime();
    // exit scope, deleting s1, which should block until operation terminates
  }
  time = qi::os::ustime() - time;
  EXPECT_GT(time, 60000); // 100 - 20 - 10 - margin
  EXPECT_EQ(10, v);

  // check disable-call behavior again in our more complex threaded setup
  {
    notify = 0;
    SetValue2 s1(v);
    boost::thread(wrap,
      qi::bind<void(void)>(&SetValue2::delayExchange, &s1, 100, 11),
      50,
      &notify);
  }
  while (notify != 3)
    qi::os::msleep(10);
  EXPECT_EQ(10, v); //call not made
}

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

void set_from_future(int* tgt, qi::Future<void> f)
{
  *tgt = f.isFinished()?2:0;
}

TEST(FutureTrack, WeakPtr)
{
  int v = 0;
  boost::shared_ptr<SetValue> s(new SetValue(v));
  boost::weak_ptr<SetValue> w(s);
  delayValue(-1).connect(&SetValue::exchange, w, 1);
  for (int i=0; i<50&&v!=1; ++i)
    qi::os::msleep(10);
  EXPECT_EQ(1, v);
  v=0;
  // check that _1 works in connect
  delayValue(-1).connect(&set_from_future, &v, _1);
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
  int v =0;
  SetValue2* w = new SetValue2(v);
  delayValue(-1).connect(&SetValue2::exchange, w, 1);
  for (int i=0; i<50&&v!=1; ++i)
    qi::os::msleep(10);
  EXPECT_EQ(1, v);
  v=0;
  // check that _1 works in connect
  delayValue(-1).connect(&set_from_future, &v, _1);
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
  EXPECT_EQ(0, *gSuccess);
  boost::thread_group tg;

  tg.create_thread(boost::bind(&consumer, boost::ref(gSuccess), pro.future()));
  tg.create_thread(boost::bind(&consumer, boost::ref(gSuccess), pro.future()));
  tg.create_thread(boost::bind(&consumer, boost::ref(gSuccess), pro.future()));
  tg.create_thread(boost::bind(&producer, pro));
  tg.join_all();
  EXPECT_EQ(3, *gSuccess);
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
  qi::async(boost::function<void()>(boost::bind(&qi::Promise<int>::setValue, pro, 42)), 20000);
  EXPECT_EQ(qi::FutureState_FinishedWithValue, fut.wait(qi::SteadyClock::now() + qi::MilliSeconds(40)));
  EXPECT_TRUE(fut.isFinished());
  EXPECT_GT(qi::SteadyClock::now(), start + qi::MilliSeconds(20));
  EXPECT_LT(qi::SteadyClock::now(), start + qi::MilliSeconds(40));
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

void unlock(qi::Promise<int> prom, bool* tag)
{
  *tag = true;
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

  bool tag = false;
  {
    qi::FutureSync<int> fs;
    qi::Promise<int> p;
    fs = p.future();
    eventLoop->async(boost::bind(unlock, p, &tag), 50000);
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
      eventLoop->async(boost::bind(unlock, p, &tag), 50000);
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
      eventLoop->async(boost::bind(unlock, p, &tag), 50000);
    }
    ASSERT_FALSE(tag); // fs was copied: blocking disabled
  }
  ASSERT_TRUE(tag); // validate our synchro point worked

  ASSERT_TRUE(true);
}

qi::FutureSync<int> getSync(bool* tag)
{
  qi::EventLoop* el = qi::getEventLoop();
  qi::Promise<int> promise;
  el->async(boost::bind(unlock, promise, tag), 50000);
  return promise.future();
}

qi::FutureSync<int> getSync2(bool* tag)
{
  qi::EventLoop* el = qi::getEventLoop();
  qi::Promise<int> promise;
  el->async(boost::bind(unlock, promise, tag), 50000);
  return promise.future().sync();
}

qi::FutureSync<int> getGetSync(bool* tag)
{
  return getSync(tag);
}

qi::FutureSync<int> getGetSync2(bool* tag)
{
  return getSync2(tag);
}

TEST(TestFutureSync, InSitu)
{
  /* Check that whatever we do, a function returning a FutureSync is not
  * stuck if we take the sync, and blocks if we ignore it
  */
  bool tag = false;
  {
    qi::FutureSync<int> fs = getSync(&tag);
    ASSERT_FALSE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
  {
    qi::FutureSync<int> fs = getSync2(&tag);
    ASSERT_FALSE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
  {
    qi::FutureSync<int> fs = getGetSync(&tag);
    ASSERT_FALSE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
  {
    qi::FutureSync<int> fs = getGetSync2(&tag);
    ASSERT_FALSE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
  {
    getSync(&tag);
    ASSERT_TRUE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
  {
    getSync2(&tag);
    ASSERT_TRUE(tag);
  }
  ASSERT_TRUE(tag);
  tag = false;
}

TEST(TestFutureSync, ThrowOnDestroy) {
  qi::Promise<int> prom;

  prom.setError("touctouc");
  EXPECT_THROW(qi::FutureSync<int>(prom.future()), std::runtime_error);
  EXPECT_THROW(qi::FutureSync<int>(prom.future()), std::runtime_error); //multiple futuresync on the same future should fail.
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
  EXPECT_NO_THROW(qi::FutureSync<int>(prom.future()).isCancelable());
}

void do_nothing(TestFutureI*) {}

TEST(TestFutureError, MultipleSetValue)
{
  qi::Promise<int> p;
  qi::Future<int> f = p.future();
  p.setValue(0);
  p.reset();
  p.setError("");
  p.reset();
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


TEST(TestFutureCancel, NotCanceleable)
{
  qi::Promise<int> p;
  qi::Future<int> f = p.future();
  EXPECT_ANY_THROW({f.cancel();});
}

static void setTrue(bool* b)
{
  *b = true;
}

TEST(TestFutureCancel, AsyncCallCanceleable)
{
  bool b = false;
  qi::Future<void> f = qi::getEventLoop()->async(
    boost::bind(&setTrue, &b), 200);
  f.cancel();
  // f is going to cancel asynchronously, so it can already be cancelled, or not
  qi::os::msleep(400);
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
  qi::Promise<int> promise(qi::PromiseNoop<int>);

  ASSERT_FALSE(promise.isCancelRequested());

  qi::Future<int> future = promise.future();

  ASSERT_TRUE(future.isRunning());
  ASSERT_TRUE(future.isCancelable());

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
  ASSERT_TRUE(f.isCancelable());
  f.cancel();
  ASSERT_TRUE(f.isFinished());
  ASSERT_TRUE(f.isCanceled());
  ASSERT_TRUE(f.isCancelable());
  ASSERT_FALSE(f.hasError(qi::FutureTimeout_None));
  ASSERT_FALSE(f.hasValue(qi::FutureTimeout_None));


  p = qi::Promise<int>(&doError);
  f = p.future();

  ASSERT_FALSE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  ASSERT_TRUE(f.isCancelable());
  f.cancel();
  ASSERT_TRUE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  ASSERT_TRUE(f.isCancelable());
  ASSERT_TRUE(f.hasError(qi::FutureTimeout_None));
  ASSERT_FALSE(f.hasValue(qi::FutureTimeout_None));

  p = qi::Promise<int>(&doValue);
  f = p.future();

  ASSERT_FALSE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  ASSERT_TRUE(f.isCancelable());
  f.cancel();
  ASSERT_TRUE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  ASSERT_TRUE(f.isCancelable());
  ASSERT_FALSE(f.hasError(qi::FutureTimeout_None));
  ASSERT_TRUE(f.hasValue(qi::FutureTimeout_None));

  p = qi::Promise<int>(&doNothing);
  f = p.future();

  ASSERT_FALSE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  ASSERT_TRUE(f.isCancelable());
  f.cancel();
  ASSERT_FALSE(f.isFinished());
  ASSERT_FALSE(f.isCanceled());
  ASSERT_TRUE(f.isCancelable());
  EXPECT_THROW(f.hasError(qi::FutureTimeout_None), qi::FutureException);
  EXPECT_THROW(f.hasValue(qi::FutureTimeout_None), qi::FutureException);
}

TEST(TestFutureCancel, Canceled)
{
  bool b = false;
  qi::Future<void> f = qi::getEventLoop()->async(
    boost::bind(&setTrue, &b), 200000);
  f.cancel();
  // Depending on multi-thread timing future can be finished or not at this point
  qi::os::msleep(400);
  ASSERT_TRUE(!b);
  ASSERT_TRUE(f.isFinished());
  ASSERT_FALSE(f.hasError());
  ASSERT_TRUE(f.isCanceled());
}

void setValueWait(boost::mutex& mutex, int waittime, int& i, int v)
{
  boost::unique_lock<boost::mutex> lock(mutex);
  // we should never be called in parallel
  ASSERT_TRUE(lock.owns_lock());
  if (waittime)
    qi::os::msleep(waittime);
  i = v;
}

TEST(TestStrand, StrandSimple)
{
  boost::mutex mutex;

  qi::Strand strand(*qi::getEventLoop());
  int i = 0;
  qi::Future<void> f1 = strand.async(boost::bind<void>(&setValueWait,
        boost::ref(mutex), 300, boost::ref(i), 1));
  qi::os::msleep(5);
  EXPECT_FALSE(f1.isFinished());
  qi::Future<void> f2 = strand.async(boost::bind<void>(&setValueWait,
        boost::ref(mutex), 0, boost::ref(i), 2));
  qi::os::msleep(500);
  EXPECT_EQ(i, 2);
}

static void fail()
{
  ASSERT_TRUE(false);
}

TEST(TestStrand, StrandCancel)
{
  qi::Strand strand(*qi::getEventLoop());
  // cancel before scheduling
  qi::Future<void> f1 = strand.async(fail, qi::MilliSeconds(100));
  f1.cancel();
  ASSERT_EQ(qi::FutureState_Canceled, f1.wait());
}

static void increment(boost::mutex& mutex, int waittime, int& i)
{
  boost::unique_lock<boost::mutex> lock(mutex);
  // we should never be called in parallel
  ASSERT_TRUE(lock.owns_lock());
  if (waittime)
    qi::os::msleep(waittime);
  ++i;
}

static const int STRAND_NB_TRIES = 100;

TEST(TestStrand, AggressiveCancel)
{
  boost::mutex mutex;
  std::vector<qi::Future<void> > futures;

  qi::Strand strand(*qi::getEventLoop());
  int i = 0;
  for (int j = 0; j < STRAND_NB_TRIES; ++j)
  {
    qi::Future<void> f1 = strand.async(boost::bind<void>(&increment,
          boost::ref(mutex), 1, boost::ref(i)));
    futures.push_back(f1);
  }
  BOOST_FOREACH(qi::Future<void>& future, futures)
    future.cancel();

  int successCount = 0;
  BOOST_FOREACH(qi::Future<void>& future, futures)
  {
    if (future.wait() != qi::FutureState_Canceled)
      successCount++;
  }
  if (i == STRAND_NB_TRIES || i == 0)
    qiLogWarning() << "i = " << i <<
      ", this test is meaningless, you should do something about that";
  ASSERT_EQ(successCount, i);
}

TEST(TestStrand, StrandDestruction)
{
  boost::mutex mutex;
  int i = 0;

  {
    qi::Strand strand(*qi::getEventLoop());
    for (int j = 0; j < STRAND_NB_TRIES; ++j)
    {
      qi::Future<void> f1 = strand.async(boost::bind<void>(&increment,
            boost::ref(mutex), 1, boost::ref(i)));
    }
  }

  ASSERT_EQ(STRAND_NB_TRIES, i);
}

TEST(TestStrand, StrandDestructionWithCancel)
{
  boost::mutex mutex;
  std::vector<qi::Future<void> > futures;
  int i = 0;

  {
    qi::Strand strand(*qi::getEventLoop());
    for (int j = 0; j < STRAND_NB_TRIES; ++j)
    {
      qi::Future<void> f1 = strand.async(boost::bind<void>(&increment,
            boost::ref(mutex), 1, boost::ref(i)));
      futures.push_back(f1);
    }
    BOOST_FOREACH(qi::Future<void>& future, futures)
      future.cancel();
  }

  int successCount = 0;
  BOOST_FOREACH(qi::Future<void>& future, futures)
  {
    if (future.wait() != qi::FutureState_Canceled)
      successCount++;
  }
  if (i == STRAND_NB_TRIES || i == 0)
    qiLogWarning() << "i = " << i <<
      ", this test is meaningless, you should do something about that";
  ASSERT_EQ(successCount, i);
}

static void deleteStrand(qi::Strand* strand)
{
  delete strand;
}

TEST(TestStrand, StrandDestructionBeforeEnd)
{
  qi::Strand* strand = new qi::Strand(*qi::getEventLoop());
  qi::Future<void> f = strand->async(boost::bind(deleteStrand, strand));
  f.value();
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

TEST(TestPromiseBarrier, SimpleBarrier)
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
  ASSERT_EQ(it, *a);
}

TEST(TestPromiseBarrier, ClosedBarrier)
{
  qi::FutureBarrier<void> barrier;

  // Can add a future to the barrier because is not yet closed.
  qi::Promise<void> prom;
  ASSERT_TRUE(barrier.addFuture(prom.future()));
  prom.setValue(0);

  barrier.future().wait();

  // Invalid promise, because FutureBarrier is closed.
  qi::Promise<void> prom2;
  ASSERT_FALSE(barrier.addFuture(prom2.future()));
}

TEST(TestPromiseBarrier, CompleteExample)
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
  qi::Promise<void> end;
  barrier.future().connect(boost::bind(&checkBarrier, end, _1));

  // Wait for the end of the check.
  end.future().wait();
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
  ASSERT_TRUE(prom2.future().isCancelable());
  prom2.future().cancel();
  while(prom2.future().isRunning())
    qi::os::msleep(5);
  ASSERT_TRUE(prom2.future().isCanceled());
  ASSERT_FALSE(prom2.future().hasValue());
  ASSERT_FALSE(prom2.future().hasError());
}

int ping(int v)
{
  if (v>= 0)
    return v;
  else
    throw std::runtime_error("Invalid argument ");
}

TEST(EventLoop, async)
{
  qi::EventLoop* el = qi::getEventLoop();
  qi::Future<int> f = el->async<int>(boost::bind(ping, 42), 200000);
  EXPECT_FALSE(f.isFinished());
  f.wait();
  EXPECT_FALSE(f.hasError());
  EXPECT_EQ(f.value(), 42);

  f = el->async<int>(boost::bind(ping, 42), 200000);
  EXPECT_FALSE(f.isFinished());
  EXPECT_NO_THROW(f.cancel());
  EXPECT_EQ(f.wait(), qi::FutureState_Canceled);

  f = el->async<int>(boost::bind(ping, -1), 200000);
  EXPECT_FALSE(f.isFinished());
  f.wait();
  EXPECT_TRUE(f.hasError());

  f = el->async<int>(boost::bind(ping, 42), qi::MilliSeconds(20));
  qi::os::msleep(25);
  EXPECT_TRUE(f.isFinished());

  f = el->async<int>(boost::bind(ping, 42), qi::SteadyClock::now() + qi::MilliSeconds(10));
  qi::os::msleep(15);
  EXPECT_TRUE(f.isFinished());
}

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
  pt.setCallback(&inc, boost::ref(a));
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
    pt.setCallback(&inc, boost::ref(a));
    pt.setUsPeriod(0);
    pt.start();
    qi::os::msleep(i%20);
    pt.stop();
  }
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
  pt.setCallback(&inc, boost::ref(a));
  pt.setUsPeriod(1000);
  pt.start();
  std::vector<qi::Future<void> > futures;
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, boost::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, boost::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, boost::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, boost::ref(pt))));
  for (unsigned int i = 0; i < futures.size(); ++i)
    futures[i].wait();
  pt.stop();
}

TEST(TestPeriodicTask, TriggerStartStop)
{
  // just test that there is no segfault or deadlock
  qi::PeriodicTask pt;
  qi::Atomic<int> a;
  pt.setCallback(&inc, boost::ref(a));
  pt.setUsPeriod(1000);
  std::vector<qi::Future<void> > futures;
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, boost::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, boost::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, boost::ref(pt))));
  futures.push_back(qi::getEventLoop()->async(boost::bind(&loopTrigger, boost::ref(pt))));
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

int get42() { return 42; }

TEST(EventLoop, asyncFast)
{
  qi::EventLoop* el = qi::getEventLoop();
  for (int i = 0; i < 10; ++i)
  {
    qi::Future<int> f = el->async<int>(get42);
    f.wait();
  }
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif
