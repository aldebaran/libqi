/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <thread>
#include <system_error>
#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <boost/thread/thread.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/core/ignore_unused.hpp>

#include <ka/memory.hpp>
#include <ka/scoped.hpp>
#include <qi/preproc.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>

#include <qi/eventloop.hpp>
#include <qi/future.hpp>

#include <qi/getenv.hpp>

#include "eventloop_p.hpp"
#ifdef WITH_PROBES
# include "tp_qi.h"
#else
# define tracepoint(...)
#endif

qiLogCategory("qi.eventloop");

namespace qi {

  class EventLoopAsio::WorkerThreadPool
  {
    using Container = std::vector<std::thread>;

  public:
    ~WorkerThreadPool() { joinAll(); }

    // Launches a new worker thread which will run the function 'func(args)'.
    // Note: It is undefined behavior to call this method while joinAll is also being called.
    template<class Func, class... Args>
    void launch(Func&& func, Args&&... args)
    {
      _workers->emplace_back(std::forward<Func>(func), std::forward<Args>(args)...);
    }

    // Launches 'count' new worker threads which will each run the function 'func(args)'.
    // Both the function func and the arguments args will be copied for each call.
    // Note: It is undefined behavior to call this method while joinAll is also being called.
    template<class Func, class... Args>
    void launchN(int count, Func&& func, Args&&... args)
    {
      auto syncedWorkers = _workers.synchronize();
      syncedWorkers->reserve(syncedWorkers->size() + static_cast<Container::size_type>(count));
      for (int i = 0; i < count; ++i)
      {
        syncedWorkers->emplace_back(func, args...);
      }
    }

    // Joins all the worker threads and clears the threads container, effectively destroying the thread objects.
    // Throws a std::system_error if called from one of the worker threads as this would be a deadlock.
    void joinAll()
    {
      Container workers;
      {
        auto syncedWorkers = _workers.synchronize();
        if (isWorker(*syncedWorkers, std::this_thread::get_id()))
        {
          throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
        }

        using std::swap;
        swap(*syncedWorkers, workers);
      }

      for (auto& worker : workers)
      {
        if (worker.joinable())
        {
          try
          {
            worker.join();
          }
          catch (const std::exception& ex)
          {
            qiLogWarning() << "Failed to join a worker thread: " << ex.what();
          }
        }
      }
    }

    // This method is thread safe but is ambiguous when joinAll is also being called.
    Container::size_type size() const
    {
      return _workers->size();
    }

    // This method is thread safe but may return a false negative when joinAll is also being called.
    bool isWorker(std::thread::id id) const
    {
      return isWorker(*_workers, id);
    }

  private:
    static bool isWorker(const Container& workers, std::thread::id id)
    {
      return boost::algorithm::any_of(workers, [&](const std::thread& t) { return t.get_id() == id; });
    }

    boost::synchronized_value<Container> _workers;
  };

  using SteadyTimer = boost::asio::basic_waitable_timer<SteadyClock>;

  static std::atomic<uint64_t> gTaskId{0};
  static const auto gThreadCountEnvVar = "QI_EVENTLOOP_THREAD_COUNT";
  static const auto gMaxThreadsEnvVar  = "QI_EVENTLOOP_MAX_THREADS";
  static const auto gPingTimeoutEnvVar = "QI_EVENTLOOP_PING_TIMEOUT";
  static const auto gGracePeriodEnvVar = "QI_EVENTLOOP_GRACE_PERIOD";
  static const auto gMaxTimeoutsEnvVar = "QI_EVENTLOOP_MAX_TIMEOUTS";
  const char* const EventLoopAsio::defaultName = "MainEventLoop";

  EventLoopAsio::EventLoopAsio(int threadCount, std::string name, bool spawnOnOverload)
    : EventLoopPrivate(std::move(name))
    , _work(nullptr)
    , _maxThreads(0)
    , _workerThreads(new WorkerThreadPool())
    , _spawnOnOverload(spawnOnOverload)
  {
    start(threadCount);
  }

