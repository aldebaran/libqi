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
#include <cstdlib>
#include <queue>
#include <qi/log.hpp>
#include <cerrno>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

#ifdef _WIN32
 #include <winsock2.h> // for socket
 #include <WS2tcpip.h> // for socklen_t
#else
 #include <arpa/inet.h>
#endif

#include <qimessaging/transport_server.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/session.hpp>
#include "src/session_p.hpp"
#include "src/network_thread.hpp"
#include "src/transport_server_p.hpp"
#include "src/transport_socket_libevent_p.hpp"

namespace qi {
#define MAX_LINE 16384



void accept_cb(struct evconnlistener *listener,
               evutil_socket_t        fd,
               struct sockaddr       *QI_UNUSED(a),
               int                    QI_UNUSED(slen),
               void                  *p)
{
  TransportServerPrivate *_p = static_cast<TransportServerPrivate*>(p);
  _p->accept(fd, listener, p);
}


void TransportServerPrivate::accept(evutil_socket_t        fd,
                                    struct evconnlistener *listener,
                                    void                  *QI_UNUSED(context))
{
  struct event_base *base = evconnlistener_get_base(listener);

  qi::TransportSocket *ts = new qi::TransportSocket();
  ts->_p = new qi::TransportSocketLibEvent(ts, fd, base);
  connection.push(ts);

  tsi->newConnection();
}

bool TransportServerPrivate::start(struct event_base *base, const qi::Url &url)
{
  struct evconnlistener *listener;
  static struct sockaddr_storage listen_on_addr;
  memset(&listen_on_addr, 0, sizeof(listen_on_addr));
  int socklen = sizeof(listen_on_addr);
  struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in *>(&listen_on_addr);

  socklen = sizeof(struct sockaddr_in);
  sin->sin_family = AF_INET;
  if ((sin->sin_addr.s_addr = inet_addr(url.host().c_str())) == INADDR_NONE)
  {
    qiLogError("qimessaging.transportserver") << "Provided IP is not valid" << std::endl;
    return false;
  }

  bool findPort = url.port() == 0;
  unsigned short port = findPort ? qi::os::findAvailablePort(0) : url.port();

  do
  {
    sin->sin_port = htons(port);
    listener = evconnlistener_new_bind(base,
                                       accept_cb,
                                       this,
                                       LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC | LEV_OPT_REUSEABLE,
                                       -1,
                                       (struct sockaddr*)&listen_on_addr,
                                       socklen);

    if (!findPort || (findPort && listener))
    {
      break;
    }

    port = qi::os::findAvailablePort(port);
  }
  while (port != 0);

  if (listener)
  {
    std::stringstream out;
    out << port;

    listenUrl = "tcp://" + url.host() + ":" + out.str();
    qiLogVerbose("qimessaging.transportserver") << "Starting server at "
                                                << listenUrl.str();
  }
  else
  {
    qiLogError("qimessaging.transportserver") << "Could not start server at "
                                              << url.str();
  }

  return listener != 0;
}

TransportServerInterface::~TransportServerInterface()
{
}

TransportServer::TransportServer()
{
  _p = new TransportServerPrivate();
}

TransportServer::~TransportServer()
{
  delete _p;
}

bool TransportServer::start(qi::Session *session, const qi::Url &url)
{
  struct event_base *base = session->_p->_networkThread->getEventBase();
  return _p->start(base, url);
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

void TransportServer::setCallbacks(TransportServerInterface *delegate)
{
  _p->tsi = delegate;
}

}
