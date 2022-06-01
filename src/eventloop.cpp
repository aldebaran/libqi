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
#include <boost/range/algorithm/count_if.hpp>

#include <ka/memory.hpp>
#include <ka/scoped.hpp>
#include <qi/preproc.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>

#include <qi/eventloop.hpp>
#include <qi/future.hpp>

#include <qi/getenv.hpp>

#include "eventloop_p.hpp"

qiLogCategory("qi.eventloop");

namespace qi {
  /// Component responsible for maintaining a container of threads.
  ///
  /// Threads that have finished are marked as inactive, and are later joined
  /// and overwritten when new threads must be created.
  class EventLoopAsio::WorkerThreadPool
  {
    // A thread associated to bookkeeping data:
    // - the last date it has run some work
    // - if it is currently active or not
    struct ThreadData
    {
      using Clock = SteadyClock;

      std::thread thread;
      Clock::time_point lastWorkDate = Clock::now();
      bool active = true;

      explicit ThreadData(std::thread t) : thread(std::move(t))
      {
      }
    };

    // TODO: Perform measurements to see if a more specific data structure would
    // get better performances.
    using Container = std::vector<ThreadData>;

  public:
    ~WorkerThreadPool() { joinAll(); }

    // Launches a new worker thread which will run the function 'func(args)'.
    // Note: It is undefined behavior to call this method while joinAll is also being called.
    template<class Func, class... Args>
    void launch(Func&& func, Args&&... args)
    {
      using ka::fwd;
      launchN(1, fwd<Func>(func), fwd<Args>(args)...);
    }