  void EventLoopAsio::start(int threadCount)
  {
    if (_workerThreads->size() > 0) // workers are already running
    {
      qiLogVerbose() << "The event loop is already started and worker threads are running, this call to start is ignored.";
      return;
    }

    if (threadCount <= 0)
    {
      threadCount =
          qi::os::getEnvDefault(gThreadCountEnvVar, std::max(static_cast<int>(std::thread::hardware_concurrency()), 3));
    }

    _io.reset();
    delete _work.exchange(new boost::asio::io_service::work(_io));

    _maxThreads = qi::os::getEnvDefault(gMaxThreadsEnvVar, 150);
    _workerThreads->launchN(threadCount, &EventLoopAsio::runWorkerLoop, this);
    if (_spawnOnOverload)
    {
      _pingThread = std::thread(&EventLoopAsio::runPingLoop, this);
    }
  }

  EventLoopAsio::~EventLoopAsio()
  {
    try
    {
      stop();
    }
    catch (const std::exception& ex)
    {
      qiLogWarning() << "Failed to stop and join the EventLoopAsio: " << ex.what();
    }
    catch (...)
    {
      qiLogWarning() << "Failed to stop and join the EventLoopAsio: unknown exception";
    }
  }

  void EventLoopAsio::stop()
  {
    qiLogDebug() << "Stopping EventLoopAsio: " << this;
    delete _work.exchange(nullptr);

    // FIXME: Although destroying _work should be enough, we have to explicitly stop the io_service
    // because in some cases some work seems to get "stuck" in it for a reason that is unknown yet.
    _io.stop();

    join();
  }

  void EventLoopAsio::runPingLoop()
  {
    qi::os::setCurrentThreadName("EvLoop.mon");
    static const auto timeoutDuration = MilliSeconds{ qi::os::getEnvDefault(gPingTimeoutEnvVar, 500u) };
    static const auto graceDuration = MilliSeconds{ qi::os::getEnvDefault(gGracePeriodEnvVar, 0u) };
    static const auto maxTimeouts = qi::os::getEnvDefault(gMaxTimeoutsEnvVar, 20u);

    unsigned int nbTimeout = 0;
    while (_work.load())
    {
      qiLogDebug() << "Ping";
      auto calling = asyncCall(Seconds{0}, []{});
      auto callState = calling.waitFor(timeoutDuration);
      QI_ASSERT(callState != FutureState_None);
      if (callState == FutureState_Running)
      {
        const auto workerCount = static_cast<int>(_workerThreads->size());
        const auto maxThreads = _maxThreads.load();
        if (maxThreads && workerCount > maxThreads) // we count in nThreads
        {
          ++nbTimeout;
          qiLogInfo() << "Threadpool " << _name << " limit reached (" << nbTimeout
                      << " timeouts, number of tasks: " << _totalTask.load()
                      << ", number of active tasks: " << _activeTask.load()
                      << ", number of threads: " << workerCount
                      << ", maximum number of threads: " << maxThreads << ")";

          if (nbTimeout >= maxTimeouts)
          {
            qiLogError() << "Threadpool " << _name <<
              ": System seems to be deadlocked, sending emergency signal";
            {
              auto syncedEmergencyCallback = _emergencyCallback.synchronize();
              if (*syncedEmergencyCallback)
              {
                try {
                  (*syncedEmergencyCallback)();
                } catch (const std::exception& ex) {
                  qiLogWarning() << "Emergency callback failed: " << ex.what();
                } catch (...) {
                  qiLogWarning() << "Emergency callback failed: unknown exception";
                }
              }
            }
          }
        }
        else
        {
          qiLogInfo() << _name << ": Spawning more threads (" << workerCount << ')';
          _workerThreads->launch(&EventLoopAsio::runWorkerLoop, this);
        }
        boost::this_thread::sleep_for(graceDuration);
      }
      else
      {
        // If the event loop has been stopped and work has been destroyed at this point then
        // maybe the future has been set in error, so just ignore the result and leave.
        if (!_work.load())
        {
          qiLogDebug() << "Ignoring ping result, the event loop is being stopped";
          break;
        }

        QI_ASSERT(callState == FutureState_FinishedWithValue);
        nbTimeout = 0;
        qiLogDebug() << "Ping ok";
        boost::this_thread::sleep_for(timeoutDuration);
      }
    }
  }

