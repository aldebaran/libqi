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

# include <qimessaging/api.hpp>
# include <qimessaging/message.hpp>
# include <qimessaging/url.hpp>
# include <qimessaging/transport_socket.hpp>

# include "src/transport_socket_p.hpp"

namespace qi
{
  class Session;
  class TransportSocketPrivate;
  class TransportSocketLibEvent : public TransportSocketPrivate
  {
  public:
    TransportSocketLibEvent(TransportSocket *socket);
    TransportSocketLibEvent(TransportSocket *socket, int fileDesc, void *data);
    virtual ~TransportSocketLibEvent();

    virtual bool connect(qi::Session *session,
                         const qi::Url &url);
    virtual void disconnect();
    virtual bool send(const qi::Message &msg);

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
    struct bufferevent *bev;
    int                 fd;
    struct event       *clean_event;
  };

}

#endif  // _QIMESSAGING_TRANSPORT_SOCKET_LIBEVENT_P_HPP_
