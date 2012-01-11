/*
** transport-server.cpp
** Login : <hcuche@hcuche-de>
** Started on  Wed Jan 11 10:19:42 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#include <iostream>
#include <string>
#include <qi/log.hpp>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <arpa/inet.h>

#include "transport-server.hpp"

#define MAX_LINE 16384

static void readcb(struct bufferevent *bev,
                   void *ctx)
{
  char buf[1024];
  size_t n;
  struct evbuffer *input = bufferevent_get_input(bev);

  while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0)
  {
    fwrite(buf, 1, n, stdout);
    fflush(stdout);
  }

  bufferevent_free(bev);
}

static void errorcb(struct bufferevent *bev,
                    short error,
                    void *ctx)
{
  if (error & BEV_EVENT_EOF)
  {
    // connection has been closed, do any clean up here
    qiLogError("qimessaging.TransportServer") << "connection has been closed, do any clean up here" << std::endl;
  }
  else if (error & BEV_EVENT_ERROR)
  {
    // check errno to see what error occurred
    qiLogError("qimessaging.TransportServer")  << "check errno to see what error occurred" << std::endl;
  }
  else if (error & BEV_EVENT_TIMEOUT)
  {
    // must be a timeout event handle, handle it
    qiLogError("qimessaging.TransportServer")  << "must be a timeout event handle, handle it" << std::endl;
  }

  bufferevent_free(bev);
}

static void accept(evutil_socket_t listener,
                   short ev,
                   void *arg)
{
  struct event_base *base = static_cast<struct event_base *>(arg);
  struct sockaddr_storage ss;
  socklen_t slen = sizeof(ss);

  int fd = ::accept(listener, (struct sockaddr*)&ss, &slen);

  if (fd < 0)
  {
    qiLogError("qimessaging.TransportServer") << "Gateway: Could not accept client" << std::endl;
  }
  else if (fd > FD_SETSIZE)
  {
    ::close(fd);
  }
  else
  {
    struct bufferevent *bev;
    evutil_make_socket_nonblocking(fd);
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, readcb, NULL, errorcb, NULL);
    bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
  }
}

TransportServer::TransportServer(const std::string &address,
                                 unsigned short port)
  : _address(address)
  , _port(port)
{
}

TransportServer::~TransportServer()
{
}

void TransportServer::run(struct event_base *base)
{
  evutil_socket_t sock;
  struct event*   sockEvent;

  // get tcp socket
  if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    qiLogError("qimessaging.transportserver") << "Could not get socket" << std::endl;
    return;
  }
  evutil_make_socket_nonblocking(sock);

  // get valid IP
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(_port);

  if ((addr.sin_addr.s_addr = inet_addr(_address.c_str())) == INADDR_NONE)
  {
    qiLogError("qimessaging.transportserver") << "Provided IP is not valid" << std::endl;
    return;
  }

  // bind socket
  if (::bind(sock, (struct sockaddr*)&addr, sizeof (addr)) == -1)
  {
    qiLogError("qimessaging.transportserver") << "Could not bind socket" << std::endl;
    return;
  }

  //listen on the socket
  if (::listen(sock, SOMAXCONN) == -1)
  {
    qiLogError("qimessaging.transportserver") << "Could not listen on socket" << std::endl;
    return;
  }

  sockEvent = event_new(base, sock, EV_READ | EV_PERSIST,
                        accept, base);

  event_add(sockEvent, NULL);
}
