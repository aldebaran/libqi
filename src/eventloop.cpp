/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>

#include <boost/thread.hpp>

#include <qi/preproc.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qi/threadpool.hpp>

#include <qi/eventloop.hpp>

#include "eventloop_p.hpp"

namespace qi {

  EventLoopLibEvent::EventLoopLibEvent()
  : _destroyMe(false)
  , _running(false)
  , _threaded(false)
  {
    static bool libevent_init = false;


  #ifdef _WIN32
      // libevent does not call WSAStartup
      WSADATA WSAData;
      // TODO: handle return code
      ::WSAStartup(MAKEWORD(1, 0), &WSAData);
  #endif

    if (!libevent_init)
    {
  #ifdef EVTHREAD_USE_WINDOWS_THREADS_IMPLEMENTED
      evthread_use_windows_threads();
  #endif
  #ifdef EVTHREAD_USE_PTHREADS_IMPLEMENTED
      evthread_use_pthreads();
  #endif
      libevent_init = !libevent_init;
    }

    if (!(_base = event_base_new()))
      return;
  }


  void EventLoopLibEvent::start()
  {
    if (_running || _threaded)
      return;
    _threaded = true;
    _thd = boost::thread(&EventLoopPrivate::run, this);
    while (!_running)
      qi::os::msleep(0);
  }

  EventLoopLibEvent::~EventLoopLibEvent()
  {
    if (_running && boost::this_thread::get_id() != _id)
      qiLogError("Destroying EventLoopPrivate from itself while running");
    stop();
    join();

  #ifdef _WIN32
    // TODO handle return code
    ::WSACleanup();
  #endif
  }

  void EventLoopLibEvent::destroy(bool join)
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

  void EventLoopLibEvent::run()
  {
    qiLogDebug("qi.EventLoop") << this << "run starting";
    _running = true;
    _id = boost::this_thread::get_id();
    // stop will set _base to 0 to delect usage attempts after stop
    // so use a local copy.
    event_base* base = _base;
    // libevent needs a dummy op to perform otherwise dispatch exits immediately
    struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
    //bufferevent_setcb(bev, NULL, NULL, ::qi::errorcb, this);
    bufferevent_enable(bev, EV_READ|EV_WRITE);

    event_base_dispatch(base);
    qiLogDebug("qi.EventLoop") << this << "run ending";
    bufferevent_free(bev);
    event_base_free(base);
    _base = 0;
    {
      boost::recursive_mutex::scoped_lock sl(_mutex);
      _running = false;
      if (_destroyMe)
        delete this;
    }
  }

  bool EventLoopLibEvent::isInEventLoopThread()
  {
    return boost::this_thread::get_id() == _id;
  }

  void EventLoopLibEvent::stop()
  {
    qiLogDebug("qi.EventLoop") << this << "stopping";
    event_base* base = 0;
    { // Ensure no multipe calls are made.
      boost::recursive_mutex::scoped_lock sl(_mutex);
      if (_base && _running)
      {
        base = _base;
        _base = 0;
      }
    }
    if (base)
      if (event_base_loopexit(base, NULL) != 0)
        qiLogError("networkThread") << "Can't stop the EventLoopPrivate";

  }

  void EventLoopLibEvent::join()
  {
    if (_base)
      qiLogError("EventLoop") << "join() called before stop()";
    if (boost::this_thread::get_id() == _id)
      return;
    if (_threaded)
      _thd.join();
    else
      while (_running)
        qi::os::msleep(0);
  }

  struct AsyncCallHandle
  {
    bool              cancelled;
    event*            ev;
    qi::Promise<void>  *result;
    boost::function<void()> callback;
  };

  static void post(evutil_socket_t,
    short what,
    void *context)
  {
    boost::function<void()>* cb = (boost::function<void()>*)context;
    (*cb)();
    delete cb;
  }

  static void async_call(evutil_socket_t,
    short what,
    void *context)
  {
    boost::shared_ptr<AsyncCallHandle>* handlePtr
      = (boost::shared_ptr<AsyncCallHandle>*)context;
    AsyncCallHandle* handle = handlePtr->get();
    assert(handle->result);
    event_del(handle->ev);
    if (!handle->cancelled)
    {
      handle->callback();
      handle->result->setValue(0);
    }
    else
      handle->result->setError("Cancelled");

    event_free(handle->ev);
    delete handle->result;
    handle->result = 0;
    delete handlePtr;
  }

