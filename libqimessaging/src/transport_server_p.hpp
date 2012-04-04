/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_TRANSPORT_SERVER_P_HPP_
#define _QIMESSAGING_TRANSPORT_SERVER_P_HPP_

# include <qimessaging/api.hpp>
# include <event2/event.h>
# include <event2/bufferevent.h>
# include <qimessaging/url.hpp>

# include <string>
# include <queue>

namespace qi {

  class TransportServerPrivate
  {
  public:
    TransportServerPrivate()
      : tsi(0)
      , base(0)
      , listenUrl("tcp://0.0.0.0:0")
    {
    }

    ~TransportServerPrivate()
    {
    }

    bool start(struct event_base *base, const qi::Url &url);

    void accept(evutil_socket_t        fd,
                struct evconnlistener *listener,
                void                  *context);

    std::queue<qi::TransportSocket*>  connection;
    TransportServerInterface          *tsi;
    struct event_base                 *base;
    qi::Url                            listenUrl;

  };

}

#endif  // _QIMESSAGING_TRANSPORT_SERVER_P_HPP_
