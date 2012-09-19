#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TCPTRANSPORTSOCKET_HPP_
#define _SRC_TCPTRANSPORTSOCKET_HPP_

# include <qimessaging/transportsocket.hpp>
# include <qimessaging/eventloop.hpp>

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

#endif  // _SRC_TCPTRANSPORTSOCKET_HPP_
