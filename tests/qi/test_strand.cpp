/*
**  Copyright (C) 2018 Softbank Robotics Europe
**  See COPYING for the license
*/

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <thread>
#include <random>
#include <boost/thread/mutex.hpp>

#include <ka/errorhandling.hpp>
#include <ka/macro.hpp>
#include <qi/application.hpp>
#include <qi/future.hpp>
#include <qi/os.hpp>
#include <qi/strand.hpp>
#include <qi/periodictask.hpp>
#include <qi/actor.hpp>
#include <qi/log.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/testutils/testutils.hpp>
#include <qi/eventloop.hpp>

#include "test_qilog.hpp"

qiLogCategory("test");

const qi::MilliSeconds usualTimeout{ 200 };
const std::chrono::milliseconds stdUsualTimeout{usualTimeout.count()};

void setValueWait(boost::mutex& mutex, std::chrono::milliseconds waittime, int& i, int v)
{
  boost::unique_lock<boost::mutex> lock(mutex, boost::try_to_lock);
  // we should never be called in parallel
  ASSERT_TRUE(lock.owns_lock());
  if (waittime != std::chrono::milliseconds::zero())
    std::this_thread::sleep_for(waittime);
  i = v;
}

TEST(TestStrand, StrandSimple)
{
  boost::mutex mutex;

  qi::Strand strand{*qi::getEventLoop()};
  int i = 0;
  qi::Future<void> f1 = strand.async(boost::bind<void>(&setValueWait,
        boost::ref(mutex), std::chrono::milliseconds{ 100 }, boost::ref(i), 1));
  std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
  EXPECT_FALSE(f1.isFinished());
  qi::Future<void> f2 = strand.async(boost::bind<void>(&setValueWait,
        boost::ref(mutex), std::chrono::milliseconds{ 0 }, boost::ref(i), 2));
  std::this_thread::sleep_for(stdUsualTimeout);
  EXPECT_EQ(i, 2);
}


TEST(TestStrand, StrandSchedulerForRunsAndEnds)
{
  qi::Strand strand;
  std::promise<void> promise;
  auto stranded = strand.schedulerFor([&]{ promise.set_value(); });
  stranded();
  auto status = promise.get_future().wait_for(stdUsualTimeout);
  EXPECT_EQ(std::future_status::ready, status);
}


TEST(TestStrand, StrandSchedulerForWithBoundReference)
{
  qi::Strand strand;
  std::promise<int> promise;
  int expected = 42;
  auto function = [&](int& n){ promise.set_value(n); };
  auto bound = std::bind(std::move(function), std::ref(expected));
  auto stranded = strand.schedulerFor(std::move(bound));
  expected = 13;
  stranded();
  auto future = promise.get_future();
  auto status = future.wait_for(stdUsualTimeout);
  EXPECT_EQ(std::future_status::ready, status);
  EXPECT_EQ(expected, future.get());
}


TEST(TestStrand, StrandSchedulerForWithMoveOnlyBoundReference)
{
  qi::Strand strand;
  std::promise<int> promise;
  std::unique_ptr<int> expected{new int{42}};
  auto function = [&](std::unique_ptr<int>& n){ promise.set_value(*n); };
  auto bound = std::bind(std::move(function), std::ref(expected));
  auto stranded = strand.schedulerFor(std::move(bound));
  *expected = 13;
  stranded();
  auto future = promise.get_future();
  auto status = future.wait_for(stdUsualTimeout);
  EXPECT_EQ(std::future_status::ready, status);
  EXPECT_EQ(*expected, future.get());
}


TEST(TestStrand, StrandSchedulerForWithMoveOnlyArg)
{
  qi::Strand strand;
  std::promise<int> promise;
  std::unique_ptr<int> expected{new int{42}};
  auto function = [&](std::unique_ptr<int>& n){ promise.set_value(*n); };
  auto stranded = strand.schedulerFor(std::move(function));
  *expected = 13;
  stranded(std::ref(expected)); // stranded(expected) must not compile
  auto future = promise.get_future();
  auto status = future.wait_for(stdUsualTimeout);
  EXPECT_EQ(std::future_status::ready, status);
  EXPECT_EQ(*expected, future.get());
}

