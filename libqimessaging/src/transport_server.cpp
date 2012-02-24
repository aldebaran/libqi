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
#include <cstring>
#include <queue>
#include <qi/log.hpp>
#include <cerrno>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#ifdef _WIN32
 #include <winsock2.h> // for socket
 #include <WS2tcpip.h> // for socklen_t
#else
 #include <arpa/inet.h>
#endif

#include <qimessaging/transport_server.hpp>
#include <qimessaging/transport_socket.hpp>

namespace qi {
#define MAX_LINE 16384


class TransportServerPrivate
{
public:
  TransportServerPrivate()
    : tsi(0)
    , base(0)
  {
  }

  ~TransportServerPrivate()
  {
    event_base_free(base);
  }

  void accept(evutil_socket_t listener,
              short events,
              void *context);


  std::queue<qi::TransportSocket*>  connection;
  TransportServerInterface          *tsi;
  struct event_base                 *base;
};

void accept(evutil_socket_t listener,
            short events,
            void *context)
{
  TransportServer *ts = static_cast<TransportServer*>(context);
  ts->_p->accept(listener, events, context);
}

void TransportServerPrivate::accept(evutil_socket_t listener,
                                    short events,
                                    void *context)
{
  struct sockaddr_storage ss;
  socklen_t slen = sizeof(ss);

  int fd = ::accept(listener, (struct sockaddr*)&ss, &slen);

  if (fd < 0)
  {
    qiLogError("qimessaging.TransportServer") << "Gateway: Could not accept client" << std::endl;
  }
  else if (fd > FD_SETSIZE)
  {
    evutil_closesocket(fd);
  }
  else
  {
    evutil_make_socket_nonblocking(fd);
    connection.push(new qi::TransportSocket(fd, base));
    tsi->newConnection();
  }
}


TransportServer::TransportServer()
{
  _p = new TransportServerPrivate();
}

TransportServer::~TransportServer()
{
  delete _p;
}


bool TransportServer::start(const std::string &address,
                            unsigned short port,
                            struct event_base *base)
{
  evutil_socket_t sock;
  struct event*   sockEvent;

  // get tcp socket
  if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    qiLogError("qimessaging.transportserver") << "Could not get socket" << std::endl;
    return false;
  }

  evutil_make_socket_nonblocking(sock);
  evutil_make_listen_socket_reuseable(sock);
  evutil_make_socket_closeonexec(sock);

  // get valid IP
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if ((addr.sin_addr.s_addr = inet_addr(address.c_str())) == INADDR_NONE)
  {
    qiLogError("qimessaging.transportserver") << "Provided IP is not valid" << std::endl;
    return false;
  }

  // bind socket
  if (::bind(sock, (struct sockaddr*)&addr, sizeof (addr)) == -1)
  {
    qiLogError("qimessaging.transportserver") << "Could not bind socket ("
                                              << strerror(errno) << ")" << std::endl;
    return false;
  }

  //listen on the socket
  if (::listen(sock, SOMAXCONN) == -1)
  {
    qiLogError("qimessaging.transportserver") << "Could not listen on socket" << std::endl;
    return false;
  }

  _p->base = base;
  sockEvent = event_new(_p->base, sock, EV_READ | EV_PERSIST,
                        ::qi::accept, this);

  event_add(sockEvent, NULL);

  return true;
}

TransportSocket *TransportServer::nextPendingConnection()
{
  if (!_p->connection.empty())
  {
    qi::TransportSocket *front = _p->connection.front();
    _p->connection.pop();
    return front;
  }

  return NULL;
}

void TransportServer::setDelegate(TransportServerInterface *delegate)
{
  _p->tsi = delegate;
}

}
