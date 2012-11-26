/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>

#include <boost/thread.hpp>

#include <qi/log.hpp>
#include <qi/application.hpp>

#include <qi/eventloop.hpp>

#include "eventloop_p.hpp"

namespace qi {

  EventLoopPrivate::EventLoopPrivate()
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


  void EventLoopPrivate::start()
  {
    if (_running || _threaded)
      return;
    _threaded = true;
    _thd = boost::thread(&EventLoopPrivate::run, this);
    while (!_running)
      qi::os::msleep(0);
  }

  EventLoopPrivate::~EventLoopPrivate()
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

  void EventLoopPrivate::destroy(bool join)
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

  void EventLoopPrivate::run()
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

  bool EventLoopPrivate::isInEventLoopThread()
  {
    return boost::this_thread::get_id() == _id;
  }

  void EventLoopPrivate::stop()
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

  void EventLoopPrivate::join()
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

  qi::Future<void> EventLoopPrivate::asyncCall(uint64_t usDelay, boost::function<void ()> cb)
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

  EventLoop::AsyncCallHandle EventLoopPrivate::notifyFd(evutil_socket_t fd, EventLoop::NotifyFdCallbackFunction cb, short evflags, bool persistant)
  {
    EventLoop::AsyncCallHandle res;
    if (!_base) {
      qiLogDebug("eventloop") << "Discarding notifyChange after loop destruction.";
      return res;
    }
    if (persistant)
      evflags |= EV_PERSIST;
    struct event *ev = event_new(_base,
                                 fd,
                                 evflags,
                                 fduse_call,
                                 new EventLoop::AsyncCallHandle(res));
    res._p->ev = ev;
    res._p->cancelled = false;
    res._p->persistant = persistant;
    std::swap(res._p->fdcallback,cb);
    event_add(ev, NULL);
    return res;
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
  {
    _p = new EventLoopPrivate();
  }

  EventLoop::~EventLoop()
  {
    _p->destroy(false);
    _p = 0;
  }

  bool EventLoop::isInEventLoopThread()
  {
    return _p->isInEventLoopThread();
  }

  void EventLoop::join()
  {
    _p->join();
  }

  void EventLoop::start()
  {
    _p->start();
  }

  void EventLoop::stop()
  {
    _p->stop();
  }

  void EventLoop::run()
  {
    _p->run();
  }

  void *EventLoop::nativeHandle() {
    return static_cast<void *>(_p->getEventBase());
  }

  qi::Future<void>
  EventLoop::asyncCall(
    uint64_t usDelay,
    boost::function<void ()> callback)
  {
    return _p->asyncCall(usDelay, callback);
  }

  EventLoop::AsyncCallHandle
  EventLoop::notifyFd(int fileDescriptor,
                      NotifyFdCallbackFunction callback,
                      FileOperation fdUsage)
  {
    switch (fdUsage)
    {
      case FileOperation_Read:
        return _p->notifyFd(static_cast<evutil_socket_t>(fileDescriptor), callback, EV_READ, true);
      case FileOperation_Write:
        return _p->notifyFd(static_cast<evutil_socket_t>(fileDescriptor), callback, EV_WRITE, true);
      case FileOperation_ReadOrWrite:
        return _p->notifyFd(static_cast<evutil_socket_t>(fileDescriptor), callback, EV_READ|EV_WRITE, true);
    }
    return _p->notifyFd(static_cast<evutil_socket_t>(fileDescriptor), callback, EV_READ|EV_WRITE, true);
  }

  static void eventloop_stop(EventLoop* ctx)
  {
    ctx->stop();
    ctx->join();
    delete ctx;
  }

  static EventLoop* _get(EventLoop* &ctx)
  {
    if (!ctx)
    {
      if (! qi::Application::initialized())
      {
        qiLogInfo("EventLoop") << "Creating event loop while no qi::Application() is running";
      }
      ctx = new EventLoop();
      ctx->start();
      Application::atExit(boost::bind(&eventloop_stop, ctx));
    }
    return ctx;
  }

  static EventLoop* _netEventLoop = 0;
  static EventLoop* _objEventLoop = 0;
  EventLoop* getDefaultNetworkEventLoop()
  {
    return _get(_netEventLoop);
  }

  EventLoop* getDefaultObjectEventLoop()
  {
    return _get(_objEventLoop);
  }

}
