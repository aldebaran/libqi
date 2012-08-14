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
#include "src/transport_server_libevent_p.hpp"
#include "src/transport_server_dummy_p.hpp"
#include "src/transport_socket_libevent_p.hpp"

namespace qi
{


  static TransportServerPrivate * newTSP(TransportServer* self,
    const qi::Url &url,
    qi::EventLoop* ctx) {
    if (url.protocol() == "tcp") {
      return new TransportServerLibEventPrivate(self, url, ctx);
    }

    qiLogError("TransportServer") << "Unrecognized protocol to create the TransportServer."
                                  << " TransportServer create with dummy implementation.";
    return new TransportServerDummyPrivate(self, url, ctx);
  }

  TransportServerInterface::~TransportServerInterface()
  {
  }

  TransportServer::TransportServer()
    : _p(new TransportServerDummyPrivate(this, "", 0))
  {
  }


  TransportServer::TransportServer(const qi::Url &url,
    qi::EventLoop* ctx
    )
    : _p(newTSP(this, url, ctx))
  {
  }

  TransportServer::~TransportServer()
  {
    _p->tsi.clear();
    close();
    _p->destroy();
    _p = 0;
  }

  bool TransportServer::listen(const qi::Url &url, qi::EventLoop* ctx)
  {
    TransportServerPrivate *save = _p;

    _p = newTSP(this, url, ctx);
    _p->tsi = save->tsi;

    delete save;
    return listen();
  }

  bool TransportServer::listen()
  {
    return _p->listen();
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

  std::vector<qi::Url> TransportServer::endpoints() const
  {
    return _p->_endpoints;
  }

  bool TransportServer::close() {
    return _p?_p->close():true;
  }

}
