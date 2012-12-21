/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/thread.hpp>

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

  void EventLoopAsio::destroy(bool join)
  {
    bool needJoin;
    bool needDelete;
    {
      boost::recursive_mutex::scoped_lock sl(_mutex);
      needJoin = join && _running && (boost::this_thread::get_id() != _id);
      needDelete = needJoin || !_running;
      _destroyMe = !needDelete;
    }
    stop(); // Deadlock if called within the scoped_lock
    if (needJoin)
      this->join();

    if (needDelete)
      delete this;
  }

  void EventLoopAsio::run()
  {
    qiLogDebug("qi.EventLoop") << this << "run starting";
    _running = true;
    _id = boost::this_thread::get_id();
    boost::asio::io_service::work worker(_io);
    _io.run();
    {
      boost::recursive_mutex::scoped_lock sl(_mutex);
      _running = false;
      if (_destroyMe)
        delete this;
    }
  }

  bool EventLoopAsio::isInEventLoopThread()
  {
    return boost::this_thread::get_id() == _id;
  }

  void EventLoopAsio::stop()
  {
    qiLogDebug("qi.EventLoop") << this << "stopping";
    _io.stop();
  }

  void EventLoopAsio::join()
  {
    _io.stop();
    if (boost::this_thread::get_id() == _id)
      return;
    if (_threaded)
      _thd.join();
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

  void notify_fd_bounce(
    const boost::system::error_code& erc,
    EventLoop::FileOperation op, EventLoop::AsyncCallHandle h
    )
  {
    if (!h._p->sd)
      return; // Other callback destroyed the socket
    if (h._p->cancelled && erc)
    {
      delete h._p->sd;
      h._p->sd = 0;
      return;
    }
    h._p->fdcallback(h._p->sd->native_handle(), op);
    if (op == EventLoop::FileOperation_Read)
    {
      h._p->sd->async_read_some(
          boost::asio::null_buffers(),
          boost::bind(&notify_fd_bounce, _1, EventLoop::FileOperation_Read, h)
          );
    }
    else if (op == EventLoop::FileOperation_Write)
    {
       h._p->sd->async_write_some(
          boost::asio::null_buffers(),
          boost::bind(&notify_fd_bounce, _1, EventLoop::FileOperation_Write, h)
          );
    }
  }
  EventLoop::AsyncCallHandle EventLoopAsio::notifyFd(int fd_, EventLoop::NotifyFdCallbackFunction cb,
    EventLoop::FileOperation fdUsage)
  {
    // A posix fd should be able to handle everything but regular files,
    // for which fd notification does not make sense anyway.
    EventLoop::AsyncCallHandle res;
    boost::asio::posix::stream_descriptor* sd = new boost::asio::posix::stream_descriptor(_io);
    res._p->sd = sd;
    res._p->fdcallback = cb;
    sd->assign(fd_);
    if (fdUsage & EventLoop::FileOperation_Read)
    {
      sd->async_read_some(
          boost::asio::null_buffers(),
          boost::bind(&notify_fd_bounce, _1, EventLoop::FileOperation_Read, res)
          );
    }
    if (fdUsage & EventLoop::FileOperation_Write)
    {
       sd->async_write_some(
          boost::asio::null_buffers(),
          boost::bind(&notify_fd_bounce, _1, EventLoop::FileOperation_Write, res)
          );
    }
    return res;
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

  void EventLoopThreadPool::destroy(bool join)
  {
    _stopping = true;
    if (join)
    {
      _pool->waitForAll();
      delete this;
    }
    else
      boost::thread(&EventLoopThreadPool::destroy, this, true);
  }

  EventLoopThreadPool::~EventLoopThreadPool()
  {
    delete _pool;
  }

  EventLoop::AsyncCallHandle EventLoopThreadPool::notifyFd(int fd,
      EventLoop::NotifyFdCallbackFunction cb, EventLoop::FileOperation fdUsage)
  {
    throw std::runtime_error("notifyFd not implemented for thread pool");
  }

  static void delay_call(uint64_t usDelay, boost::function<void()> callback)
  {
    if (usDelay)
      qi::os::msleep(usDelay/1000);
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
      qi::os::msleep(usDelay/1000);
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
      _p->destroy(false);
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

  EventLoop::AsyncCallHandle
  EventLoop::notifyFd(int fileDescriptor,
                      NotifyFdCallbackFunction callback,
                      FileOperation fdUsage)
  {
    CHECK_STARTED;
    return _p->notifyFd(fileDescriptor, callback, fdUsage);
  }


  static void eventloop_stop(EventLoop* ctx)
  {
    ctx->stop();
    ctx->join();
    delete ctx;
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
    }
    return ctx;
  }

  static EventLoop* _netEventLoop = 0;
  static EventLoop* _objEventLoop = 0;
  static EventLoop* _poolEventLoop = 0;

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

}