TEST(TestStrand, StrandCancel)
{
  qi::Strand strand{*qi::getEventLoop()};
  // cancel before scheduling
  auto f = strand.asyncDelay([]{}, qi::Seconds(1000));
  f.cancel();
  ASSERT_EQ(qi::FutureState_Canceled, f.wait());
}

TEST(TestStrand, StrandLogOnUncaughtExceptionInPostedTask)
{
  MockLogHandler mockLogHandler("strand_post_error");

  qi::Strand strand;

  EXPECT_CALL(mockLogHandler, log(qi::LogLevel::LogLevel_Warning, testing::_, testing::_));
  strand.post([=]{ throw "lol"; });
  strand.async([]{}).wait(); //Make sure to wait for the throwing task to be completely processed.
  qi::log::flush();
}

TEST(TestStrand, AsyncDelayThenDestroyIsSafe)
{
  auto future = []
  {
    qi::Strand strand;
    return strand.asyncDelay([]{}, usualTimeout);
  }();

  ASSERT_TRUE(future.isValid());
  const auto status = future.wait();
  ASSERT_EQ(qi::FutureState_FinishedWithError, status);
}

TEST(TestStrand, StrandCancelScheduled)
{
  qi::Strand strand;
  // cancel before scheduling
  qi::Promise<void> syncProm;
  auto syncFut = syncProm.future();
  strand.async([=]{ syncFut.wait(); }); // lock up the strand
  auto scheduledTaskFut = strand.async([]{}); // no delay should schedule the callback right away.
  scheduledTaskFut.cancel();
  syncProm.setValue(nullptr);
  ASSERT_EQ(qi::FutureState_Canceled, scheduledTaskFut.wait());
}

static void increment(boost::mutex& mutex, std::chrono::milliseconds waittime, std::atomic<unsigned int>& i)
{
  boost::unique_lock<boost::mutex> lock(mutex, boost::try_to_lock);
  // we should never be called in parallel
  ASSERT_TRUE(lock.owns_lock());
  if (waittime != std::chrono::milliseconds::zero())
    std::this_thread::sleep_for(waittime);
  ++i;
}

static const unsigned int STRAND_NB_TRIES = 100;

TEST(TestStrand, AggressiveCancel)
{
  boost::mutex mutex;
  std::vector<qi::Future<void>> futures;

  qi::Strand strand;
  std::atomic<unsigned int> i(0);
  for (unsigned int j = 0; j < STRAND_NB_TRIES; ++j)
  {
    qi::Future<void> f1 = strand.async(boost::bind<void>(&increment,
          boost::ref(mutex), std::chrono::milliseconds{1}, boost::ref(i)));
    futures.push_back(f1);
    f1 = strand.async(boost::bind<void>(&increment,
          boost::ref(mutex), std::chrono::milliseconds{50}, boost::ref(i)));
    futures.push_back(f1);
  }
  for(auto&& future: futures)
    future.cancel();

  unsigned int successCount = 0;
  for(auto&& future: futures)
  {
    if (future.wait() != qi::FutureState_Canceled)
      successCount++;
  }
  if (i == futures.size() || i == 0)
    qiLogWarning() << "i = " << i <<
      ", this test is meaningless, you should do something about that";
  ASSERT_EQ(successCount, i);
}

TEST(TestStrand, StrandDestruction)
{
  boost::mutex mutex;
  std::atomic<unsigned int> i(0);

  std::vector<qi::Future<void>> futures;
  {
    qi::Strand strand;
    futures.reserve(STRAND_NB_TRIES);
    for (unsigned int j = 0; j < STRAND_NB_TRIES; ++j)
    {
      futures.push_back(strand.async([&]{
        increment(mutex, std::chrono::milliseconds{1}, i);
      }));
    }
  }
  for (auto& future : futures)
  {
    ASSERT_TRUE(future.isFinished());
  }
}

