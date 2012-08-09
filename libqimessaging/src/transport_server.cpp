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


  static TransportServerPrivate * newTSP(qi::Session *session, const qi::Url &url) {
    if (url.protocol() == "tcp") {
      return new TransportServerLibEventPrivate(session, url);
    }

    qiLogError("TransportServer") << "Unrecognized protocol to create the TransportServer."
                                  << " TransportServer create with dummy implementation.";
    return new TransportServerDummyPrivate(session, url);
  }

  TransportServerInterface::~TransportServerInterface()
  {
  }

  TransportServer::TransportServer()
    : _p(new TransportServerDummyPrivate(NULL, ""))
  {
  }


  TransportServer::TransportServer(qi::Session *session,
                                   const qi::Url &url)
    : _p(newTSP(session, url))
  {
  }

  TransportServer::~TransportServer()
  {
    _p->tsi.clear();
    close();
    _p->destroy();
    _p = 0;
  }

  bool TransportServer::listen(qi::Session *session,
                              const qi::Url &url)
  {
    TransportServerPrivate *save = _p;

    _p = newTSP(session, url);
    _p->tsi = save->tsi;

    delete save;
    return listen();
  }

  bool TransportServer::listen()
  {
    return _p->listen();
  }

  void TransportServer::join()
  {
    if (_p)
      _p->join();
  }

  void TransportServer::addCallbacks(TransportServerInterface *delegate)
  {
    if (_p == NULL)
    {
      qiLogError("TransportServer") << "TransportServer is not start. "
                                    << "You cannot set callbacks.";
      return;
    }
    if (delegate)
    {
      boost::mutex::scoped_lock l(_p->mutexCallback);
      _p->tsi.push_back(delegate);
    }
    else
      qiLogError("TransportServer") << "Trying to set invalid callback on TransportServer.";
  }

  void TransportServer::removeCallbacks(TransportServerInterface *delegate)
  {
    if (_p == NULL)
    {
      qiLogError("TransportServer") << "TransportServer is not start. "
                                    << "You cannot set callbacks.";
      return;
    }
    if (delegate)
    {
      boost::mutex::scoped_lock l(_p->mutexCallback);
      std::vector<TransportServerInterface *>::iterator it;
      for (it = _p->tsi.begin(); it != _p->tsi.end(); ++it)
      {
        if (*it == delegate)
        {
          _p->tsi.erase(it);
          break;
        }
      }
    }
    else
      qiLogError("TransportServer") << "Trying to erase invalid callback on TransportServer.";
  }

  qi::Url TransportServer::listenUrl() const {
    return _p->listenUrl;
  }

  bool TransportServer::close() {
    return _p?_p->close():true;
  }

}
