/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <qi/log.hpp>
#include <cerrno>

#ifdef _WIN32
#include <winsock2.h> // for socket
#include <WS2tcpip.h> // for socklen_t
#else
#include <arpa/inet.h>
#endif

#include <qimessaging/transportserver.hpp>
#include <qimessaging/transportsocket.hpp>
#include "transportserverasio_p.hpp"
#include "transportserverdummy_p.hpp"
#include "tcptransportsocket_p.hpp"

namespace qi
{


  static TransportServerPrivate * newTSP(TransportServer* self,
    const qi::Url &url,
    qi::EventLoop* ctx) {
    if (url.protocol() == "tcp") {
      return new TransportServerAsioPrivate(self, url, ctx);
    }

    qiLogError("TransportServer") << "Unrecognized protocol to create the TransportServer."
                                  << " TransportServer create with dummy implementation.";
    return new TransportServerDummyPrivate(self, url, ctx);
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
    close();
    _p->destroy();
    _p = 0;
  }

  bool TransportServer::listen(const qi::Url &url, qi::EventLoop* ctx)
  {
    close();
    delete _p;
    _p = newTSP(this, url, ctx);
    return listen();
  }

  bool TransportServer::listen()
  {
    return _p->listen();
  }

  qi::Url TransportServer::listenUrl() const {
    return _p->listenUrl;
  }

  std::vector<qi::Url> TransportServer::endpoints() const
  {
    return _p->_endpoints;
  }

  void TransportServer::close() {
    _p->close();
    _p->listenUrl = qi::Url("");
  }

}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