TEST(TestStrand, StrandDestructionWithMethodAndConcurrency)
{
  // ASSERT_NOSEGFAULT_NOCRASH_NOBADTHINGS();
  boost::mutex mutex;
  std::atomic<unsigned int> i(0);

  std::vector<qi::Future<void>> futures;
  qi::Strand strand;
  futures.reserve(STRAND_NB_TRIES);
  for (unsigned int j = 0; j < STRAND_NB_TRIES/4; ++j)
    futures.push_back(qi::getEventLoop()->async([&]{
            strand.async(boost::bind<void>(&increment,
                boost::ref(mutex), std::chrono::milliseconds{1}, boost::ref(i)));
          }));
  for (unsigned int j = 0; j < STRAND_NB_TRIES/4; ++j)
    strand.async(boost::bind<void>(&increment,
            boost::ref(mutex), std::chrono::milliseconds{1}, boost::ref(i)));
  strand.join();
  for (unsigned int j = 0; j < STRAND_NB_TRIES/4; ++j)
    futures.push_back(qi::getEventLoop()->async([&]{
            strand.async(boost::bind<void>(&increment,
                boost::ref(mutex), std::chrono::milliseconds{1}, boost::ref(i)));
          }));
  for (unsigned int j = 0; j < STRAND_NB_TRIES/4; ++j)
    strand.async(boost::bind<void>(&increment,
            boost::ref(mutex), std::chrono::milliseconds{1}, boost::ref(i)));
  for (auto& future : futures)
    ASSERT_TRUE(future.wait());
}

TEST(TestStrand, StrandDestructionWithCancel)
{
  boost::mutex mutex;
  std::vector<qi::Future<void> > futures;
  std::atomic<unsigned int> i(0);

  {
    qi::Strand strand;
    for (unsigned int j = 0; j < STRAND_NB_TRIES; ++j)
    {
      qi::Future<void> f1 = strand.async(boost::bind<void>(&increment,
            boost::ref(mutex), std::chrono::milliseconds{1}, boost::ref(i)));
      futures.push_back(f1);
      f1 = strand.async(boost::bind<void>(&increment,
            boost::ref(mutex), std::chrono::milliseconds{50}, boost::ref(i)));
      futures.push_back(f1);
    }
    for(qi::Future<void>& future: futures)
      future.cancel();
  }

  unsigned int successCount = 0;
  for(qi::Future<void>& future: futures)
  {
    if (future.wait() != qi::FutureState_Canceled)
      successCount++;
  }
  if (i == futures.size() || i == 0)
    qiLogWarning() << "i = " << i <<
      ", this test is meaningless, you should do something about that";
  ASSERT_EQ(successCount, i);
}


TEST(TestStrand, StrandDestructionBeforeEnd)
{
  qi::Strand* strand = new qi::Strand(*qi::getEventLoop());
  qi::Future<void> f = strand->async([=]{ delete strand; });
  f.wait();
}

TEST(TestStrand, StrandDestructionWithSchedulerFor)
{
  std::vector<qi::Future<void>> futures;
  qi::Future<void> fut;
  auto f = []{
    // allocate on heap to help asan & co
    std::unique_ptr<qi::Strand> strand(new qi::Strand(*qi::getEventLoop()));
    return strand->schedulerFor(boost::function<void()>([]{ ADD_FAILURE(); }));
  }();

  fut = qi::async([f, &futures]{
        for (int i = 0; i < 300; ++i)
          futures.push_back(f());
      });
  fut.wait();
  for (auto& future : futures)
    future.wait();
}

struct MyActor : qi::Actor
{
  std::atomic<int> callcount { 0 };
  std::atomic<bool> calling { false };
  ~MyActor() { joinTasks(); }
  int f(int end, qi::Promise<void> finished)
  {
    int startval = prop.get().value();
    EXPECT_FALSE(calling);
    calling = true;
    std::this_thread::sleep_for(std::chrono::milliseconds{5});
    EXPECT_TRUE(calling);
    calling = false;
    EXPECT_EQ(startval, prop.get().value());
    if (++callcount == end + 1)
      finished.setValue(nullptr);
    return 42;
  }
  qi::Future<int> val()
  {
    return qi::Future<int>(42);
  }
  qi::Future<int> thrw()
  {
    throw std::runtime_error("throw");
  }
  qi::Future<int> fail()
  {
    return qi::makeFutureError<int>("fail");
  }
  qi::Signal<int> sig;
  qi::Property<int> prop;
};
QI_REGISTER_OBJECT(MyActor, f, val, thrw, fail, sig, prop);

