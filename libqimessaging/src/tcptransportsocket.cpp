/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "src/tcptransportsocket.hpp"
#include "src/tcptransportsocket_p.hpp"

namespace qi {

  //## TcpTransportSocket
  TcpTransportSocket::TcpTransportSocket(EventLoop* eventLoop)
    : TransportSocket(new TcpTransportSocketPrivate(this, eventLoop))
  {
  }

  TcpTransportSocket::TcpTransportSocket(int fileDesc, EventLoop* eventLoop)
    : TransportSocket(new TcpTransportSocketPrivate(this, fileDesc, eventLoop))
  {
  }

  TcpTransportSocket::~TcpTransportSocket() {
  }


  qi::FutureSync<bool> TcpTransportSocket::connect(const qi::Url &url) {
    return static_cast<TcpTransportSocketPrivate*>(_p)->connect(url);
  }

  qi::FutureSync<void> TcpTransportSocket::disconnect() {
    return static_cast<TcpTransportSocketPrivate*>(_p)->disconnect();
  }

  bool TcpTransportSocket::send(const qi::Message &msg) {
    return static_cast<TcpTransportSocketPrivate*>(_p)->send(msg);
  }

}