  void EventLoopAsio::runWorkerLoop()
  {
    qiLogDebug() << this << "run starting from pool";
    qi::os::setCurrentThreadName(_name);

    while (true) {
      try
      {
        _io.run();
        //the handler finished by himself. just quit.
        break;
      } catch(const detail::TerminateThread& /* e */) {
        break;
      } catch(const std::exception& e) {
        qiLogWarning() << "Error caught in eventloop(" << _name << ").async: " << e.what();
      } catch(...) {
        qiLogWarning() << "Uncaught exception in eventloop(" << _name << ")";
      }
    }
  }

  bool EventLoopAsio::isInThisContext() const
  {
    return _workerThreads->isWorker(std::this_thread::get_id());
  }

  void EventLoopAsio::join()
  {
    if (_pingThread.joinable())
    {
      qiLogVerbose() << "Waiting for the ping thread ...";
      _pingThread.join();
      qiLogDebug()  << "Waiting for the ping thread - DONE";
    }

    qiLogVerbose()
        << "Waiting threads from the pool \"" << _name << "\", remaining tasks: "
        << _totalTask.load() << " (" << _activeTask.load() <<  " active)...";
    _workerThreads->joinAll();
    qiLogDebug()  << "Waiting threads from the pool - DONE";
  }

  /// Destructible D
  template <typename D>
  void EventLoopAsio::invoke_maybe(boost::function<void()> f, qi::uint64_t id, qi::Promise<void> p,
                                   const boost::system::error_code& erc, D countTask)
  {
    boost::ignore_unused(id, countTask);
    if (!erc)
    {
      auto _ = ka::scoped_incr_and_decr(_activeTask);
      tracepoint(qi_qi, eventloop_task_start, id);

      try
      {
        f();
        tracepoint(qi_qi, eventloop_task_stop, id);
        p.setValue(0);
      }
      catch (const detail::TerminateThread& /* e */)
      {
        throw;
      }
      catch (const std::exception& ex)
      {
        tracepoint(qi_qi, eventloop_task_error, id);
        p.setError(ex.what());
      }
      catch (...)
      {
        tracepoint(qi_qi, eventloop_task_error, id);
        p.setError("unknown error");
      }
    }
    else
    {
      tracepoint(qi_qi, eventloop_task_cancel, id);
      p.setCanceled();
    }
  }

  void EventLoopAsio::post(qi::Duration delay,
      const boost::function<void ()>& cb, ExecutionOptions options)
  {
    static boost::system::error_code erc;

    if (!_work.load())
    {
      // This seems to be an error but as we have this log a lot sometimes at the destruction
      // of an event loop, it has been decided to tune it down.
      qiLogVerbose() << "Schedule attempt on destroyed thread pool";
      return;
    }

    if (delay == qi::Duration(0))
    {
      const auto id = ++gTaskId;
      tracepoint(qi_qi, eventloop_post, id, cb.target_type().name());

      auto countTotalTask = ka::shared_ptr(ka::scoped_incr_and_decr(_totalTask));
      _io.post([=] { invoke_maybe(cb, id, Promise<void>{}, erc, countTotalTask); });
    }
    else
    {
      asyncCall(delay, cb, options).then([](const Future<void>& fut)
      {
        if (fut.hasError())
        {
          qiLogError() << "Error during asyncCall: " << fut.error();
        }
      });
    }
  }

  namespace detail
  {
    template<class CancelFunc>
    qi::Promise<void> makeCancelingPromise(ExecutionOptions options, CancelFunc&& onCancel)
    {
      if (options.onCancelRequested == CancelOption::NeverSkipExecution)
        return qi::Promise<void>();
      else
        return qi::Promise<void>(std::forward<CancelFunc>(onCancel));
    }
  }