TEST(TestStrand, TypeErasedCall)
{
  boost::shared_ptr<MyActor> obj(new MyActor);
  qi::AnyObject aobj(obj);

  EXPECT_EQ(42, aobj.async<int>("val").value());
  EXPECT_TRUE(aobj.async<int>("thrw").hasError());
  EXPECT_TRUE(aobj.async<int>("fail").hasError());

  EXPECT_EQ(42, aobj.call<int>("val"));
  EXPECT_ANY_THROW(aobj.call<int>("thrw"));
  EXPECT_ANY_THROW(aobj.call<int>("fail"));
}

TEST(TestStrand, AllFutureSignalPropertyPeriodicTaskAsyncTypeErasedDynamic)
{
  static const int TOTAL = 50;
  std::random_device rd;
  std::seed_seq seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
  std::default_random_engine randEngine(seq);
  std::uniform_int_distribution<int> distrib;

  int callcount = 0;
  {
    MyActor obj;

    qi::DynamicObjectBuilder builder;
    builder.setThreadingModel(qi::ObjectThreadingModel_SingleThread);
    builder.advertiseMethod("f",
        boost::function<void(int, qi::Promise<void>)>(boost::bind(&MyActor::f, &obj, _1, _2)));
    builder.advertiseSignal("sig", &obj.sig);
    builder.advertiseProperty("prop", &obj.prop);

    qi::AnyObject aobj(builder.object());

    qi::Promise<void> finished;

    for (int i = 0; i < 25; ++i)
      aobj.async<void>("f", TOTAL, finished);
    for (int i = 0; i < 50; ++i)
      aobj.setProperty("prop", distrib(randEngine));
    QI_EMIT obj.sig(TOTAL);
    // we need one more call (the second test expects a periodic task to run at
    // least once)
    for (int i = 0; i < 26; ++i)
      aobj.async<void>("f", TOTAL, finished);
    finished.future().wait();
    callcount = obj.callcount;
  }
  ASSERT_EQ(TOTAL + 1, callcount);
}

void chaincall(qi::AnyObject aobj, qi::Promise<void> finished, int TOTAL)
{
  for (int i = 0; i < 50; ++i)
    EXPECT_EQ(42, aobj.call<int>("f", TOTAL, finished));
}

TEST(TestStrand, AllFutureSignalPropertyPeriodicTaskAsyncCallTypeErased)
{
  static const int TOTAL = 300;
  std::random_device rd;
  std::seed_seq seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
  std::default_random_engine randEngine(seq);
  std::uniform_int_distribution<int> distrib;

  int callcount = 0;
  {
    boost::shared_ptr<MyActor> obj(new MyActor);
    qi::AnyObject aobj(obj);

    qi::Promise<void> finished;

    qi::PeriodicTask per;
    per.setUsPeriod(30);
    per.setCallback(&MyActor::f, obj.get(), TOTAL, finished);

    qi::Promise<void> prom;
    qi::Signal<int> signal;

    static_assert(std::is_same<decltype(prom.future().andThen(qi::bind(&MyActor::f, obj.get(), TOTAL, finished)).unwrap()), qi::Future<int>>::value, "andThen future type incorrect");
    static_assert(std::is_same<decltype(prom.future().then(qi::bind(&MyActor::f, obj.get(), TOTAL, finished)).unwrap()), qi::Future<int>>::value, "then future type incorrect");

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
    for (int i = 0; i < 25; ++i)
      prom.future().connect(&MyActor::f, obj.get(), TOTAL, finished);
KA_WARNING_POP()
    for (int i = 0; i < 10; ++i)
      prom.future().andThen(qi::bind(&MyActor::f, obj.get(), TOTAL, finished));
    for (int i = 0; i < 5; ++i)
      prom.future().then(qi::bind(&MyActor::f, obj.get(), TOTAL, finished));
    for (int i = 0; i < 50; ++i)
      signal.connect(&MyActor::f, obj.get(), _1, finished);
    for (int i = 0; i < 50; ++i)
      aobj.connect("sig", boost::function<void(int)>(obj->stranded(boost::bind(&MyActor::f, obj.get(), _1, finished))));

    per.start();
    for (int i = 0; i < 25; ++i)
      aobj.async<void>("f", TOTAL, finished);
    for (int i = 0; i < 25; ++i)
      qi::async(qi::bind(&MyActor::f, obj, TOTAL, finished));
    for (int i = 0; i < 50; ++i)
      aobj.setProperty("prop", distrib(randEngine));
    qi::Future<void> f = qi::async(boost::bind(chaincall, aobj, finished, TOTAL));
    prom.setValue(0);
    QI_EMIT signal(TOTAL);
    QI_EMIT obj->sig(TOTAL);
    for (int i = 0; i < 25; ++i)
      aobj.async<void>("f", TOTAL, finished);
    for (int i = 0; i < 25; ++i)
      qi::async(qi::bind(&MyActor::f, obj, TOTAL, finished));
    f.wait();
    finished.future().wait();
    callcount = obj->callcount;
  }
  ASSERT_LT(TOTAL, callcount);
}

