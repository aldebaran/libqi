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
#include "src/transport_server_libevent_p.hpp"

namespace qi
{
  void accept_cb(struct evconnlistener *listener,
                 evutil_socket_t        fd,
                 struct sockaddr       *QI_UNUSED(a),
                 int                    QI_UNUSED(slen),
                 void                  *p)
  {
    TransportServerLibEventPrivate *_p = static_cast<TransportServerLibEventPrivate *>(p);
    _p->accept(fd, listener, p);
  }

  void TransportServerLibEventPrivate::accept(evutil_socket_t        fd,
                                              struct evconnlistener *listener,
                                              void                  *QI_UNUSED(context))
  {
    struct event_base *base = evconnlistener_get_base(listener);

    qi::TransportSocket *ts = new qi::TransportSocket();
    if (ts->_p)
      delete ts->_p;
    ts->_p = new qi::TransportSocketLibEvent(ts, fd, base);
    connection.push(ts);

    tsi->newConnection();
  }

  bool TransportServerLibEventPrivate::start()
  {
    struct event_base *base = mainSession->_p->_networkThread->getEventBase();
    struct evconnlistener *listener;
    static struct sockaddr_storage listen_on_addr;
    memset(&listen_on_addr, 0, sizeof(listen_on_addr));
    int socklen = sizeof(listen_on_addr);
    struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in *>(&listen_on_addr);

    socklen = sizeof(struct sockaddr_in);
    sin->sin_family = AF_INET;
    if ((sin->sin_addr.s_addr = inet_addr(listenUrl.host().c_str())) == INADDR_NONE)
    {
      qiLogError("qimessaging.transportserver") << "Provided IP is not valid" << std::endl;
      return false;
    }

    bool findPort = listenUrl.port() == 0;
    unsigned short port = findPort ? qi::os::findAvailablePort(0) : listenUrl.port();

    do
    {
      sin->sin_port = htons(port);
      listener = evconnlistener_new_bind(base,
                                         accept_cb,
                                         this,
                                         LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC | LEV_OPT_REUSEABLE,
                                         -1,
                                         (struct sockaddr *)&listen_on_addr,
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

      listenUrl = "tcp://" + listenUrl.host() + ":" + out.str();
      qiLogVerbose("qimessaging.transportserver") << "Starting server at "
                                                  << listenUrl.str();
    }
    else
    {
      qiLogError("qimessaging.transportserver") << "Could not start server at "
                                                << listenUrl.str();
    }

    return listener != 0;
  }

  TransportServerLibEventPrivate::TransportServerLibEventPrivate(qi::Session *session,
                                                                 const qi::Url &url)
    : TransportServerPrivate(session, url)
    , base(0)
  {
  }

  TransportServerLibEventPrivate::~TransportServerLibEventPrivate()
  {
  }
}
