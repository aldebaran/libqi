/*
** network-thread.cpp
** Login : <hcuche@hcuche-de>
** Started on  Tue Jan 10 11:41:39 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#include <iostream>
#include <event2/thread.h>
#include <qi/log.hpp>

#include "src/network_thread.hpp"

namespace qi {

static void errorcb(struct bufferevent *QI_UNUSED(bev),
                    short               error,
                    void               *QI_UNUSED(ctx))
{
  if (error & BEV_EVENT_EOF)
  {
    // connection has been closed, do any clean up here
    qiLogError("qimessaging.TransportSocket") << "connection has been closed, do any clean up here" << std::endl;
  }
  else if (error & BEV_EVENT_ERROR)
  {
    // check errno to see what error occurred
    qiLogError("qimessaging.TransportSocket")  << "check errno to see what error occurred" << std::endl;
  }
  else if (error & BEV_EVENT_TIMEOUT)
  {
    // must be a timeout event handle, handle it
    qiLogError("qimessaging.TransportSocket")  << "must be a timeout event handle, handle it" << std::endl;
  }
}

NetworkThread::NetworkThread()
: _destroyMe(false)
, _running(true)
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
  _thd = boost::thread(&NetworkThread::run, this);
}

NetworkThread::~NetworkThread()
{
  if (_running && boost::this_thread::get_id() != _thd.get_id())
    qiLogError("Destroying NetworkThread from itself while running");
  stop();
  join();

#ifdef _WIN32
  // TODO handle return code
  ::WSACleanup();
#endif
}

void NetworkThread::destroy(bool join)
{
  bool needJoin;
  bool needDelete;
  {
    boost::recursive_mutex::scoped_lock sl(_mutex);
    needJoin = join && _running && (boost::this_thread::get_id() != _thd.get_id());
    needDelete = needJoin || !_running;
    _destroyMe = !needDelete;
  }
  stop(); // Deadlock if called within the scoped_lock
  if (needJoin)
    _thd.join();
  if (needDelete)
    delete this;
}

void NetworkThread::run()
{
  // stop will set _base to 0 to delect usage attempts after stop
  // so use a local copy.
  event_base* base = _base;
  struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
  bufferevent_setcb(bev, NULL, NULL, ::qi::errorcb, this);
  bufferevent_enable(bev, EV_READ|EV_WRITE);

  event_base_dispatch(base);
  bufferevent_free(bev);
  event_base_free(base);
  {
    boost::recursive_mutex::scoped_lock sl(_mutex);
    _running = false;
    if (_destroyMe)
      delete this; // I assume this is safe even if someone is joining us.
  }
}

bool NetworkThread::isInNetworkThread()
{
  return boost::this_thread::get_id() == _thd.get_id();
}

void NetworkThread::stop()
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
      qiLogError("networkThread") << "Can't stop the NetworkThread";

}

void NetworkThread::join()
{
  if (boost::this_thread::get_id() != _thd.get_id())
    _thd.join();
}

static void async_call(evutil_socket_t,
  short what,
  void *context)
{
  boost::shared_ptr<NetworkThread::AsyncCallHandler>* handler =
  (boost::shared_ptr<NetworkThread::AsyncCallHandler>*)context;
  if (!(*handler)->cancelled)
    (*handler)->callback();
  event_del((*handler)->ev);
  event_free((*handler)->ev);
  delete handler;
}

boost::shared_ptr<NetworkThread::AsyncCallHandler> NetworkThread::asyncCall(uint64_t usDelay, boost::function0<void> cb)
{
  boost::shared_ptr<AsyncCallHandler> res(new AsyncCallHandler);
  struct timeval period;
  period.tv_sec = usDelay / 1000000ULL;
  period.tv_usec = usDelay % 1000000ULL;
  struct event *ev = event_new(_base, -1, 0, async_call,
    new boost::shared_ptr<AsyncCallHandler>(res));
  // Order is important.
  res->ev = ev;
  res->cancelled = false;
  res->callback = cb;
  event_add(ev, &period);
  return res;
}

struct event_base* NetworkThread::getEventBase()
{
  return _base;
}

}
