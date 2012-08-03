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
  event_base_free(_base);
#ifdef _WIN32
  // TODO handle return code
  ::WSACleanup();
#endif
}

void NetworkThread::run()
{

  struct bufferevent *bev = bufferevent_socket_new(_base, -1, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, NULL, NULL, ::qi::errorcb, this);
  bufferevent_enable(bev, EV_READ|EV_WRITE);

  event_base_dispatch(_base);
}

void NetworkThread::stop()
{
  if (event_base_loopexit(_base, NULL) != 0)
    qiLogError("networkThread") << "Can't stop the NetworkThread";
  this->join();
}

void NetworkThread::join()
{
  _thd.join();
}

struct event_base* NetworkThread::getEventBase()
{
  return _base;
}

}
