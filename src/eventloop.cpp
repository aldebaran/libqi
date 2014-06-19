/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>

#include <qi/preproc.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qi/threadpool.hpp>

#include <qi/eventloop.hpp>
#include <qi/future.hpp>

#include "eventloop_p.hpp"
#include "tp_qi.h"

qiLogCategory("qi.eventloop");

namespace qi {

  static qi::Atomic<uint32_t> gTaskId = 0;


  template<typename T>
  static T getEnvParam(const char* name, T defaultVal)
  {
    std::string sval = qi::os::getenv(name);
    if (sval.empty())
      return defaultVal;
    else
      return boost::lexical_cast<T>(sval);
  }

  EventLoopAsio::EventLoopAsio()
  : _mode(Mode_Unset)
  , _destroyMe(false)
  {
    _name = "asioeventloop";
  }


  void EventLoopAsio::start(int nthread)
  {
    if (*_running || _mode != Mode_Unset)
      return;
    if (nthread == 0)
    {
      nthread = boost::thread::hardware_concurrency();
      if (nthread < 3)
        nthread = 3;
      const char* envNthread = getenv("QI_EVENTLOOP_THREAD_COUNT");
      if (envNthread)
        nthread = strtol(envNthread, 0, 0);
    }
    if (nthread == 1)
    {
      _mode = Mode_Threaded;
      _thd = boost::thread(&EventLoopPrivate::run, this);
    }
    else
    {
      _maxThreads = getEnvParam("QI_EVENTLOOP_MAX_THREADS", 150);
      _mode = Mode_Pooled;
      _work = new boost::asio::io_service::work(_io);
      for (int i=0; i<nthread; ++i)
        boost::thread(&EventLoopAsio::_runPool, this);
      boost::thread(&EventLoopAsio::_pingThread, this);
    }
    while (!*_running)
      qi::os::msleep(0);
  }

  EventLoopAsio::~EventLoopAsio()
  {
    if (*_running && boost::this_thread::get_id() == _id)
      qiLogError() << "Destroying EventLoopPrivate from itself while running";
    stop();
    join();
  }

  void EventLoopAsio::destroy()
  {
    if (isInEventLoopThread())
      boost::thread(&EventLoopAsio::destroy, this);
    else
    {
      delete this;
    }
  }

  static void ping_me(bool & ping, boost::condition_variable& cond)
  {
    ping = true;
    cond.notify_all();
  }

  static bool bool_identity(bool& b)
  {
    return b;
  }

  void EventLoopAsio::_pingThread()
  {
    qi::os::setCurrentThreadName("EvLoop.mon");
    static int msTimeout = getEnvParam("QI_EVENTLOOP_PING_TIMEOUT", 500);
    static int msGrace = getEnvParam("QI_EVENTLOOP_GRACE_PERIOD", 0);
    static int maxTimeouts = getEnvParam("QI_EVENTLOOP_MAX_TIMEOUTS", 20);
    ++_nThreads;
    boost::mutex mutex;
    boost::condition_variable cond;
    bool gotPong = false;
    unsigned int nbTimeout = 0;
    while (_work)
    {
      qiLogDebug() << "Ping";
      gotPong = false;
      post(0, boost::bind(&ping_me, boost::ref(gotPong), boost::ref(cond)));
      boost::mutex::scoped_lock l(mutex);
      if (!cond.timed_wait(l,
        boost::get_system_time()+ boost::posix_time::milliseconds(msTimeout),
        boost::bind(&bool_identity, boost::ref(gotPong))))
      {
        if (_maxThreads && *_nThreads >= _maxThreads + 1) // we count in nThreads
        {
          ++nbTimeout;
          qiLogInfo() << "Thread " << _name << " limit reached (" << nbTimeout << " timeouts)" << *_totalTask << " / " << _maxThreads << " active: " << *_activeTask;;

          if (nbTimeout >= maxTimeouts)
          {
            qiLogInfo() << "threadpool: " << _name <<
              ": System seems to be deadlocked, sending emergency signal";
            if (_emergencyCallback)
            {
              try {
                _emergencyCallback();
              } catch (...) {
              }
            }
          }
        }
        else
        {
          qiLogInfo() << _name << ": Spawning more threads (" << *_nThreads << ')';
          boost::thread(&EventLoopAsio::_runPool, this);
        }
        qi::os::msleep(msGrace);
      }
      else
      {
        nbTimeout = 0;
        qiLogDebug() << "Ping ok";
        qi::os::msleep(msTimeout);
      }
    }
    if (!--_nThreads)
      --_running;
  }