  qi::Future<void> EventLoopAsio::asyncCall(qi::Duration delay,
      boost::function<void ()> cb, ExecutionOptions options)
  {
    static boost::system::error_code erc;

    if (!_work.load())
      return qi::makeFutureError<void>("Schedule attempt on destroyed thread pool");

    const auto id = ++gTaskId;

    auto countTotalTask = ka::shared_ptr(ka::scoped_incr_and_decr(_totalTask));

    tracepoint(qi_qi, eventloop_delay, id, cb.target_type().name(), boost::chrono::duration_cast<qi::MicroSeconds>(delay).count());
    if (delay > Duration::zero())
    {
      boost::shared_ptr<boost::asio::steady_timer> timer = boost::make_shared<boost::asio::steady_timer>(boost::ref(_io));
      timer->expires_from_now(boost::chrono::duration_cast<boost::asio::steady_timer::duration>(delay));
      auto prom = detail::makeCancelingPromise(options, boost::bind(&boost::asio::steady_timer::cancel, timer));
      timer->async_wait([=](const boost::system::error_code& erc) { invoke_maybe(cb, id, prom, erc, countTotalTask); });
      return prom.future();
    }
    Promise<void> prom;
    _io.post([=] { invoke_maybe(cb, id, prom, erc, countTotalTask); });
    return prom.future();
  }

  void EventLoopAsio::post(qi::SteadyClockTimePoint timepoint,
      const boost::function<void ()>& cb, ExecutionOptions options)
  {
    asyncCall(timepoint, cb, options).then([](const Future<void>& fut)
    {
      if (fut.hasError())
      {
        qiLogError() << "Error during asyncCall: " << fut.error();
      }
    });
  }

  qi::Future<void> EventLoopAsio::asyncCall(qi::SteadyClockTimePoint timepoint,
      boost::function<void ()> cb, ExecutionOptions options)
  {
    if (!_work.load())
      return qi::makeFutureError<void>("Schedule attempt on destroyed thread pool");

    const auto id = ++gTaskId;

    auto countTotalTask = ka::shared_ptr(ka::scoped_incr_and_decr(_totalTask));

    //tracepoint(qi_qi, eventloop_delay, id, cb.target_type().name(), qi::MicroSeconds(delay).count());
    boost::shared_ptr<SteadyTimer> timer = boost::make_shared<SteadyTimer>(boost::ref(_io));
    timer->expires_at(timepoint);
    auto prom = detail::makeCancelingPromise(options, boost::bind(&SteadyTimer::cancel, timer));
    timer->async_wait([=](const boost::system::error_code& erc) { invoke_maybe(cb, id, prom, erc, countTotalTask); });
    return prom.future();
  }

  void EventLoopAsio::setMaxThreads(unsigned int max)
  {
    _maxThreads = static_cast<int>(max);
  }

  void* EventLoopAsio::nativeHandle()
  {
    return static_cast<void*>(&_io);
  }

  EventLoop::EventLoop(std::string name, int nthreads, bool spawnOnOverload)
    : _p(std::make_shared<EventLoopAsio>(nthreads, name, spawnOnOverload))
    , _name(name)
  {
  }

  EventLoop::~EventLoop()
  {
    // TODO after compiler upgrades: auto p = std::atomic_exchange(&_p, {});
    // We need to acquire a copy of the ptr but also reset the member ptr.
    ImplPtr localImpl;
    swap(_p, localImpl);

    while (localImpl.use_count() > 1) // Until we are the sole owner.
    {
      std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }
  }

  // With P = std::shared_ptr<T>
  // boost::synchronized_value<P> PP
  // Procedure<U (P)> Proc1
  // Procedure<U ()> Proc2
  template<typename PP, typename Proc1, typename Proc2 = ka::constant_function_t<void>>
    auto safeCall(PP& syncPtr, Proc1&& proc, Proc2&& onFail = Proc2{})
      -> decltype(std::forward<Proc1>(proc)(syncPtr.get()))
  {
    if (auto ptr = syncPtr.get()) // Keep alive for the time of the call
      return std::forward<Proc1>(proc)(ptr);
    else
      return std::forward<Proc2>(onFail)();
  }