  static void fduse_call(evutil_socket_t socket,
    short what,
    void *context)
  {
    EventLoop::AsyncCallHandle* handle = (EventLoop::AsyncCallHandle*)context;
    if (!handle->_p->cancelled) {
      if (what & EV_READ)
        handle->_p->fdcallback(socket, EventLoop::FileOperation_Read);
      else if (what & EV_WRITE)
        handle->_p->fdcallback(socket, EventLoop::FileOperation_Write);
      /// If this event is persistant and was not cancelled, do not delete.
      if (handle->_p->persistant)
        return;
    }
    event_del(handle->_p->ev);
    event_free(handle->_p->ev);
    delete handle;
  }

  void async_call_cancel(boost::shared_ptr<AsyncCallHandle> handle)
  {
    handle->cancelled = true;
  }

  void EventLoopLibEvent::post(uint64_t usDelay, boost::function<void ()> cb)
  {
    if (!_base) {
      qiLogDebug("eventloop") << "Discarding asyncCall after loop destruction.";
      return;
    }
    struct timeval period;
    period.tv_sec = static_cast<long>(usDelay / 1000000ULL);
    period.tv_usec = usDelay % 1000000ULL;
    struct event *ev = event_new(_base, -1, 0, &::qi::post,
      new boost::function<void()>(cb));
    event_add(ev, &period);
  }

  qi::Future<void> EventLoopLibEvent::asyncCall(uint64_t usDelay, boost::function<void ()> cb)
  {
    if (!_base) {
      qiLogDebug("eventloop") << "Discarding asyncCall after loop destruction.";
      return makeFutureError<void>("EventLoop destroyed");
    }
    boost::shared_ptr<AsyncCallHandle> h(new AsyncCallHandle());
    /* The future will hold the AsyncCallHandle, bound in onCancel.
      The future is also in the AsyncCallHandle
      The AsyncCallHandle is passed by pointer to eventfd callback.
      Once invoked, it will reset the future in AsyncCallHandle.
      Then the last future will delete onCancel which will clear bound data
    */
    struct timeval period;
    period.tv_sec = static_cast<long>(usDelay / 1000000ULL);
    period.tv_usec = usDelay % 1000000ULL;
    struct event *ev = event_new(_base, -1, 0, async_call,
      new boost::shared_ptr<AsyncCallHandle>(h));
    // Order is important.
    h->ev = ev;
    h->cancelled = false;
    std::swap(h->callback,cb);
    qi::Promise<void> prom(boost::bind(&async_call_cancel, h));
    h->result = new qi::Promise<void>(prom);
    event_add(ev, &period);
    return prom.future();
  }

  EventLoop::AsyncCallHandle EventLoopLibEvent::notifyFd(int fd_, EventLoop::NotifyFdCallbackFunction cb,
    EventLoop::FileOperation fdUsage)
  {
    evutil_socket_t fd =  static_cast<evutil_socket_t>(fd_);
    short evflags = 0;
    switch (fdUsage)
    {
    case EventLoop::FileOperation_Read: evflags = EV_READ; break;
    case EventLoop::FileOperation_Write: evflags = EV_WRITE; break;
    case EventLoop::FileOperation_ReadOrWrite: evflags = EV_READ | EV_WRITE; break;
    }

    EventLoop::AsyncCallHandle res;
    if (!_base) {
      qiLogDebug("eventloop") << "Discarding notifyChange after loop destruction.";
      return res;
    }
    evflags |= EV_PERSIST;
    struct event *ev = event_new(_base,
                                 fd,
                                 evflags,
                                 fduse_call,
                                 new EventLoop::AsyncCallHandle(res));
    res._p->ev = ev;
    res._p->cancelled = false;
    res._p->persistant = true;
    std::swap(res._p->fdcallback,cb);
    event_add(ev, NULL);
    return res;
  }

  void* EventLoopLibEvent::nativeHandle()
  {
    return static_cast<void*>(_base);
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
      boost::function<void ()> callback)
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
    _p = new EventLoopLibEvent();
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
    _p = new EventLoopLibEvent();
    _p->run();
  }

  void *EventLoop::nativeHandle() {
    CHECK_STARTED;
    return _p->nativeHandle();
  }

  void EventLoop::post(uint64_t usDelay, boost::function<void ()> callback)
  {
    CHECK_STARTED;
    _p->post(usDelay, callback);
  }

  qi::Future<void>
  EventLoop::asyncCall(
    uint64_t usDelay,
    boost::function<void ()> callback)
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