  void EventLoopAsio::_runPool()
  {
    qiLogDebug() << this << "run starting from pool";
    qi::os::setCurrentThreadName(_name);
    _running.setIfEquals(0, 1);
    ++_nThreads;
    try
    {
      _io.run();
    }
    catch(const std::exception& e)
    {
      qiLogWarning() << "Error caught in eventloop(" << _name << ").async: " << e.what();
    }
    catch(...)
    {}
    if (!--_nThreads)
      --_running;
  }

  void EventLoopAsio::run()
  {
    qiLogDebug() << this << "run starting";
    std::string dontusecpuaffinity = qi::os::getenv("QI_EVENTLOOP_NO_CPU_AFFINITY");
    if (dontusecpuaffinity.empty()) {
      std::vector<int> cpus;
      cpus.push_back(0);
      bool ret = qi::os::setCurrentThreadCPUAffinity(cpus);
      qiLogVerbose() << "AsioEventLoop: Set cpu thread affinity to " << 1 << " (" << ret <<")";
    } else {
      qiLogVerbose() << "AsioEventLoop: Cpu thread affinity not set because QI_EVENTLOOP_NO_CPU_AFFINITY is set.";
    }
    qi::os::setCurrentThreadName("asioeventloop");
    _running.setIfEquals(0,1);
    _id = boost::this_thread::get_id();
    _work = new boost::asio::io_service::work(_io);
    _io.run();
    bool destroyMe;
    {
      boost::recursive_mutex::scoped_lock sl(_mutex);
      --_running;
      destroyMe = _destroyMe;
    }
    if (destroyMe)
      delete this;
  }

  bool EventLoopAsio::isInEventLoopThread()
  {
    return boost::this_thread::get_id() == _id;
  }

  void EventLoopAsio::stop()
  {
    qiLogDebug() << "stopping eventloopasio: " << this;
    boost::recursive_mutex::scoped_lock sl(_mutex);
    if (_work)
    {
      delete _work;
      _work = 0;
    }
  }

  void EventLoopAsio::join()
  {
    if (_mode == Mode_Threaded)
    {
      if (boost::this_thread::get_id() == _id)
      {
        qiLogError() << "Cannot join from within event loop thread";
        return;
      }
      if (_thd.joinable())
      {
        try {
          _thd.join();
        }
        catch(const boost::thread_resource_error& e)
        {
          qiLogWarning() << "Join an already joined thread: " << e.what();
        }
        return;
      }
    }
    else
    {
      qiLogDebug() << "Waiting for threads to terminate...";
      while (*_running)
        qi::os::msleep(0);
      qiLogDebug()  << "Waiting done";
    }
  }

  class ScopedIncDec {
  public:
    ScopedIncDec(qi::Atomic<qi::uint32_t>& atom)
      : _atom(atom)
    {
      ++_atom;
    }

    ~ScopedIncDec() {
      --_atom;
    }

    qi::Atomic<qi::uint32_t>& _atom;
  };

  class ScopedExitDec {
  public:
    ScopedExitDec(qi::Atomic<qi::uint32_t>& atom)
      : _atom(atom)
    {
    }

    ~ScopedExitDec() {
      --_atom;
    }

    qi::Atomic<qi::uint32_t>& _atom;
  };


