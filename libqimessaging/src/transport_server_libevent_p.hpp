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
# include "src/transport_server_p.hpp"

# include <event2/event.h>
# include <event2/bufferevent.h>

# include <string>
# include <queue>

struct evconnlistener;

namespace qi
{
  class TransportServerLibEventPrivate : public TransportServerPrivate
  {
  public:
    TransportServerLibEventPrivate(TransportServer* self,
                                   const qi::Url &url,
                                   EventLoop* ctx);
    virtual ~TransportServerLibEventPrivate();

    virtual bool listen();
    virtual void close();
    virtual void destroy();
    void accept(evutil_socket_t        fd,
                struct evconnlistener *listener);

    void accept_error(struct evconnlistener *listener);

    //struct event_base     *_base;
    struct evconnlistener *_listener;
  private:
    TransportServerLibEventPrivate() {};
  };
}

#endif  // _QIMESSAGING_TRANSPORT_SERVER_LIBEVENT_P_HPP_
