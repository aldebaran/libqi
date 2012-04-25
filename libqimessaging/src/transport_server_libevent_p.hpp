/*
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
 **
 ** Copyright (C) 2012 Aldebaran Robotics
 */

#pragma once
#ifndef _QIMESSAGING_TRANSPORT_SERVER_LIBEVENT_P_HPP_
#define _QIMESSAGING_TRANSPORT_SERVER_LIBEVENT_P_HPP_

# include <qimessaging/api.hpp>
# include <qimessaging/url.hpp>
# include <qimessaging/session.hpp>
# include "src/transport_server_p.hpp"

# include <event2/event.h>
# include <event2/bufferevent.h>

# include <string>
# include <queue>

namespace qi
{
  class TransportServerLibEventPrivate : public TransportServerPrivate
  {
  public:
    TransportServerLibEventPrivate(qi::Session *session,
                                   const qi::Url &url);
    virtual ~TransportServerLibEventPrivate();

    virtual bool start();

    void accept(evutil_socket_t        fd,
                struct evconnlistener *listener,
                void                  *context);

    struct event_base *base;

  private:
    TransportServerLibEventPrivate() {};
  };
}

#endif  // _QIMESSAGING_TRANSPORT_SERVER_LIBEVENT_P_HPP_
