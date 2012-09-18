/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_TCP_TRANSPORT_SOCKET_HPP_
#define _QIMESSAGING_TCP_TRANSPORT_SOCKET_HPP_

# include <qimessaging/transport_socket.hpp>
# include <qimessaging/event_loop.hpp>

namespace qi
{

  class TcpTransportSocketPrivate;
  class TcpTransportSocket : public TransportSocket
  {
    QI_DISALLOW_COPY_AND_ASSIGN(TcpTransportSocket);

  public:
    explicit TcpTransportSocket(EventLoop* eventloop = getDefaultNetworkEventLoop());
    explicit TcpTransportSocket(int fileDesc, EventLoop* eventloop = getDefaultNetworkEventLoop());
    virtual ~TcpTransportSocket();

    virtual qi::FutureSync<bool> connect(const qi::Url &url);
    virtual qi::FutureSync<void> disconnect();
    virtual bool send(const qi::Message &msg);
  };

  typedef boost::shared_ptr<TcpTransportSocket> TcpTransportSocketPtr;

}

#endif  // _QIMESSAGING_TRANSPORT_SOCKET_HPP_
