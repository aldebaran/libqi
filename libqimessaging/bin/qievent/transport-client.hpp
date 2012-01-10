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


# include <event2/event.h>
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
  explicit TransportClient(const std::string &adress,
                           unsigned short port);
  virtual ~TransportClient();

  bool send(const std::string &msg,
            struct event_base *base);

  void setDelegate(TransportClientDelegate *delegate);
  TransportClientDelegate *_tcd;

private:
  std::string         _adress;
  unsigned short      _port;
};

#endif	    /* !TRANSPORT_CLIENT_PP_ */
