/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/thread.hpp>
#include <boost/program_options.hpp>

#include <qi/preproc.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qi/threadpool.hpp>

#include <qi/eventloop.hpp>

#include "eventloop_p.hpp"

namespace qi {

  EventLoopAsio::EventLoopAsio()
  : _destroyMe(false)
  , _running(false)
  , _threaded(false)
  {
  }


  void EventLoopAsio::start()
  {
    if (_running || _threaded)
      return;
    _threaded = true;
    _thd = boost::thread(&EventLoopPrivate::run, this);
    while (!_running)
      qi::os::msleep(0);
  }

  EventLoopAsio::~EventLoopAsio()
  {
    if (_running && boost::this_thread::get_id() != _id)
      qiLogError("Destroying EventLoopPrivate from itself while running");
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

  void EventLoopAsio::run()
  {
    qiLogDebug("qi.EventLoop") << this << "run starting";
    _running = true;
    _id = boost::this_thread::get_id();
    _work = new boost::asio::io_service::work(_io);
    _io.run();
    bool destroyMe;
    {
      boost::recursive_mutex::scoped_lock sl(_mutex);
      _running = false;
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
    qiLogDebug("qi.EventLoop") << this << "stopping";
    boost::recursive_mutex::scoped_lock sl(_mutex);
    if (_work)
    {
      delete _work;
      _work = 0;
    }
  }

  void EventLoopAsio::join()
  {
    if (boost::this_thread::get_id() == _id)
    {
      qiLogError("qi.EventLoop") << "Cannot join from within event loop thread";
      return;
    }
    if (_threaded)
      try {
        _thd.join();
      }
      catch(const boost::thread_resource_error& e)
      {
        qiLogWarning("qi.EventLoop") << "Join an already joined thread: " << e.what();
      }
    else
      while (_running)
        qi::os::msleep(0);
  }

  void EventLoopAsio::post(uint64_t usDelay, const boost::function<void ()>& cb)
  {
    if (!usDelay)
      _io.post(cb);
    else
      asyncCall(usDelay, cb);
  }

  static void invoke_maybe(boost::function<void()> f, qi::Promise<void> p, const boost::system::error_code& erc)
  {
    if (!erc)
    {
      f();
      p.setValue(0);
    }
    else
      p.setError("Operation cancelled");
  }

  qi::Future<void> EventLoopAsio::asyncCall(uint64_t usDelay, boost::function<void ()> cb)
  {
    boost::shared_ptr<boost::asio::deadline_timer> timer(new boost::asio::deadline_timer(_io));
    timer->expires_from_now(boost::posix_time::microseconds(usDelay));
    qi::Promise<void> prom(boost::bind(&boost::asio::deadline_timer::cancel, timer));
    timer->async_wait(boost::bind(&invoke_maybe, cb, prom, _1));
    return prom.future();
  }


  void* EventLoopAsio::nativeHandle()
  {
    return static_cast<void*>(&_io);
  }

  EventLoopThreadPool::EventLoopThreadPool(int minWorkers, int maxWorkers, int minIdleWorkers, int maxIdleWorkers)
  {
    _stopping = false;
    _pool = new ThreadPool(minWorkers, maxWorkers, minIdleWorkers, maxIdleWorkers);
  }

  bool EventLoopThreadPool::isInEventLoopThread()
  {
    // The point is to know if a call will be synchronous. It never is
    // with thread pool
    return false;
  }

  void EventLoopThreadPool::start()
  {
  }

  void EventLoopThreadPool::run()
  {
  }

  void EventLoopThreadPool::join()
  {
    _pool->waitForAll();
  }

  void EventLoopThreadPool::stop()
  {
    _stopping = true;
  }

  void* EventLoopThreadPool::nativeHandle()
  {
    return 0;
  }

  void EventLoopThreadPool::destroy()
  {
    _stopping = true;
    // Ensure delete is not called from one of the threads of the event loop
    boost::thread(&EventLoopThreadPool::destroy, this);
  }

  EventLoopThreadPool::~EventLoopThreadPool()
  {
    delete _pool;
  }

  static void delay_call(uint64_t usDelay, boost::function<void()> callback)
  {
    if (usDelay)
      qi::os::msleep(static_cast<unsigned int>(usDelay/1000));
    try
    {
      callback();
    }
    catch(const std::exception& e)
    {
      qiLogError("qi.EventLoop") << "Exception caught in async call: " << e.what();
    }
    catch(...)
    {
      qiLogError("qi.EventLoop") << "Unknown exception caught in async call";
    }
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
    _pool->schedule(boost::bind(&delay_call, usDelay, callback));
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

   // Basic pimpl bouncers.
  EventLoop::AsyncCallHandle::AsyncCallHandle()
  {
    _p = boost::shared_ptr<AsyncCallHandlePrivate>(new AsyncCallHandlePrivate());
  }

  EventLoop::AsyncCallHandle::~AsyncCallHandle()
  {
  }

  void EventLoop::AsyncCallHandle::cancel()
  {
    _p->cancel();
  }

  EventLoop::EventLoop()
  : _p(0)
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
    CHECK_STARTED;
    _p->join();
  }

  void EventLoop::start()
  {
    if (_p)
      return;
    _p = new EventLoopAsio();
    _p->start();
  }

  void EventLoop::startThreadPool(int minWorkers, int maxWorkers, int minIdleWorkers, int maxIdleWorkers)
  {
    #define OR(name, val) (name==-1?val:name)
    if (_p)
      return;
    _p = new EventLoopThreadPool(OR(minWorkers, 2), OR(maxWorkers, 8), OR(minIdleWorkers,1), OR(maxIdleWorkers, 4));
    #undef OR
  }


  void EventLoop::stop()
  {
    CHECK_STARTED;
    _p->stop();
  }

  void EventLoop::run()
  {
    if (_p)
      return;
    _p = new EventLoopAsio();
    _p->run();
  }

  void *EventLoop::nativeHandle() {
    CHECK_STARTED;
    return _p->nativeHandle();
  }

  void EventLoop::post(const boost::function<void ()>& callback,uint64_t usDelay)
  {
    CHECK_STARTED;
    _p->post(usDelay, callback);
  }

  qi::Future<void>
  EventLoop::async(
    boost::function<void ()> callback,
    uint64_t usDelay)
  {
    CHECK_STARTED;
    return _p->asyncCall(usDelay, callback);
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

  static void monitor_cancel(boost::shared_ptr<MonitorContext> ctx)
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
        qiLogDebug("qi.EventLoop") << "Long ping " << pingDelay;
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
      assert(ctx->mon.isCanceleable());
    }
  }

  qi::Future<void> EventLoop::monitorEventLoop(EventLoop* helper, uint64_t maxDelay)
  {
    // Context data is a Future*[2]
    boost::shared_ptr<MonitorContext> ctx(new MonitorContext);
    ctx->target = this;
    ctx->helper = helper;
    ctx->maxDelay = maxDelay;
    ctx->promise = Promise<void>(boost::bind(&monitor_cancel, ctx));
    ctx->isFired = false;
    ctx->ending = false;
    monitor_ping(ctx);
    return ctx->promise.future();
  }

  static void eventloop_stop(EventLoop* ctx)
  {
    ctx->stop();
    delete ctx;
  }

  static EventLoop* _netEventLoop = 0;
  static EventLoop* _objEventLoop = 0;
  static EventLoop* _poolEventLoop = 0;
  static double _monitorInterval = 0;

  static void monitor_notify(const char* which)
  {
    qiLogError("qi.EventLoop") << which << " event loop stuck?";
  }
  static EventLoop* _get(EventLoop* &ctx, bool isPool)
  {
    if (!ctx)
    {
      if (! qi::Application::initialized())
      {
        qiLogInfo("EventLoop") << "Creating event loop while no qi::Application() is running";
      }
      ctx = new EventLoop();
      if (isPool)
        ctx->startThreadPool();
      else
        ctx->start();
      Application::atExit(boost::bind(&eventloop_stop, ctx));
      if (!isPool && _netEventLoop && _objEventLoop && _monitorInterval)
      {
        int64_t d = static_cast<qi::int64_t>(_monitorInterval * 1e6);
        _netEventLoop->monitorEventLoop(_objEventLoop, d)
          .connect(boost::bind(&monitor_notify, "network"));
        _objEventLoop->monitorEventLoop(_netEventLoop, d)
          .connect(boost::bind(&monitor_notify, "object"));
      }
    }
    return ctx;
  }



  EventLoop* getDefaultNetworkEventLoop()
  {
    return _get(_netEventLoop, false);
  }

  EventLoop* getDefaultObjectEventLoop()
  {
    return _get(_objEventLoop, false);
  }

  EventLoop* getDefaultThreadPoolEventLoop()
  {
    return _get(_poolEventLoop, true);
  }
  static void setMonitorInterval(double v)
  {
    _monitorInterval = v;
  }
  namespace {
  _QI_COMMAND_LINE_OPTIONS(
    "EventLoop monitoring",
    ("loop-monitor-latency", value<double>()->notifier(&setMonitorInterval), "Warn if event loop is stuck more than given duration in seconds")
    )
  }
}
