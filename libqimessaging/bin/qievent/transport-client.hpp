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

#ifndef   	TRANSPORT_CLIENT_HPP_
# define   	TRANSPORT_CLIENT_HPP_

# include <string>
# include <qi/macro.hpp>

# include <event2/util.h>
# include <event2/event.h>
# include <event2/buffer.h>
# include <event2/bufferevent.h>

class TransportClientDelegate
{
public:
  virtual void onConnected() = 0;
  virtual void onWrite()     = 0;
  virtual void onRead()      = 0;
};

class TransportClient
{
  QI_DISALLOW_COPY_AND_ASSIGN(TransportClient);

public:
  explicit TransportClient();
  virtual ~TransportClient();


  void setconnection(const std::string &host,
                     unsigned short port);

  void setDelegate(TransportClientDelegate *delegate);

  bool send(const std::string &msg);

private:

  static void writecb(struct bufferevent* bev, void* context);
  static void readcb(struct bufferevent* bev, void* context);
  static void errorcb(struct bufferevent *bev,
                                short error,
                                void *ctx);

  std::string        _host;
  unsigned short     _port;

//  struct event_base *_base;

  TransportClientDelegate *_tcd;

};

#endif	    /* !TRANSPORT_CLIENT_PP_ */
