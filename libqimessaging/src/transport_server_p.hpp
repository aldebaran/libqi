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
# include <qimessaging/url.hpp>
# include <qimessaging/session.hpp>

# include <string>
# include <queue>

# include <qimessaging/transport_server.hpp>

namespace qi {

  class TransportServerPrivate
  {
  public:
    TransportServerPrivate(qi::Session *session, const qi::Url &url)
      : tsi(0)
      , mainSession(session)
      , listenUrl(url)
    {}

    virtual ~TransportServerPrivate()
    {
    }

    virtual bool listen() = 0;
    virtual bool close() = 0;

  public:
    std::queue<qi::TransportSocket*> connection;
    TransportServerInterface        *tsi;
    qi::Session                     *mainSession;
    qi::Url                          listenUrl;

  protected:
    TransportServerPrivate()
      : tsi(0)
      , mainSession(NULL)
      , listenUrl("")
    {};
  };

}

#endif  // _QIMESSAGING_TRANSPORT_SERVER_P_HPP_
