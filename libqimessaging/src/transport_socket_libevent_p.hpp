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

namespace qi
{
  class Session;
  class TransportSocketPrivate;

  class TransportSocketLibEvent : public TransportSocketPrivate
  {
  public:
    TransportSocketLibEvent(TransportSocket *socket);
    TransportSocketLibEvent(TransportSocket *socket, int fileDesc,
      EventLoop* ctx = getDefaultNetworkEventLoop());
    void setFD(int fd);
    virtual ~TransportSocketLibEvent();

    virtual qi::FutureSync<bool> connect(const qi::Url &url, EventLoop* ctx);
    virtual qi::FutureSync<void> disconnect();
    virtual bool send(const qi::Message &msg);
    virtual void destroy();
    void readcb(struct bufferevent *bev,
                void               *QI_UNUSED(context));
    void writecb(struct bufferevent *QI_UNUSED(bev),
                 void               *QI_UNUSED(context));
    void eventcb(struct bufferevent *bev,
                 short events,
                 void *QI_UNUSED(context));

    void onCleanPendingMessages();
    void cleanPendingMessages();
    void onWriteDone();

    static void onBufferSent(const void *QI_UNUSED(data),
                             size_t QI_UNUSED(datalen),
                             void *buffer);
    static void onMessageSent(const void *QI_UNUSED(data),
                              size_t QI_UNUSED(datalen),
                              void *msg);

  private:
    friend class TransportServerLibEventPrivate;
    friend void disconnect_dec(TransportSocketLibEvent* ptr);
    friend void send_dec(TransportSocketLibEvent*, Message*);
    friend void connect_dec(TransportSocketLibEvent* ptr, qi::Url url, EventLoop*);
    friend void setfd_dec(TransportSocketLibEvent* ptr, int fd);
    bool _send(qi::Message* msg);
    boost::recursive_mutex mutex;
    struct bufferevent *bev;
    int                 fd;
    struct event       *clean_event;
    qi::atomic<long>   inMethod; // used for reentrency and async method tracking
    EventLoop* context;
    qi::Promise<bool>  connectPromise;
    qi::Promise<void>  disconnectPromise;
  };

}

#endif  // _QIMESSAGING_TRANSPORT_SOCKET_LIBEVENT_P_HPP_
