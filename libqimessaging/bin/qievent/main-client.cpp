/*
** main-client.cpp
** Login : <hcuche@hcuche-de>
** Started on  Tue Jan  3 13:52:00 2012 Herve Cuche
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

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "transport-client.hpp"

class RemoteService : public TransportClientDelegate {
public:
  RemoteService() {
    tc.setDelegate(this);

    tc.setconnection("127.0.0.1", 12345);
    tc.send("totot\n");
  }

  virtual void onConnected() { std::cout << "connected" << std::endl;};
  virtual void onWrite() { std::cout << "wroten" << std::endl;};
  virtual void onRead() { std::cout << "read" << std::endl;};

private:
  TransportClient tc;
};

int main(int argc, char *argv[])
{
  RemoteService rs;
  return 0;

}

//#define MAX_LINE 16384

//void errorcb(struct bufferevent *bev,
//             short error,
//             void *ctx)
//{



//  if (error & BEV_EVENT_EOF)
//  {
//    // connection has been closed, do any clean up here
//  }
//  else if (error & BEV_EVENT_ERROR)
//  {
//    // check errno to see what error occurred
//  }
//  else if (error & BEV_EVENT_TIMEOUT)
//  {
//    // must be a timeout event handle, handle it
//  }

//  std::string lol("toto ");
//  std::string lol1("toto\n");
//  bufferevent_write(bev, lol.c_str(), lol.size());
//  bufferevent_write(bev, lol1.c_str(), lol1.size());

//  bufferevent_free(bev);
//}

//void run(void)
//{
//  struct event_base *base;
//  struct bufferevent *bev;

//  base = event_base_new();

//  bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
//  bufferevent_setcb(bev, NULL, NULL, errorcb, NULL);
//  bufferevent_enable(bev, EV_WRITE);
//  bufferevent_disable(bev, EV_READ);

//  if (bufferevent_socket_connect_hostname(bev, NULL, AF_INET, "127.0.0.1", 12345) < 0)
//  {
//    /* Error starting connection */
//    bufferevent_free(bev);
//    return;
//  }

//  event_base_dispatch(base);
//  return;
//}

//int main(int argc, char *argv[])
//{
//  setvbuf(stdout, NULL, _IONBF, 0);

//  run();
//  return 0;
//}