    // Launches 'count' new worker threads which will each run the function 'func(args)'.
    // Both the function func and the arguments args will be copied for each call.
    // Note: It is undefined behavior to call this method while joinAll is also being called.
    //
    // Algorithm:
    //  - launch as many threads as possible in "inactive slots" (threads that
    //      have finished their work)
    //  - launch the remaining threads in new slots
    //
    // Note: The thread container itself is never shrunk.
    template<class Func, class... Args>
    void launchN(int launchCount, Func&& func, Args&&... args)
    {
      auto syncedWorkers = _workers.synchronize();
      auto& workers = *syncedWorkers;
      auto b = workers.begin();
      const auto e = workers.end();
      while (launchCount != 0)
      {
        b = findInactive(b, e);
        if (b == e)
        {
          // No more inactive slots: push new threads.
          // `insert(pos, count, value)` cannot be used because `std::thread` is
          // not copyable.
          workers.reserve(static_cast<Container::size_type>(launchCount));
          while (launchCount != 0)
          {
            workers.emplace_back(std::thread{func, args...});
            --launchCount;
          }
          QI_ASSERT(launchCount == 0); // Postcondition.
        }
        else
        {
          // Inactive slot found. Join if possible and overwrite with a new thread.
          ThreadData& t = *b;
          if (t.thread.joinable())
          {
            t.thread.join();
          }
          t = ThreadData{std::thread{func, args...}};
          ++b;
          --launchCount;
        }
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

      for (auto& worker: workers)
      {
        if (worker.thread.joinable())
        {
          try
          {
            worker.thread.join();
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

    std::size_t activeWorkerCount() const
    {
      return activeWorkerCountUnsync(*_workers.synchronize());
    }

    void setInactive(const std::thread::id& id)
    {
      visitThreadDataUnsync(*_workers.synchronize(), id, [](ThreadData& t) {
        t.active = false;
      });
    }

    // A thread must terminate if it has been idle for too long, provided the
    // minimum number of threads has not been reached.
    bool mustTerminate(const std::thread::id& id, MilliSeconds maxIdleDuration,
      unsigned int minThreadCount) const
    {
      qiLogDebug() << "mustTerminate(" << id << ", " << maxIdleDuration.count() << " ms, " << minThreadCount << ")";
      auto syncedWorkers = _workers.synchronize();
      const auto& workers = *syncedWorkers;

      const bool threadIdleTooLong = ThreadData::Clock::now() -
        lastWorkDateUnsync(id, workers) > maxIdleDuration;

      return threadIdleTooLong && activeWorkerCountUnsync(workers) > minThreadCount;
    }

    void updateLastWorkDate(const std::thread::id& id)
    {
      auto syncedWorkers = _workers.synchronize();
      visitThreadDataUnsync(*syncedWorkers, id, [](ThreadData& t) {
        t.lastWorkDate = ThreadData::Clock::now();
      });
    }

  private:
    static bool isWorker(const Container& workers, std::thread::id id)
    {
      return boost::algorithm::any_of(workers, [&](const ThreadData& t) {
        return t.thread.get_id() == id;
      });
    }

    static std::size_t activeWorkerCountUnsync(const Container& workers)
    {
      return boost::range::count_if(workers, [](const ThreadData& t) {
        return t.active;
      });
    }

    /// Precondition: readableBoundedRange(b, e)
    ///
    /// Iterator<ThreadData> I
    template<typename I>
    static I findInactive(I b, I e)
    {
      return std::find_if(b, e, [](const ThreadData& t) {
        return !t.active;
      });
    }

    // Applies a procedure on the `ThreadData` with the given thread id.
    //
    // Throws `std::runtime_error` if the thread id is not found.
    //
    // Sequence<ThreadData> S,
    // Procedure<_ (ThreadData)> Proc
    template<typename S, typename Proc>
    static auto visitThreadDataUnsync(S&& workers, const std::thread::id& id, Proc proc)
      -> decltype(proc(*begin(workers)))
    {
      auto b = begin(workers);
      const auto e = end(workers);
      b = std::find_if(b, e, [=](const ThreadData& t) {
        return t.thread.get_id() == id;
      });
      if (b == e)
      {
        std::ostringstream ss;
        ss << "WorkerThreadPool::visitThreadDataUnsync: thread id not found. id = " << id;
        throw std::runtime_error(ss.str());
      }
      return proc(*b);
    }

    static ThreadData::Clock::time_point lastWorkDateUnsync(const std::thread::id& id,
        const Container& workers)
    {
      return visitThreadDataUnsync(workers, id, [](const ThreadData& t) {
        return t.lastWorkDate;
      });
    }

    boost::synchronized_value<Container> _workers;
  };

  using SteadyTimer = boost::asio::basic_waitable_timer<SteadyClock>;

  static std::atomic<uint64_t> gTaskId{0};
  static const auto gThreadCountEnvVar = "QI_EVENTLOOP_THREAD_COUNT";
  static const auto gMinThreadsEnvVar = "QI_EVENTLOOP_MIN_THREADS";
  static const auto gMaxThreadsEnvVar = "QI_EVENTLOOP_MAX_THREADS";
  static const auto gPingTimeoutEnvVar = "QI_EVENTLOOP_PING_TIMEOUT";
  static const auto gGracePeriodEnvVar = "QI_EVENTLOOP_GRACE_PERIOD";
  static const auto gMaxTimeoutsEnvVar = "QI_EVENTLOOP_MAX_TIMEOUTS";
  static const auto gThreadMaxIdleDurationMsEnvVar = "QI_EVENTLOOP_THREAD_MAX_IDLE_DURATION";
  const char* const EventLoopAsio::defaultName = "MainEventLoop";

  EventLoopAsio::EventLoopAsio(int threadCount, int minThreadCount, int maxThreadCount,
                               std::string name, bool spawnOnOverload)
    : EventLoopPrivate(std::move(name))
    , _io(threadCount)
    , _work(nullptr)
    , _minThreads(minThreadCount)
    , _maxThreads(maxThreadCount)
    , _workerThreads(new WorkerThreadPool())
    , _spawnOnOverload(spawnOnOverload)
  {
    start(threadCount);
  }

  EventLoopAsio::EventLoopAsio(int threadCount, std::string name, bool spawnOnOverload)
    : EventLoopAsio(threadCount, -1, 0, std::move(name), spawnOnOverload)
  {
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
      threadCount = qi::os::getEnvDefault(
        gThreadCountEnvVar,
        std::max(static_cast<int>(std::thread::hardware_concurrency()), 3));
    }

    _io.reset();
    delete _work.exchange(new boost::asio::io_service::work(_io));

    auto min = _minThreads.load();
    auto max = _maxThreads.load();

    qiLogVerbose() << "start: thread count limits: initial (minimum, maximum) "
      "before any adjustment = " << "(" << min << ", " << max << ")";

    if (min < 0)
    {
      min = qi::os::getEnvDefault(gMinThreadsEnvVar,
        static_cast<int>(std::thread::hardware_concurrency()));
      qiLogVerbose() << "start: thread count limits: min <- " << min
        << " (read from environment variable " << gMinThreadsEnvVar << ","
        << " with default " << std::thread::hardware_concurrency() << ")";
    }

    if (max <= 0)
    {
      const int defaultMax = 150;
      max = qi::os::getEnvDefault(gMaxThreadsEnvVar, defaultMax);
      qiLogVerbose() << "start: thread count limits: max <- " << max
        << " (read from environment variable " << gMaxThreadsEnvVar << ","
        << " with default " << defaultMax << ")";
    }

    if (max < min)
    {
      qiLogWarning() << "start: thread count limits: max (=" << max << ") < min (=" << min << ")";
      min = max;
      qiLogWarning() << "start: thread count limits: min / max adjustment: "
        << "min <- max <- " << min;
    }

    qiLogVerbose() << "start: thread count limits: final (minimum, maximum) after "
      << "potential adjustment = (" << min << ", " << max << ")";

    setMinThreads(min);
    setMaxThreads(max);

    if (threadCount < min)
    {
      qiLogWarning() << "start: thread limits: thread count (=" << threadCount << ") < min (=" << min << ")";
      threadCount = min;
      qiLogWarning() << "start: thread limits: thread count adjustment: thread count = min = " << min;
    }

    if (threadCount > max)
    {
      qiLogWarning() << "start: thread limits: thread count (=" << threadCount << ") > max (=" << max << ")";
      threadCount = max;
      qiLogWarning() << "start: thread limits: thread count adjustment: thread count = max = " << max;
    }

    qiLogVerbose() << "start: number of threads that will be launched = " << threadCount
      << " (between (min, max) = (" << min << ", " << max << "))";

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

  MilliSeconds EventLoopAsio::maxIdleDuration() const
  {
    static const MilliSeconds d{
      os::getEnvDefault(gThreadMaxIdleDurationMsEnvVar, 5000u)};
    return d;
  }

  // The thread running this function is responsible for:
  // - creating threads in case of contention
  // - destroying threads that have been idle for too long
  //
  // # Thread creation
  //
  // The detection of contention is done by launching a task and looking if it
  // has been run before a given duration. If the duration has been exceeded, a
  // new thread is spawned (a questionable legacy behavior...) under a maximum
  // limit. If this maximum limit is reached too many times, an
  // "emergency callback" is called.
  //
  // # Thread destruction
  //
  // Each thread is associated to the last date it has run a task. A task is
  // periodically launched by the "ping loop" to test the contention. This task
  // also checks how long the thread has been idle (the running of this
  // monitoring task does not count as real work). If the maximum idle duration
  // has been exceeded, a specific exception is thrown causing the worker to
  // stop.
  //
  // Note: On a lower-level side, it is the worker thread pool
  // (`WorkerThreadPool`) that is responsible for the management of the
  // container of threads.
  void EventLoopAsio::runPingLoop()
  {
    qi::os::setCurrentThreadName("EvLoop.mon");
    const auto timeoutDuration = MilliSeconds{ qi::os::getEnvDefault(gPingTimeoutEnvVar, 500u) };
    const auto graceDuration = MilliSeconds{ qi::os::getEnvDefault(gGracePeriodEnvVar, 0u) };
    const auto maxTimeouts = qi::os::getEnvDefault(gMaxTimeoutsEnvVar, 20u);
    const auto maxIdle = maxIdleDuration();
    const auto prefix = "Threadpool " + _name + ": ";

    unsigned int nbTimeout = 0;
    while (_work.load())
    {
      qiLogDebug() << "Ping";
      auto calling = asyncCallInternal(Seconds{0},
        [this, maxIdle]() {
          // TODO: check that the eventloop cannot be dead by this point.
          if (_workerThreads->mustTerminate(
                std::this_thread::get_id(), maxIdle, _minThreads.load()))
          {
            throw detail::TerminateThread{};
          }
        },
        defaultExecutionOptions(),
        UpdateLastWorkDate{false} // the "ping task" doesn't count as real work.
      );
      auto callState = calling.waitFor(timeoutDuration);
      QI_ASSERT(callState != FutureState_None);
      if (callState == FutureState_Running)
      {
        const auto workerCount = static_cast<int>(_workerThreads->activeWorkerCount());
        const auto maxThreads = _maxThreads.load();
        if (maxThreads && workerCount >= maxThreads) // we count in nThreads
        {
          ++nbTimeout;
          qiLogInfo() << prefix << "Size limit reached ("
                      << nbTimeout << " timeouts / " << maxTimeouts << " max"
                      << ", number of tasks: " << _totalTask.load()
                      << ", number of active tasks: " << _activeTask.load()
                      << ", number of threads: " << workerCount
                      << ", maximum number of threads: " << maxThreads << ")";

          if (nbTimeout >= maxTimeouts)
          {
            qiLogError() << prefix <<
              "System seems to be deadlocked, sending emergency signal";
            {
              auto syncedEmergencyCallback = _emergencyCallback.synchronize();
              if (*syncedEmergencyCallback)
              {
                try {
                  (*syncedEmergencyCallback)();
                } catch (const std::exception& ex) {
                  qiLogWarning() << prefix << "Emergency callback failed: " << ex.what();
                } catch (...) {
                  qiLogWarning() << prefix << "Emergency callback failed: unknown exception";
                }
              }
            }
          }
        }
        else
        {
          const auto nextWorkerCount = workerCount + 1;
          const auto minThreads = _minThreads.load();
          std::ostringstream details;
          details << "min: " << minThreads << ", max: ";
          if (maxThreads) details << maxThreads;
          else            details << "no limit";
          if (minThreads != 0)
          {
            const auto sizeRatioMin = 100*nextWorkerCount/minThreads;
            details << ", size/min: " << sizeRatioMin << "%";
          }
          if (maxThreads != 0)
          {
            const auto sizeRatioMax = detail::posInBetween(0, nextWorkerCount, maxThreads);
            details << ", size/max: " << sizeRatioMax << "%";
            const auto growingCapacity = maxThreads - minThreads; // [min,max]
            const auto growth = nextWorkerCount - minThreads;
            const auto growingRate = detail::posInBetween(minThreads, nextWorkerCount, maxThreads);
            details << ", growth ratio: " << growingRate << "%"
                    << " (" << growth << "/" << growingCapacity << ")";
          }
          qiLogInfo() << prefix << "Spawning 1 more thread. New size: " << nextWorkerCount << " (" << details.str() << ")";

          try
          {
            _workerThreads->launch(&EventLoopAsio::runWorkerLoop, this);
          }
          catch (const std::system_error& ex)
          {
            // TODO: report some system info about memory usage etc. in this case.
            // One of the possible reason to fail here is that there is no memory available.
            qiLogWarning() << prefix << "Spawning thread " << nextWorkerCount
              << " failed with system error "<< ex.code() << " : " << ex.what();
          }
          catch (const std::exception& ex)
          {
            qiLogWarning() << prefix << "Spawning thread " << nextWorkerCount
              << " failed with error: " << ex.what();
          }
          catch (...)
          {
            qiLogWarning() << prefix << "Spawning thread " << nextWorkerCount
              << " failed with unknown error.";
          }

        }
        boost::this_thread::sleep_for(graceDuration);
      }
      else
      {
        // If the event loop has been stopped and work has been destroyed at this point then
        // maybe the future has been set in error, so just ignore the result and leave.
        if (!_work.load())
        {
          qiLogDebug() << prefix << "Ignoring ping result, the event loop is being stopped";
          break;
        }

        const bool idleThreadHasTerminated =
             callState == FutureState_FinishedWithError
          && calling.error() == detail::TerminateThread::message();

        QI_IGNORE_UNUSED(idleThreadHasTerminated);
        QI_ASSERT(callState == FutureState_FinishedWithValue || idleThreadHasTerminated);
        nbTimeout = 0;
        qiLogDebug() << prefix << "Ping ok";
        boost::this_thread::sleep_for(timeoutDuration);
      }
    }
  }

  void EventLoopAsio::runWorkerLoop()
  {
    qiLogDebug() << this << ": run starting from pool "
      "(workerCount = " << _workerThreads->activeWorkerCount() << ")";
    qi::os::setCurrentThreadName(_name);

    while (true) {
      try
      {
        _io.run();
        //the handler finished by himself. just quit.
        break;
      } catch(const detail::TerminateThread& /* e */) {
        _workerThreads->setInactive(std::this_thread::get_id());
        qiLogVerbose() << _name << ": Terminated idle thread "
          "(new worker count = " << _workerThreads->activeWorkerCount() << ')';
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
                                   const boost::system::error_code& erc, D countTask, UpdateLastWorkDate update)
  {
    boost::ignore_unused(id, countTask);
    if (!erc)
    {
      auto _ = ka::scoped_incr_and_decr(_activeTask);

      try
      {
        f();
        p.setValue(0);
      }
      catch (const detail::TerminateThread& /* e */)
      {
        p.setError(detail::TerminateThread::message());
        throw;
      }
      catch (const std::exception& ex)
      {
        p.setError(ex.what());
      }
      catch (...)
      {
        p.setError("unknown error");
      }
    }
    else
    {
      p.setCanceled();
    }
    if (src(update))
    {
      _workerThreads->updateLastWorkDate(std::this_thread::get_id());
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

      auto countTotalTask = ka::shared_ptr(ka::scoped_incr_and_decr(_totalTask));
      _io.post([=] { invoke_maybe(cb, id, Promise<void>{}, erc, countTotalTask,
                                  UpdateLastWorkDate{true}); });
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
    return asyncCallInternal(delay, std::move(cb), options, UpdateLastWorkDate{true});
  }

  qi::Future<void> EventLoopAsio::asyncCallInternal(
    qi::Duration delay, boost::function<void ()> cb, ExecutionOptions options,
    UpdateLastWorkDate update)
  {
    static boost::system::error_code erc;

    if (!_work.load())
      return qi::makeFutureError<void>("Schedule attempt on destroyed thread pool");

    const auto id = ++gTaskId;

    auto countTotalTask = ka::shared_ptr(ka::scoped_incr_and_decr(_totalTask));

    if (delay > Duration::zero())
    {
      boost::shared_ptr<boost::asio::steady_timer> timer = boost::make_shared<boost::asio::steady_timer>(_io);
      timer->expires_from_now(boost::chrono::duration_cast<boost::asio::steady_timer::duration>(delay));
      auto prom = detail::makeCancelingPromise(options, boost::bind(&boost::asio::steady_timer::cancel, timer));
      timer->async_wait([=](const boost::system::error_code& erc) {
        invoke_maybe(cb, id, prom, erc, countTotalTask, update);
      });
      return prom.future();
    }
    Promise<void> prom;
    _io.post([=] { invoke_maybe(cb, id, prom, erc, countTotalTask, update); });
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
    return asyncCallInternal(timepoint, std::move(cb), options, UpdateLastWorkDate{true});
  }

  qi::Future<void> EventLoopAsio::asyncCallInternal(
    qi::SteadyClockTimePoint timepoint, boost::function<void ()> cb,
    ExecutionOptions options, UpdateLastWorkDate update)
  {
    if (!_work.load())
      return qi::makeFutureError<void>("Schedule attempt on destroyed thread pool");

    const auto id = ++gTaskId;

    auto countTotalTask = ka::shared_ptr(ka::scoped_incr_and_decr(_totalTask));

    boost::shared_ptr<SteadyTimer> timer = boost::make_shared<SteadyTimer>(_io);
    timer->expires_at(timepoint);
    auto prom = detail::makeCancelingPromise(options, boost::bind(&SteadyTimer::cancel, timer));
    timer->async_wait([=](const boost::system::error_code& erc) {
      invoke_maybe(cb, id, prom, erc, countTotalTask, update);
    });
    return prom.future();
  }

  void EventLoopAsio::setMinThreads(unsigned int min)
  {
    _minThreads = static_cast<int>(min);
  }

  void EventLoopAsio::setMaxThreads(unsigned int max)
  {
    _maxThreads = static_cast<int>(max);
  }

  void* EventLoopAsio::nativeHandle()
  {
    return static_cast<void*>(&_io);
  }

  int EventLoopAsio::workerCount() const
  {
    return _workerThreads->activeWorkerCount();
  }

  EventLoop::EventLoop(std::string name, int nthreads, bool spawnOnOverload)
    : _p(std::make_shared<EventLoopAsio>(nthreads, name, spawnOnOverload))
    , _name(name)
  {
  }

  EventLoop::EventLoop(std::string name, int nthreads, int minThreads, int maxThreads,
    bool spawnOnOverload)
    : _p(std::make_shared<EventLoopAsio>(nthreads, minThreads, maxThreads, name, spawnOnOverload))
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

  void EventLoop::setMinThreads(unsigned int min)
  {
    return safeCall(_p, [=](const ImplPtr& impl){
      return impl->setMinThreads(min);
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
    EventLoop* _getInternal(EventLoop* &ctx, int nthreads,
      const std::string& name, bool spawnOnOverload, boost::mutex& mutex,
      std::atomic<int>& init, int minThreads, int maxThreads)
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
          // TODO: use make_unique once we can use C++14
          ctx = new EventLoop(name, nthreads, minThreads, maxThreads, spawnOnOverload);
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
    // We do not decide here the min thread count, nor the max thread count.
    // Let the defaults be used (hence, min = -1, max = 0)
    return _getInternal(ctx, nthreads, EventLoopAsio::defaultName, true, mutex, init, -1, 0);
  }

  static EventLoop* _getNetwork(EventLoop* &ctx)
  {
    static boost::mutex mutex;
    static std::atomic<int> init(0);
    // This eventloop has only one thread (hence, min thread count = max thread count = 1).
    return _getInternal(ctx, 1, "EventLoopNetwork", false, mutex, init, 1, 1);
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