  void EventLoopAsio::invoke_maybe(boost::function<void()> f, qi::uint32_t id, qi::Promise<void> p, const boost::system::error_code& erc)
  {
    ScopedExitDec _(_totalTask);
    if (!erc)
    {
      ScopedIncDec _(_activeTask);
      tracepoint(qi_qi, eventloop_task_start, id);
      f();
      tracepoint(qi_qi, eventloop_task_stop, id);
      p.setValue(0);
    }
    else {
      tracepoint(qi_qi, eventloop_task_cancel, id);
      p.setCanceled();
    }
  }

  void EventLoopAsio::post(uint64_t usDelay, const boost::function<void ()>& cb)
  {
    static boost::system::error_code erc;
    qi::Promise<void> p;
    if (!usDelay) {
      uint32_t id = ++gTaskId;
      tracepoint(qi_qi, eventloop_post, id, cb.target_type().name());


      ++_totalTask;
      _io.post(boost::bind<void>(&EventLoopAsio::invoke_maybe, this, cb, id, p, erc));
    }
    else
      asyncCall(usDelay, cb);
  }

  qi::Future<void> EventLoopAsio::asyncCall(uint64_t usDelay, boost::function<void ()> cb)
  {
    if (!_work)
      return qi::makeFutureError<void>("Schedule attempt on destroyed thread pool");

    uint32_t id = ++gTaskId;

    ++_totalTask;
    tracepoint(qi_qi, eventloop_delay, id, cb.target_type().name(), usDelay);
    boost::shared_ptr<boost::asio::deadline_timer> timer = boost::make_shared<boost::asio::deadline_timer>(boost::ref(_io));
    timer->expires_from_now(boost::posix_time::microseconds(usDelay));
    qi::Promise<void> prom(boost::bind(&boost::asio::deadline_timer::cancel, timer));
    timer->async_wait(boost::bind(&EventLoopAsio::invoke_maybe, this, cb, id, prom, _1));
    return prom.future();
  }

  void EventLoopAsio::setMaxThreads(unsigned int max)
  {
    _maxThreads = max;
  }

  void* EventLoopAsio::nativeHandle()
  {
    return static_cast<void*>(&_io);
  }

  EventLoopThreadPool::EventLoopThreadPool(int minWorkers, int maxWorkers, int minIdleWorkers, int maxIdleWorkers)
  {
    qiLogDebug() << this << " EventLoopThreadPool CTOR";
    _stopping = false;
    _pool = new ThreadPool(minWorkers, maxWorkers, minIdleWorkers, maxIdleWorkers);
    qiLogDebug() << this << " EventLoopThreadPool CTOR done";
  }

  bool EventLoopThreadPool::isInEventLoopThread()
  {
    // The point is to know if a call will be synchronous. It never is
    // with thread pool
    return false;
  }

  void EventLoopThreadPool::start(int /*nthreads*/)
  {
    qiLogDebug() << this << " EventLoopThreadPool start (and done)";
  }

  void EventLoopThreadPool::run()
  {
  }

  void EventLoopThreadPool::join()
  {
    qiLogDebug() << this << " EventLoopThreadPool join";
    _pool->waitForAll();
    qiLogDebug() << this << " EventLoopThreadPool join done";
  }

  void EventLoopThreadPool::stop()
  {
    qiLogDebug() << this << " EventLoopThreadPool stop (and done)";
    _stopping = true;
    _pool->stop();
  }

  void* EventLoopThreadPool::nativeHandle()
  {
    return 0;
  }

  void EventLoopThreadPool::destroy()
  {
    _stopping = true;
    // Ensure delete is not called from one of the threads of the event loop
    boost::thread(&EventLoopThreadPool::_destroy, this);
  }
  void EventLoopThreadPool::_destroy()
  {
    delete this;
  }

  EventLoopThreadPool::~EventLoopThreadPool()
  {
    stop();
    join();
    delete _pool;
  }

  static void delay_call(uint64_t usDelay, boost::function<void()>* callback)
  {
    if (usDelay)
      qi::os::msleep(static_cast<unsigned int>(usDelay/1000));
    try
    {
      (*callback)();
    }
    catch(const std::exception& e)
    {
      qiLogError() << "Exception caught in async call: " << e.what();
    }
    catch(...)
    {
      qiLogError() << "Unknown exception caught in async call";
    }
    delete callback;
  }

