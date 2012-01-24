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

struct TransportClientPrivate
{
  TransportClientPrivate()
    : connected(false)
    , bev(NULL)
  {
  }

  ~TransportClientPrivate()
  {
  }

  TransportClientDelegate *tcd;
  struct bufferevent      *bev;
  bool                     connected;
};



static void readcb(struct bufferevent *bev,
                   void *context)
{
  TransportClient *tc = static_cast<TransportClient*>(context);
  tc->readcb(bev, context);
}

static void writecb(struct bufferevent* bev,
                    void* context)
{
  TransportClient *tc = static_cast<TransportClient*>(context);
  tc->writecb(bev, context);
}


static void eventcb(struct bufferevent *bev,
                    short error,
                    void *context)
{
  TransportClient *tc = static_cast<TransportClient*>(context);
  tc->eventcb(bev, error, context);
}


void TransportClient::readcb(struct bufferevent *bev,
                             void *context)
{
  char buf[1024];
  memset(buf, '\0', 1024);
  size_t n;
  struct evbuffer *input = bufferevent_get_input(bev);

  while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0)
  {
    _p->tcd->onRead(std::string(buf));
  }
}

void TransportClient::writecb(struct bufferevent* bev,
                              void* context)
{
  _p->tcd->onWrite();
}

void TransportClient::eventcb(struct bufferevent *bev,
                              short events,
                              void *context)
{
  if (events & BEV_EVENT_CONNECTED)
  {
    _p->tcd->onConnected();
    _p->connected = true;
  }
  else if (events & BEV_EVENT_EOF)
  {
    // connection has been closed, do any clean up here
    qiLogError("qimessaging.TransportClient") << "connection has been closed, do any clean up here" << std::endl;
  }
  else if (events & BEV_EVENT_ERROR)
  {
    bufferevent_free(_p->bev);
    // check errno to see what error occurred
    qiLogError("qimessaging.TransportClient")  << "Cannnot connect" << std::endl;
  }
  else if (events & BEV_EVENT_TIMEOUT)
  {
    // must be a timeout event handle, handle it
    qiLogError("qimessaging.TransportClient")  << "must be a timeout event handle, handle it" << std::endl;
  }
}

TransportClient::TransportClient()
{
  _p = new TransportClientPrivate();
}


TransportClient::~TransportClient()
{
  disconnect();
  delete _p;
}

bool TransportClient::connect(const std::string &address,
                              unsigned short port,
                              struct event_base *base)
{
  if (!_p->connected)
  {
    _p->bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(_p->bev, ::readcb, ::writecb, ::eventcb, this);
    bufferevent_setwatermark(_p->bev, EV_WRITE, 0, MAX_LINE);
    bufferevent_enable(_p->bev, EV_READ|EV_WRITE);

    bufferevent_socket_connect_hostname(_p->bev, NULL, AF_INET, address.c_str(), port);

    if (_p->connected)
      return true;
  }

  return false;
}

bool TransportClient::waitForConnected(int msecs)
{
  // no timeout
  if (msecs < 0)
  {
    while (!_p->connected)
      ;

    return true;
  }

  while (!_p->connected && msecs > 0)
  {
    qi::os::msleep(1);
    msecs--;
  }

  // timeout
  if (msecs == 0)
    return false;

  return true;
}

void TransportClient::disconnect()
{
  if (_p->connected)
  {
    bufferevent_free(_p->bev);
    _p->bev = NULL;
    _p->connected = false;
  }
}

bool TransportClient::waitForDisconnected(int msecs)
{
  // no timeout
  if (msecs < 0)
  {
    while (_p->connected)
      ;

    return true;
  }

  while (_p->connected && msecs > 0)
  {
    qi::os::msleep(1);
    msecs--;
  }

  // timeout
  if (msecs == 0)
    return false;

  return true;
}

bool TransportClient::send(const std::string &msg)
{
  if (_p->connected && !bufferevent_write(_p->bev, msg.c_str(), msg.size()))
    return true;

  return false;
}

void TransportClient::setDelegate(TransportClientDelegate *delegate)
{
  _p->tcd = delegate;
}
