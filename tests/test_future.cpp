/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
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

  qi::Promise<int> pro;

  qi::Future<int>  fut = pro.future();

  fut.connect(boost::bind(&TestFutureI::onFutureFinished, tf, _1));

  EXPECT_EQ(0, gGlobalI);
  EXPECT_FALSE(fut.isReady());
  pro.setValue(42);
  fut.wait(1000);
  EXPECT_TRUE(fut.isReady());
  EXPECT_EQ(42, fut.value());
  EXPECT_EQ(42, gGlobalI);
}

TEST_F(TestFuture, ComplexType) {
  TestFutureS tf(gGlobalS, gGlobalE);

  qi::Promise<std::string> pro;

  qi::Future<std::string>  fut = pro.future();

  fut.connect(boost::bind(&TestFutureS::onFutureFinished, tf, _1));

  EXPECT_STREQ("", gGlobalS.c_str());
  EXPECT_FALSE(fut.isReady());
  pro.setValue("42");
  EXPECT_TRUE(fut.isReady());
  EXPECT_STREQ("42", fut.value().c_str());
  EXPECT_STREQ("42", gGlobalS.c_str());
}

void producer(qi::Promise<int> pro) {
  qi::os::msleep(100);
  pro.setValue(42);
}

void consumer(qi::Atomic<int> &gSuccess, qi::Future<int> fut) {
  //wont block thread on error
  ASSERT_TRUE(fut.wait(1000));
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

  EXPECT_FALSE(fut.wait(100));
  EXPECT_FALSE(fut.isReady());
}

TEST_F(TestFuture, TestError) {
  TestFutureI tf(gGlobalI, gGlobalE);

  qi::Promise<int> pro;

  qi::Future<int>  fut = pro.future();
  fut.connect(boost::bind(&TestFutureI::onFutureFinished, tf, _1));

  EXPECT_STREQ("", gGlobalE.c_str());
  EXPECT_FALSE(fut.isReady());
  pro.setError("chiche");
  fut.wait();
  EXPECT_STREQ("chiche", gGlobalE.c_str());
  EXPECT_TRUE(fut.isReady());
  EXPECT_TRUE(fut.hasError());
}

void unlock(qi::Promise<int> prom, bool* tag)
{
  *tag = true;
  prom.setValue(1);
}

TEST(TestFutureSync, Basic)
{
  qi::EventLoop* eventLoop = qi::getDefaultNetworkEventLoop();
  ASSERT_FALSE(eventLoop->isInEventLoopThread());

  {
    qi::FutureSync<int> fs;
    ASSERT_TRUE(!fs.isReady());
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
  qi::EventLoop* el = qi::getDefaultObjectEventLoop();
  qi::Promise<int> promise;
  el->async(boost::bind(unlock, promise, tag), 50000);
  return promise.future();
}

qi::FutureSync<int> getSync2(bool* tag)
{
  qi::EventLoop* el = qi::getDefaultObjectEventLoop();
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

void do_nothing(TestFutureI*) {}

TEST(TestFutureTrack, ConnectTrack1)
{ // Check it triggers
  qi::Promise<int> p;
  qi::Future<int> f = p.future();
  int res = 0;
  std::string err;
  TestFutureI test(res, err);

  boost::shared_ptr<TestFutureI> ptr(&test, do_nothing);
  f.connect(qi::Future<int>::Slot(boost::bind(&TestFutureI::onFutureFinished, ptr.get(), _1)).track(ptr));
  p.setValue(1);
  ASSERT_EQ(1, res);
  ptr.reset();
}

TEST(TestFutureTrack, ConnectTrack2)
{ // Check it triggers when promise already set
  qi::Promise<int> p;
  qi::Future<int> f = p.future();
  int res = 0;
  std::string err;
  TestFutureI test(res, err);

  boost::shared_ptr<TestFutureI> ptr(&test, do_nothing);
  p.setValue(1);
  f.connect(qi::Future<int>::Slot(boost::bind(&TestFutureI::onFutureFinished, ptr.get(), _1)).track(ptr));
  ASSERT_EQ(1, res);
  ptr.reset();
}

TEST(TestFutureTrack, ConnectTrack3)
{ // Check it does not trigger when shared_ptr goes down
  qi::Promise<int> p;
  qi::Future<int> f = p.future();
  int res = 0;
  std::string err;
  TestFutureI test(res, err);

  boost::shared_ptr<TestFutureI> ptr(&test, do_nothing);

  f.connect(qi::Future<int>::Slot(boost::bind(&TestFutureI::onFutureFinished, ptr.get(), _1)).track(ptr));
  ptr.reset();
  p.setValue(1);
  ASSERT_EQ(0, res);
}

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

TEST(TestFutureCancel, Canceleable)
{
  bool b = false;
  qi::Future<void> f = qi::getDefaultNetworkEventLoop()->async(
    boost::bind(&setTrue, &b), 200);
  f.cancel();
  ASSERT_FALSE(f.isReady());
  qi::os::msleep(400);
  ASSERT_TRUE(!b);
  ASSERT_TRUE(f.isReady());
  ASSERT_TRUE(f.hasError());
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

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