  static void delay_call_notify(uint64_t usDelay, boost::function<void()> callback,
    qi::Promise<void> promise)
  {
    if (usDelay)
      qi::os::msleep(static_cast<unsigned int>(usDelay/1000));
    try
    {
      callback();
      promise.setValue(0);
    }
    catch(const std::exception& e)
    {
      promise.setError(std::string("Exception caught in async call: ")  + e.what());
    }
    catch(...)
    {
      promise.setError("Unknown exception caught in async call");
    }
  }

  void EventLoopThreadPool::post(uint64_t usDelay,
      const boost::function<void ()>& callback)
  {
    if (_stopping)
    {
      qiLogWarning() << "ThreadPool post() while stopping";
      return;
    }
    _pool->schedule(boost::bind(&delay_call, usDelay, new boost::function<void ()>(callback)));
  }

  qi::Future<void>  EventLoopThreadPool::asyncCall(uint64_t usDelay,
      boost::function<void ()> callback)
  {
    if (_stopping)
      return qi::makeFutureError<void>("Schedule attempt on destroyed thread pool");
    qi::Promise<void> promise;
    _pool->schedule(boost::bind(&delay_call_notify, usDelay, callback, promise));
    return promise.future();
  }

  void EventLoopThreadPool::setMaxThreads(unsigned int max)
  {
  }

  EventLoop::EventLoop(const std::string& name)
  : _p(0)
  , _name(name)
  {
  }

  EventLoop::~EventLoop()
  {
    if (_p)
      _p->destroy();
    _p = 0;
  }

  #define CHECK_STARTED                                                            \
  do {                                                                             \
    if (!_p)                                                                       \
      throw std::runtime_error("EventLoop " __HERE " : EventLoop not started");  \
  } while(0)


  bool EventLoop::isInEventLoopThread()
  {
    CHECK_STARTED;
    return _p->isInEventLoopThread();
  }

  void EventLoop::join()
  {
    qiLogDebug() << this << " EventLoop join";
    CHECK_STARTED;
    _p->join();
    qiLogDebug() << this << " EventLoop join done";
  }

  void EventLoop::start(int nthreads)
  {
    qiLogDebug() << this << " EventLoop start";
    if (_p)
      return;
    _p = new EventLoopAsio();
    _p->_name = _name;
    _p->start(nthreads);
    qiLogDebug() << this << " EventLoop start done";
  }

  void EventLoop::startThreadPool(int minWorkers, int maxWorkers, int minIdleWorkers, int maxIdleWorkers)
  {
    qiLogDebug() << this << " EventLoop startThreadPool";
    #define OR(name, val) (name==-1?val:name)
    if (_p)
      return;
    _p = new EventLoopThreadPool(OR(minWorkers, 2), OR(maxWorkers, 100), OR(minIdleWorkers,1), OR(maxIdleWorkers, 0));
    #undef OR
    qiLogDebug() << this << " EventLoop startThreadPool done";
  }


  void EventLoop::stop()
  {
    qiLogDebug() << this << " EventLoop stop";
    CHECK_STARTED;
    _p->stop();
    qiLogDebug() << this << " EventLoop stop done";
  }

  void EventLoop::run()
  {
    qiLogDebug() << this << " EventLoop run";
    if (_p)
      return;
    _p = new EventLoopAsio();
    _p->run();
    qiLogDebug() << this << " EventLoop run done";
  }

  void *EventLoop::nativeHandle() {
    CHECK_STARTED;
    return _p->nativeHandle();
  }

  void EventLoop::post(const boost::function<void ()>& callback,uint64_t usDelay)
  {
    qiLogDebug() << this << " EventLoop post " << &callback;
    CHECK_STARTED;
    _p->post(usDelay, callback);
    qiLogDebug() << this << " EventLoop post done " << &callback;
  }

