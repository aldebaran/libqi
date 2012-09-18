/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_TRANSPORT_SOCKET_LIBEVENT_P_HPP_
# define _QIMESSAGING_TRANSPORT_SOCKET_LIBEVENT_P_HPP_

# include <string>
# include <boost/thread/recursive_mutex.hpp>
# include <qi/atomic.hpp>
# include <qimessaging/api.hpp>
# include <qimessaging/message.hpp>
# include <qimessaging/url.hpp>
# include <qimessaging/transport_socket.hpp>
# include <qimessaging/event_loop.hpp>

# include "src/transport_socket_p.hpp"
# include <event2/util.h>


namespace qi
{

  class TcpTransportSocketPrivate : public TransportSocketPrivate
  {
  public:
    TcpTransportSocketPrivate(TransportSocket *self, EventLoop* eventloop = getDefaultNetworkEventLoop());
    TcpTransportSocketPrivate(TransportSocket *self, int fileDesc, EventLoop* eventloop = getDefaultNetworkEventLoop());
    virtual ~TcpTransportSocketPrivate();

    virtual qi::FutureSync<bool> connect(const qi::Url &url);
    virtual qi::FutureSync<void> disconnect();
    virtual bool send(const qi::Message &msg);

  private:
    //LibEvent callbacks
    void onRead();
    void onEvent(short events);

    //cleanup function
    static void onBufferSent(const void *QI_UNUSED(data), size_t QI_UNUSED(datalen), void *buffer);
    static void onMessageSent(const void *QI_UNUSED(data), size_t QI_UNUSED(datalen), void *msg);

    //give access to C callback
    friend void readcb(struct bufferevent *bev, void *context);
    friend void eventcb(struct bufferevent *bev, short error, void *context);

  private:
    struct bufferevent    *_bev;

    qi::Promise<bool>      _connectPromise;
    qi::Promise<void>      _disconnectPromise;

    // data to rebuild message
    bool                   _readHdr;
    qi::Message           *_msg;
    bool                   _connecting;
  };

}

#endif  // _QIMESSAGING_TRANSPORT_SOCKET_LIBEVENT_P_HPP_