TEST(TestStrand, FutureThenActorCancel)
{
  {
    boost::shared_ptr<MyActor> obj(new MyActor);

    qi::Promise<void> finished;

    qi::Promise<int> prom;
    // FIXME: This is probably not what the original code meant to test, but it was naive and
    // was relying on an implicit wait. This implementation is equivalent but it does not test much
    // (as did the old test). The reason we didn't reimplement it like it was meant is that it would
    // be racy with the current design of futures, actors and strands.
    qi::Future<int> masterFut = prom.future().then(
      [&](qi::Future<int> fut) { return qi::bind(&MyActor::f, obj, fut.value(), finished)().value(); });
    masterFut.cancel();
    ASSERT_TRUE(prom.isCancelRequested());
    prom.setValue(0);
    ASSERT_TRUE(test::finishesWithValue(masterFut));
    ASSERT_EQ(42, masterFut.value());
    ASSERT_NO_THROW(finished.future().value());
  }
}

template<typename Functor>
struct CallerOnDestruction
{
  CallerOnDestruction(Functor functor)
    : toCall(std::move(functor))
  {
  }

  ~CallerOnDestruction()
  {
    toCall();
  }

  Functor toCall;
};

template<typename Functor>
void callLongCallbackWithDestructionHook(qi::Strand& strand, Functor&& toCallOnDestruction)
{
  auto callerOnDestruction =
      std::make_shared<CallerOnDestruction<Functor>>(std::forward<Functor>(toCallOnDestruction));
  // keeps alive the caller for some time, so that we can join and have it die in our hands
  auto f = [callerOnDestruction]{ std::this_thread::sleep_for(stdUsualTimeout); };
  callerOnDestruction.reset();
  strand.async(std::move(f));
}

TEST(TestStrand, AsyncWhileJoiningDoesNotDeadlock)
{
  qi::Strand strand;
  std::atomic<bool> called;
  called = false;
  callLongCallbackWithDestructionHook(strand, [&]{
    strand.async([]{});
    called = true;
  });
  strand.join();
  ASSERT_TRUE(called.load());
}

TEST(TestStrand, CallWrappedInStrandWhileJoiningDoesNotDeadlock)
{
  qi::Strand strand;
  callLongCallbackWithDestructionHook(strand, strand.schedulerFor([]{}));
  strand.join();
}

TEST(TestStrand, CallStrandAsyncFromStrandContextExecutesImmediately)
{
  qi::Strand strand;
  auto f = strand.async([&]{
    const auto parentTid = std::this_thread::get_id();
    auto f2 = strand.async([=]{ return std::this_thread::get_id() == parentTid; });
    f2.wait(usualTimeout);
    return f2.value(0);
  });
  ASSERT_TRUE(test::finishesWithValue(f));
  ASSERT_TRUE(f.value());
}

TEST(TestStrand, CallScheduleFromStrandContextDoesNotExecuteImmediately)
{
  qi::Strand strand;
  std::vector<int> values;
  strand.async([&]{
    auto f = strand.defer([&]{ values.push_back(1); });
    values.push_back(0);
    return f;
  }).unwrap().value();

  const std::vector<int> expected{0, 1};
  ASSERT_EQ(expected, values);
}

TEST(TestStrand, build_schedulerFor)
{
  qi::Strand strand;

  // rvalue
  auto f1 = strand.schedulerFor([](int){});
  (void) f1;

  // lvalue
  auto func = [](int){};
  auto f2 = strand.schedulerFor(func);
  (void) f2;
}