  qi::Future<void>
  EventLoop::async(
    boost::function<void ()> callback,
    uint64_t usDelay)
  {
    CHECK_STARTED;
    return _p->asyncCall(usDelay, callback);
  }

  void EventLoop::setEmergencyCallback(boost::function<void()> cb)
  {
    if (!_p)
      throw std::runtime_error("call start before");
    _p->_emergencyCallback = cb;
  }

  void EventLoop::setMaxThreads(unsigned int max)
  {
    if (!_p)
      throw std::runtime_error("call start before");
    _p->setMaxThreads(max);
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
    //qiLogDebug("qi.EventLoop") << os::ustime() << " MON timeout " << ctx->isFired
    // << ' ' << (os::ustime() - ctx->startTime);
    if (!ctx->isFired)
      return; // Got the pong in the meantime, abort
    ctx->promise.setError("Event loop monitor timeout");
    /* Ping system is still on, but promise is set.
     * So future invocations of cancel() will be ignored, which makes the
     * monitoring unstopable.
     * So reset the value.
    */
    ctx->promise.reset();
  }

  static void monitor_cancel(qi::Promise<void>, boost::shared_ptr<MonitorContext> ctx)
  {
    //qiLogDebug("qi.EventLoop") << os::ustime() << " MON cancel " << ctx->isFired;
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
    //qiLogDebug("qi.EventLoop") << os::ustime() << " MON ping " << ctx->isFired;
    if (ctx->isFired)
    { // This is a pong
      ctx->isFired = false;
      // Cancel monitoring async call
      try {
        ctx->mon.cancel();
      }
      catch (const std::exception& /*e*/) {
        //qiLogDebug("qi.EventLoop") << "MON " << e.what();
      }
      uint64_t pingDelay = os::ustime() - ctx->startTime;
      if (pingDelay > ctx->maxDelay / 2)
        qiLogDebug() << "Long ping " << pingDelay;
      // Wait a bit before pinging againg
      //qiLogDebug("qi.EventLoop") << os::ustime() << " MON delay " << ctx->maxDelay;
      ctx->helper->async(boost::bind(&monitor_ping, ctx), ctx->maxDelay*5);
    }
    else
    { // Delay between pings reached, ping again
      ctx->startTime = os::ustime();
      ctx->isFired = true;
      // Start monitor async first, or the ping async can trigger before the
      // monitor async is setup
      ctx->mon = ctx->helper->async(boost::bind(&monitor_pingtimeout, ctx), ctx->maxDelay);
      ctx->target->post(boost::bind(&monitor_ping, ctx));
      assert(ctx->mon.isCancelable());
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
    ctx->stop();
    ctx->join();
    delete ctx;
    ctx = 0;
  }

  static EventLoop*    _poolEventLoop = 0;

  //the initialisation is protected by a mutex,
  //we then use an atomic to prevent having a mutex on a fastpath.
  static EventLoop* _get(EventLoop* &ctx, bool isPool, int nthreads)
  {
    //same mutex for multiples eventloops, but that's ok, used only at init.
    static boost::mutex    eventLoopMutex;
    static qi::Atomic<int> init(0);

    if (*init)
      return ctx;

    {
      boost::mutex::scoped_lock _sl(eventLoopMutex);
      if (!ctx)
      {
        if (!qi::Application::initialized())
        {
          qiLogVerbose() << "Creating event loop while no qi::Application() is running";
        }
        ctx = new EventLoop();
        if (isPool)
          ctx->startThreadPool(nthreads);
        else
          ctx->start(nthreads);
        Application::atExit(boost::bind(&eventloop_stop, boost::ref(ctx)));
      }
    }
    ++init;
    return ctx;
  }

  void startEventLoop(int nthread)
  {
    _get(_poolEventLoop, false, nthread);
  }

  EventLoop* getEventLoop()
  {
    return _get(_poolEventLoop, false, 0);
  }


  boost::asio::io_service& getIoService()
  {
    return *(boost::asio::io_service*)getEventLoop()->nativeHandle();
  }
}
