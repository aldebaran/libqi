#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TCPTRANSPORTSOCKET_P_HPP_
#define _SRC_TCPTRANSPORTSOCKET_P_HPP_

# include <string>
# include <boost/thread/recursive_mutex.hpp>
# include <qi/atomic.hpp>
# include <qimessaging/api.hpp>
# include <qimessaging/message.hpp>
# include <qimessaging/url.hpp>
# include <qimessaging/transportsocket.hpp>
# include <qi/eventloop.hpp>

# include "src/transportsocket_p.hpp"
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
    void startReading();

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

    bool send_(const qi::Message &msg, bool allocated);
    void connect_(const qi::Url &url);
    void disconnect_();
  private:
    struct bufferevent    *_bev;

    qi::Promise<bool>      _connectPromise;
    qi::Promise<void>      _disconnectPromise;

    // data to rebuild message
    bool                   _readHdr;
    qi::Message           *_msg;
    bool                   _connecting;
    bool                   _disconnecting;
    int                    _fd;
  };

}

#endif  // _SRC_TCPTRANSPORTSOCKET_P_HPP_