TEST(TestStrand, build_unwrappedSchedulerFor)
{
  qi::Strand strand;

  // rvalue
  auto f1 = strand.unwrappedSchedulerFor([](int){});
  (void) f1;

  // lvalue
  auto func = [](int){};
  auto f2 = strand.unwrappedSchedulerFor(func);
  (void) f2;

  // rvalue not convertible to function pointer
  auto f3 = strand.unwrappedSchedulerFor([](int) {});
  (void)f3;

  // lvalue not convertible to function pointer
  auto bigfunc = [](int) {};
  auto f4 = strand.unwrappedSchedulerFor(bigfunc);
  (void)f4;
}

TEST(TestStrand, LastStrandedThenContinuationCalledWhenCanceled)
{
  // This test was written because of a bug making one or several continuations
  // following a cancel (sequence point 5 to 7) randomly never be executed
  // while the whole graph is correctly finished.
  // This issue required the following context:
  // 1. Execute all the continuations in the qi::Strand qi::Strand::schedulerFor/unwrappedSchedulerFor
  //    continuation in the task graph, in particular the last .then continuation;
  // 2. The future corresponding to the task graph has to request cancel
  //    before the 2 last continuation is executed;
  // 3. The promise has to set itself to canceled when it gets a cancel request
  //    (the problem does not appear if the promise was set outside the cancel callback);

  qi::Strand strand;
  std::vector<int> executionSequence;
  static const std::vector<int> expectedSequence {0, 1, 2, 3, 4, 5, 6, 7};
  std::atomic<bool> canceledAsExpected {false};

  qi::Promise<void> promise {[&] (qi::Promise<void>& p) {
    executionSequence.push_back(3);
    p.setCanceled(); // Requirement 3
  }};

  qi::Promise<void> promiseSync1, promiseSync2;

  qi::Future<void> future = strand.defer([&] {executionSequence.push_back(0);})
    .then(strand.unwrappedSchedulerFor([&] (qi::Future<void>) {
       executionSequence.push_back(1);
       auto fut = promise.future().then(strand.unwrappedSchedulerFor( // Requirement 1
        [&] (qi::Future<void> result) {
          if(result.isCanceled())
            canceledAsExpected = true;
          executionSequence.push_back(4);
        })).unwrap()
        .then(strand.unwrappedSchedulerFor([&] (qi::Future<void>) {
          executionSequence.push_back(5);
        }, {}, qi::ExecutionOptions{ qi::CancelOption::NeverSkipExecution} )).unwrap()
        .then(strand.unwrappedSchedulerFor([&] (qi::Future<void>) {
          executionSequence.push_back(6);
        }, {}, qi::ExecutionOptions{ qi::CancelOption::NeverSkipExecution })).unwrap()
        .then(strand.unwrappedSchedulerFor([&] (qi::Future<void>) {
          executionSequence.push_back(7);
        }, {}, qi::ExecutionOptions{ qi::CancelOption::NeverSkipExecution })).unwrap();
       executionSequence.push_back(2);
       promiseSync1.setValue(nullptr);// Requirement 2
       promiseSync2.future().wait();// Requirement 2
       return fut;
     })).unwrap();

  promiseSync1.future().wait(); // Requirement 2
  future.cancel(); // Requirement 2
  promiseSync2.setValue(nullptr); // Requirement 2
  future.wait();
  EXPECT_EQ(expectedSequence, executionSequence);
  EXPECT_TRUE(canceledAsExpected);
}

TEST(TestStrand, JoinStrandEndsScheduledTaskInError)
{
  qi::Strand strand;
  qi::Promise<void> startJoinProm;

  // Block the strand by executing the task in it.
  strand.post([&]{
    // The strand is blocked by the current task, the deferred task cannot execute yet.
    auto deferredTaskFut = strand.defer([]{});
    // Trigger the join, it must set the deferred task result future in error before waiting for the
    // current task to finish.
    startJoinProm.setValue(nullptr);
    EXPECT_TRUE(deferredTaskFut.hasError());
    EXPECT_NE(std::string::npos, deferredTaskFut.error().find("strand is dying"));
  });
  startJoinProm.future().wait();
  strand.join();
}
