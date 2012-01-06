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

#include "transport-client.hpp"

#define MAX_LINE 16384

TransportClient::TransportClient()
{
  setvbuf(stdout, NULL, _IONBF, 0);
}


TransportClient::~TransportClient()
{
}

void TransportClient::setconnection(const std::string &host,
                                    unsigned short port)
{
  _host = host;
  _port = port;
}

void TransportClient::writecb(struct bufferevent* bev, void* context)
{
  (void) bev;

  TransportClient *tc = static_cast<TransportClient*>(context);
  tc->_tcd->onWrite();
}

void TransportClient::readcb(struct bufferevent* bev, void* context)
{
  (void) bev;

  TransportClient *tc = static_cast<TransportClient*>(context);
  tc->_tcd->onRead();
}

void TransportClient::errorcb(struct bufferevent *bev,
                              short error,
                              void *ctx)
{
  if (error & BEV_EVENT_EOF)
  {
    // connection has been closed, do any clean up here
    std::cout << "connection has been closed, do any clean up here" << std::endl;
  }
  else if (error & BEV_EVENT_ERROR)
  {
    // check errno to see what error occurred
    std::cout << "check errno to see what error occurred" << std::endl;
  }
  else if (error & BEV_EVENT_TIMEOUT)
  {
    // must be a timeout event handle, handle it
    std::cout << "must be a timeout event handle, handle it" << std::endl;
  }

  bufferevent_free(bev);
}

void TransportClient::setDelegate(TransportClientDelegate *delegate)
{
  _tcd = delegate;
}

bool TransportClient::send(const std::string &msg)
{
  struct event_base *base = event_base_new();

  struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, readcb, writecb, errorcb, this);
  bufferevent_setwatermark(bev, EV_WRITE, 0, MAX_LINE);
  bufferevent_enable(bev, EV_READ|EV_WRITE);

  if (bufferevent_socket_connect_hostname(bev, NULL, AF_INET, _host.c_str(), _port) < 0)
  {
    /* Error starting connection */
    bufferevent_free(bev);
    return false;
  }

  bufferevent_write(bev, msg.c_str(), msg.size());
  event_base_dispatch(base);
}
