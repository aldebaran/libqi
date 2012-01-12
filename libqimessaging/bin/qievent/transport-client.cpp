/*
** transport-client.cpp
** Login : <hcuche@hcuche-de>
** Started on  Thu Jan  5 15:21:13 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#include <iostream>
#include <cstring>
#include <qi/log.hpp>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "transport-client.hpp"
#include "network-thread.hpp"

#define MAX_LINE 16384


static void readcb(struct bufferevent *bev,
                   void *context)
{

  char buf[1024];
  memset(buf, '\0', 1024);
  size_t n;
  struct evbuffer *input = bufferevent_get_input(bev);

  TransportClient *tc = static_cast<TransportClient*>(context);

  while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0)
  {
    tc->_tcd->onRead(std::string(buf));
  }

  bufferevent_free(bev);
}


static void writecb(struct bufferevent* bev, void* context)
{
  (void) bev;

  TransportClient *tc = static_cast<TransportClient*>(context);
  tc->_tcd->onWrite();

  bufferevent_free(bev);
}

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

TransportClient::TransportClient(const std::string &address,
                                 unsigned short port)
  : _address(address)
  , _port(port)
{
}


TransportClient::~TransportClient()
{
}

bool TransportClient::send(const std::string &msg,
                           struct event_base *base)
{
  struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, readcb, writecb, errorcb, this);
  bufferevent_setwatermark(bev, EV_WRITE, 0, MAX_LINE);
  bufferevent_enable(bev, EV_READ|EV_WRITE);

  if (bufferevent_socket_connect_hostname(bev, NULL, AF_INET, _address.c_str(), _port) < 0)
  {
    bufferevent_free(bev);
    return false;
  }

  _tcd->onConnected();

  if (!bufferevent_write(bev, msg.c_str(), msg.size()))
    return true;

  return false;
}

void TransportClient::setDelegate(TransportClientDelegate *delegate)
{
  _tcd = delegate;
}