  bool EventLoop::isInThisContext() const
  {
    return safeCall(_p, [](const ImplPtr& impl){
        return impl->isInThisContext();
      }
    , []{
        qiLogDebug() << "EventLoop::isInThisContext() called while EventLoop instance is destroying - ignored.";
        return false;
      }
    );
  }

  void EventLoop::join()
  {
    qiLogDebug() << __FUNCTION__ << " is deprecated, the EventLoop is automatically joined when stopped";
    return safeCall(_p, [](const ImplPtr& impl) {
      return impl->join();
    });
  }

  void EventLoop::start(int nthreads)
  {
    qiLogDebug() << __FUNCTION__ << " is deprecated, the EventLoop is automatically started when constructed";
    return safeCall(_p, [=](const ImplPtr& impl) {
      return impl->start(nthreads);
    });
  }

  void EventLoop::stop()
  {
    qiLogDebug() << __FUNCTION__ << " is deprecated, the EventLoop is automatically stopped when destroyed";
    return safeCall(_p, [](const ImplPtr& impl){
      return impl->stop();
    });
  }

  void *EventLoop::nativeHandle()
  {
    return safeCall(_p, [](const ImplPtr& impl){
      return impl->nativeHandle();
    }
    , []{ return nullptr; });
  }

  void EventLoop::postDelayImpl(boost::function<void()> callback,
      qi::Duration delay, ExecutionOptions options)
  {
    return safeCall(_p, [&](const ImplPtr& impl){
      qiLogDebug() << this << " EventLoop post " << &callback;
      impl->post(delay, callback, options);
      qiLogDebug() << this << " EventLoop post done " << &callback;
    });
  }

  void EventLoop::post(const boost::function<void()>& callback,
      qi::SteadyClockTimePoint timepoint)
  {
    return safeCall(_p, [&](const ImplPtr& impl){
      qiLogDebug() << this << " EventLoop post " << &callback;
      impl->post(timepoint, callback);
      qiLogDebug() << this << " EventLoop post done " << &callback;
    });
  }

  namespace {
    qi::Future<void> onDestructingError()
    {
      return qi::makeFutureError<void>("Async call attempted while EventLoop instance is destroying.");
    }
  }


  qi::Future<void> EventLoop::asyncDelayImpl(boost::function<void()> callback, qi::Duration delay, ExecutionOptions options)
  {
    return safeCall(_p, [&](const ImplPtr& impl) {
        return impl->asyncCall(delay, callback, options);
      }, onDestructingError );
  }

  qi::Future<void> EventLoop::asyncAtImpl(boost::function<void()> callback, qi::SteadyClockTimePoint timepoint, ExecutionOptions options)
  {
    return safeCall(_p, [&](const ImplPtr& impl){
      return impl->asyncCall(timepoint, callback, options);
    }, onDestructingError );
  }

  void EventLoop::setEmergencyCallback(boost::function<void()> cb)
  {
    return safeCall(_p, [&](const ImplPtr& impl){
      *impl->_emergencyCallback = cb;
    });
  }

  void EventLoop::setMaxThreads(unsigned int max)
  {
    return safeCall(_p, [=](const ImplPtr& impl){
      return impl->setMaxThreads(max);
    });
  }

  struct MonitorContext
  {
    EventLoop* target;
    EventLoop* helper;
    Future<void> mon;
    bool isFired;  // true: pinging, false: waiting for next ping.
    bool ending;
    uint64_t maxDelay;
    Promise<void> promise;
    int64_t startTime;
  };

  static void monitor_pingtimeout(boost::shared_ptr<MonitorContext> ctx)
  {
    if (!ctx->isFired)
      return; // Got the pong in the meantime, abort
    ctx->promise.setError("Event loop monitor timeout");
    /* Ping system is still on, but promise is set.
     * So future invocations of cancel() will be ignored, which makes the
     * monitoring unstopable.
     * So reset the value.
    */
    ctx->promise = Promise<void>();
  }

