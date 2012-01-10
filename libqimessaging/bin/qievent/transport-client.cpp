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
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <qi/log.hpp>
#include <stdexcept>

#include "transport-client.hpp"

#define MAX_LINE 16384

TransportClient::TransportClient(const char* host, unsigned short port)
  : _host(host)
  , _port(port)
{
}


TransportClient::~TransportClient()
{
}

static void writecb(struct bufferevent* bev, void* context)
{
  (void) bev;

  TransportClient *tc = static_cast<TransportClient*>(context);
  tc->_tcd->onWrite();

  bufferevent_free(bev);
}

static void readcb(struct bufferevent* bev, void* context)
{
  (void) bev;

  TransportClient *tc = static_cast<TransportClient*>(context);
  tc->_tcd->onRead();

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



void TransportClient::run()
{
  setvbuf(stdout, NULL, _IONBF, 0);
  init();

  _base = event_base_new();

  _bev = bufferevent_socket_new(_base, -1, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(_bev, readcb, writecb, errorcb, this);
  bufferevent_setwatermark(_bev, EV_WRITE, 0, MAX_LINE);
  bufferevent_enable(_bev, EV_READ|EV_WRITE);

  if (bufferevent_socket_connect_hostname(_bev, NULL, AF_INET, _host.c_str(), _port) < 0)
  {
    /* Error starting connection */
    bufferevent_free(_bev);
  }

  event_base_dispatch(_base);

//  while (true)
//    ;
}


void TransportClient::launch(TransportClient *t)
{
  try
  {
    t->run();
  }
  catch (const std::exception& e)
  {
    qiLogError("qimessaging.TransportClient", "Gateway: %s", e.what());
  }
}

void TransportClient::init()
{
  qiLogDebug("qimessaging.TransportClient", "Gateway: Init");
  if (!(_base = event_base_new()))
    std::runtime_error("Could not init libevent");
}

void TransportClient::setDelegate(TransportClientDelegate *delegate)
{
  _tcd = delegate;
}

bool TransportClient::send(const std::string &msg)
{
  bufferevent_write(_bev, msg.c_str(), msg.size());
  return true;
}
