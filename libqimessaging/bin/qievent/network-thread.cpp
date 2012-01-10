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

#include <qi/log.hpp>

#include "network-thread.hpp"

static void errorcb(struct bufferevent *bev,
                    short error,
                    void *ctx)
{
  if (error & BEV_EVENT_EOF)
  {
    // connection has been closed, do any clean up here
    qiLogError("qimessaging.TransportClient") << "connection has been closed, do any clean up here" << std::endl;
  }
  else if (error & BEV_EVENT_ERROR)
  {
    // check errno to see what error occurred
    qiLogError("qimessaging.TransportClient")  << "check errno to see what error occurred" << std::endl;
  }
  else if (error & BEV_EVENT_TIMEOUT)
  {
    // must be a timeout event handle, handle it
    qiLogError("qimessaging.TransportClient")  << "must be a timeout event handle, handle it" << std::endl;
  }
}


NetworkThread::NetworkThread()
{
  _thd = boost::thread(&NetworkThread::run, this);
}

NetworkThread::~NetworkThread()
{
  event_base_free(_base);
}

void NetworkThread::run()
{
  if (!(_base = event_base_new()))
    return;

  struct bufferevent *bev = bufferevent_socket_new(_base, -1, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, NULL, NULL, errorcb, this);
  bufferevent_enable(bev, EV_READ|EV_WRITE);

  event_base_dispatch(_base);
}

struct event_base* NetworkThread::getEventBase()
{
  return _base;
}
