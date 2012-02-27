/*
** transport-client.hpp
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

#pragma once
#ifndef _QIMESSAGING_TRANSPORT_SOCKET_HPP_
#define _QIMESSAGING_TRANSPORT_SOCKET_HPP_

# include <string>
# include <qi/macro.hpp>

# include <event2/event.h>
# include <event2/bufferevent.h>
# include <qimessaging/message.hpp>
# include <qimessaging/url.hpp>

namespace qi {

class TransportSocket;

class TransportSocketInterface
{
public:
  virtual void onConnected(TransportSocket *client)    = 0;
  virtual void onDisconnected(TransportSocket *client) = 0;
  virtual void onWriteDone(TransportSocket *client)        = 0;
  virtual void onReadyRead(TransportSocket *client, qi::Message &msg)         = 0;
};

struct TransportSocketPrivate;

class TransportSocket
{
  QI_DISALLOW_COPY_AND_ASSIGN(TransportSocket);

public:
  TransportSocket();
  TransportSocket(int fd, struct event_base *base);
  virtual ~TransportSocket();

  bool connect(const std::string &address,
               unsigned short port,
               struct event_base *base);
  void disconnect();

  bool isConnected();

  // if msecs < 0 no timeout
  bool waitForConnected(int msecs = 30000);
  bool waitForDisconnected(int msecs = 30000);
  bool waitForId(int id, int msecs = 30000);

  void read(int id, qi::Message *msg);

  bool send(qi::Message &msg);

  void setDelegate(TransportSocketInterface *delegate);

  void readcb(struct bufferevent *bev,
              void *context);
  void writecb(struct bufferevent* bev,
               void* context);
  void eventcb(struct bufferevent *bev,
               short error,
               void *context);
private:
  TransportSocketPrivate  *_p;
};

}

#endif  // _QIMESSAGING_TRANSPORT_SOCKET_HPP_
