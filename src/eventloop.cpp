/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio/steady_timer.hpp>

#include <qi/preproc.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>

#include <qi/eventloop.hpp>
#include <qi/future.hpp>

#include "eventloop_p.hpp"
#include "tp_qi.h"

qiLogCategory("qi.eventloop");

namespace qi {

  typedef boost::asio::basic_waitable_timer<SteadyClock> SteadyTimer;

  static qi::Atomic<uint32_t> gTaskId = 0;

  EventLoopAsio::EventLoopAsio()
  : _mode(Mode_Unset)
  , _work(NULL)
  , _maxThreads(0)
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
    _maxThreads = qi::os::getEnvDefault("QI_EVENTLOOP_MAX_THREADS", 150);
    _mode = Mode_Pooled;
    _work = new boost::asio::io_service::work(_io);
    for (int i=0; i<nthread; ++i)
      boost::thread(&EventLoopAsio::_runPool, this);
    boost::thread(&EventLoopAsio::_pingThread, this);
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
    static int msTimeout = qi::os::getEnvDefault("QI_EVENTLOOP_PING_TIMEOUT", 500);
    static int msGrace = qi::os::getEnvDefault("QI_EVENTLOOP_GRACE_PERIOD", 0);
    static int maxTimeouts = qi::os::getEnvDefault("QI_EVENTLOOP_MAX_TIMEOUTS", 20);
    ++_nThreads;
    boost::mutex mutex;
    boost::condition_variable cond;
    bool gotPong = false;
    unsigned int nbTimeout = 0;
    while (_work)
    {
      qiLogDebug() << "Ping";
      gotPong = false;
      post(qi::Seconds(0), boost::bind(&ping_me, boost::ref(gotPong), boost::ref(cond)));
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

    while (true) {
      try
      {
        _io.run();
        //the handler finished by himself. just quit.
        break;
      } catch(const detail::TerminateThread& e) {
        break;
      } catch(const std::exception& e) {
        qiLogWarning() << "Error caught in eventloop(" << _name << ").async: " << e.what();
      } catch(...) {
        qiLogWarning() << "Uncaught exception in eventloop(" << _name << ")";
      }
    }
    if (!--_nThreads)
      --_running;
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

  void EventLoopAsio::post(qi::Duration delay,
      const boost::function<void ()>& cb)
  {
    static boost::system::error_code erc;
    qi::Promise<void> p;
    if (delay == qi::Duration(0)) {
      uint32_t id = ++gTaskId;
      tracepoint(qi_qi, eventloop_post, id, cb.target_type().name());


      ++_totalTask;
      _io.post(boost::bind<void>(&EventLoopAsio::invoke_maybe, this, cb, id, p, erc));
    }
    else
      asyncCall(delay, cb);
  }

  qi::Future<void> EventLoopAsio::asyncCall(qi::Duration delay,
      boost::function<void ()> cb)
  {
    if (!_work)
      return qi::makeFutureError<void>("Schedule attempt on destroyed thread pool");

    uint32_t id = ++gTaskId;

    ++_totalTask;
    tracepoint(qi_qi, eventloop_delay, id, cb.target_type().name(), boost::chrono::duration_cast<qi::MicroSeconds>(delay).count());
    boost::shared_ptr<boost::asio::steady_timer> timer = boost::make_shared<boost::asio::steady_timer>(boost::ref(_io));
    timer->expires_from_now(delay);
    qi::Promise<void> prom(boost::bind(&boost::asio::steady_timer::cancel, timer));
    timer->async_wait(boost::bind(&EventLoopAsio::invoke_maybe, this, cb, id, prom, _1));
    return prom.future();
  }

  void EventLoopAsio::post(qi::SteadyClockTimePoint timepoint,
      const boost::function<void ()>& cb)
  {
    static boost::system::error_code erc;
    qi::Promise<void> p;
    asyncCall(timepoint, cb);
  }

  qi::Future<void> EventLoopAsio::asyncCall(qi::SteadyClockTimePoint timepoint,
      boost::function<void ()> cb)
  {
    if (!_work)
      return qi::makeFutureError<void>("Schedule attempt on destroyed thread pool");

    uint32_t id = ++gTaskId;

    ++_totalTask;
    //tracepoint(qi_qi, eventloop_delay, id, cb.target_type().name(), qi::MicroSeconds(delay).count());
    boost::shared_ptr<SteadyTimer> timer = boost::make_shared<SteadyTimer>(boost::ref(_io));
    timer->expires_at(timepoint);
    qi::Promise<void> prom(boost::bind(&SteadyTimer::cancel, timer));
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

  void EventLoop::stop()
  {
    qiLogDebug() << this << " EventLoop stop";
    CHECK_STARTED;
    _p->stop();
    qiLogDebug() << this << " EventLoop stop done";
  }

  void *EventLoop::nativeHandle() {
    CHECK_STARTED;
    return _p->nativeHandle();
  }

  void EventLoop::post(const boost::function<void ()>& callback,
      uint64_t usDelay)
  {
    post(callback, qi::MicroSeconds(usDelay));
  }

  void EventLoop::post(const boost::function<void ()>& callback,
      qi::Duration delay)
  {
    qiLogDebug() << this << " EventLoop post " << &callback;
    CHECK_STARTED;
    _p->post(delay, callback);
    qiLogDebug() << this << " EventLoop post done " << &callback;
  }

  void EventLoop::post(const boost::function<void ()>& callback,
      qi::SteadyClockTimePoint timepoint)
  {
    qiLogDebug() << this << " EventLoop post " << &callback;
    CHECK_STARTED;
    _p->post(timepoint, callback);
    qiLogDebug() << this << " EventLoop post done " << &callback;
  }

  qi::Future<void>
  EventLoop::async(
    boost::function<void ()> callback,
    uint64_t usDelay)
  {
    return async(callback, qi::MicroSeconds(usDelay));
  }

  qi::Future<void>
  EventLoop::async(
    boost::function<void ()> callback,
    qi::Duration delay)
  {
    CHECK_STARTED;
    return _p->asyncCall(delay, callback);
  }

  qi::Future<void>
  EventLoop::async(
    boost::function<void ()> callback,
    qi::SteadyClockTimePoint timepoint)
  {
    CHECK_STARTED;
    return _p->asyncCall(timepoint, callback);
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
  static EventLoop* _get(EventLoop* &ctx, int nthreads)
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
        ctx->start(nthreads);
        Application::atExit(boost::bind(&eventloop_stop, boost::ref(ctx)));
      }
    }
    ++init;
    return ctx;
  }

  void startEventLoop(int nthread)
  {
    _get(_poolEventLoop, nthread);
  }

  EventLoop* getEventLoop()
  {
    return _get(_poolEventLoop, 0);
  }


  boost::asio::io_service& getIoService()
  {
    return *(boost::asio::io_service*)getEventLoop()->nativeHandle();
  }
}