  static void monitor_cancel(qi::Promise<void>, boost::shared_ptr<MonitorContext> ctx)
  {
    ctx->ending = true;
    try {
      ctx->mon.cancel();
    }
    catch (...)
    {}
  }

  static void monitor_ping(boost::shared_ptr<MonitorContext> ctx)
  {
    if (ctx->ending)
      return;
    if (ctx->isFired)
    { // This is a pong
      ctx->isFired = false;
      // Cancel monitoring async call
      try {
        ctx->mon.cancel();
      }
      catch (const std::exception& e) {
        qiLogDebug() << "Failed to cancel monitoring async call: " << e.what();
      }
      uint64_t pingDelay = os::ustime() - ctx->startTime;
      if (pingDelay > ctx->maxDelay / 2)
        qiLogDebug() << "Long ping " << pingDelay;
      // Wait a bit before pinging againg
      ctx->helper->asyncDelay(boost::bind(&monitor_ping, ctx), qi::MicroSeconds(ctx->maxDelay*5));
    }
    else
    { // Delay between pings reached, ping again
      ctx->startTime = os::ustime();
      ctx->isFired = true;
      // Start monitor async first, or the ping async can trigger before the
      // monitor async is setup
      ctx->mon = ctx->helper->asyncDelay(boost::bind(&monitor_pingtimeout, ctx), qi::MicroSeconds(ctx->maxDelay));
      ctx->target->post(boost::bind(&monitor_ping, ctx));
    }
  }

  qi::Future<void> EventLoop::monitorEventLoop(EventLoop* helper, uint64_t maxDelay)
  {
    // Context data is a Future*[2]
    boost::shared_ptr<MonitorContext> ctx = boost::make_shared<MonitorContext>();
    ctx->target = this;
    ctx->helper = helper;
    ctx->maxDelay = maxDelay;
    ctx->promise = Promise<void>(boost::bind<void>(&monitor_cancel, _1, ctx));
    ctx->isFired = false;
    ctx->ending = false;
    monitor_ping(ctx);
    return ctx->promise.future();
  }

  static void eventloop_stop(EventLoop* &ctx)
  {
    delete ctx;
    ctx = 0;
  }

  static EventLoop* _poolEventLoop = nullptr;
  static EventLoop* _networkEventLoop = nullptr;

  namespace
  {
    // The initialisation is protected by a mutex,
    // We then use an atomic to prevent having a mutex on a fastpath.
    EventLoop* _getInternal(EventLoop* &ctx, int nthreads, const std::string& name,
      bool spawnOnOverload, boost::mutex& mutex, std::atomic<int>& init)
    {
      if (init.load())
        return ctx;

      {
        boost::mutex::scoped_lock _sl(mutex);
        if (!ctx)
        {
          if (!qi::Application::initialized())
          {
            qiLogVerbose() << "Creating event loop while no qi::Application() is running";
          }
          ctx = new EventLoop(name, nthreads, spawnOnOverload); // TODO: use make_unique once we can use C++14
          Application::atExit(boost::bind(&eventloop_stop, boost::ref(ctx)));
        }
      }
      ++init;
      return ctx;
    }
  }

  static EventLoop* _get(EventLoop* &ctx, int nthreads)
  {
    static boost::mutex mutex;
    static std::atomic<int> init(0);
    return _getInternal(ctx, nthreads, EventLoopAsio::defaultName, true, mutex, init);
  }

  static EventLoop* _getNetwork(EventLoop* &ctx)
  {
    static boost::mutex mutex;
    static std::atomic<int> init(0);
    return _getInternal(ctx, 1, "EventLoopNetwork", false, mutex, init);
  }

  void startEventLoop(int nthread)
  {
    _get(_poolEventLoop, nthread);
  }

  EventLoop* getEventLoop()
  {
    return _get(_poolEventLoop, 0);
  }

  EventLoop* getNetworkEventLoop()
  {
    return _getNetwork(_networkEventLoop);
  }

}
