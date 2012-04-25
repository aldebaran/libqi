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
#include "src/transport_server_libevent_p.hpp"
#include "src/transport_server_dummy_p.hpp"
#include "src/transport_socket_libevent_p.hpp"

namespace qi
{
  TransportServerInterface::~TransportServerInterface()
  {
  }

  TransportServer::TransportServer()
  {
    _p = new TransportServerDummyPrivate(NULL, "");
  }

  TransportServer::TransportServer(qi::Session *session,
                                   const qi::Url &url)
  {
    switch (url.protocol())
    {
      case qi::Url::Protocol_Tcp:
        _p = new TransportServerLibEventPrivate(session, url);
        break;

      default:
        qiLogError("TransportServer") << "Unrecognized protocol to create the TransportServer."
                                      << " TransportServer create with dummy implementation.";
        _p = new TransportServerDummyPrivate(session, url);
        break;
    }
  }

  TransportServer::~TransportServer()
  {
    delete _p;
  }

  bool TransportServer::start(qi::Session *session,
                              const qi::Url &url)
  {
    TransportServerPrivate *save = _p;
    switch (url.protocol())
    {
      case qi::Url::Protocol_Tcp:
        _p = new TransportServerLibEventPrivate(session, url);
        _p->tsi = save->tsi;
        _p->connection = save->connection;
        break;

      default:
        qiLogError("TransportServer") << "Unrecognized protocol to start the TransportServer.";
        delete save;
        return false;
    }

    delete save;
    return start();
  }

  bool TransportServer::start()
  {
    return _p->start();
  }

  TransportSocket *TransportServer::nextPendingConnection()
  {
    if (_p == NULL)
    {
      qiLogError("TransportServer") << "TransportServer is not start. "
                                    << "You cannot get next pending connection.";
      return NULL;
    }
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
    if (_p == NULL)
    {
      qiLogError("TransportServer") << "TransportServer is not start. "
                                    << "You cannot set callbacks.";
      return;
    }
    _p->tsi = delegate;
  }
}
