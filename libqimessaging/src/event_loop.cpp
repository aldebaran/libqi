/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>

#include <boost/thread.hpp>

#include <qi/log.hpp>
#include <qi/application.hpp>

#include <qimessaging/event_loop.hpp>

#include "src/event_loop_p.hpp"

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

  static void async_call(evutil_socket_t,
    short what,
    void *context)
  {
    EventLoop::AsyncCallHandle* handle = (EventLoop::AsyncCallHandle*)context;
    if (!handle->_p->cancelled)
      handle->_p->callback();
    event_del(handle->_p->ev);
    event_free(handle->_p->ev);
    delete handle;
  }

  EventLoop::AsyncCallHandle EventLoopPrivate::asyncCall(uint64_t usDelay, boost::function<void ()> cb)
  {
    EventLoop::AsyncCallHandle res;
    if (!_base) {
      qiLogDebug("eventloop") << "Discarding asyncCall after loop destruction.";
      return res;
    }
    struct timeval period;
    period.tv_sec = usDelay / 1000000ULL;
    period.tv_usec = usDelay % 1000000ULL;
    struct event *ev = event_new(_base, -1, 0, async_call,
      new EventLoop::AsyncCallHandle(res));
    // Order is important.
    res._p->ev = ev;
    res._p->cancelled = false;
    res._p->callback = cb;
    event_add(ev, &period);
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

  EventLoop::AsyncCallHandle
  EventLoop::asyncCall(
    uint64_t usDelay,
    boost::function<void ()> callback)
  {
    return _p->asyncCall(usDelay, callback);
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
        qiLogError("EventLoop") << "EventLoop created before qi::Application()";
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
