/*
** transport-server.hpp
** Login : <hcuche@hcuche-de>
** Started on  Wed Jan 11 10:19:42 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#pragma once
#ifndef _QIMESSAGING_TRANSPORT_SERVER_HPP_
#define _QIMESSAGING_TRANSPORT_SERVER_HPP_

# include <qimessaging/api.hpp>
# include <qimessaging/url.hpp>

namespace qi {

  class TransportSocket;
  class TransportServer;
  class QIMESSAGING_API TransportServerInterface {
  public:
    virtual ~TransportServerInterface() = 0;
    virtual void newConnection(TransportServer* server, TransportSocket *socket) = 0;
    virtual void error(TransportServer* server, int err) {}
  };

  class Session;
  class TransportServerPrivate;

  class QIMESSAGING_API TransportServer
  {
    QI_DISALLOW_COPY_AND_ASSIGN(TransportServer);

  public:
    TransportServer();
    TransportServer(qi::Session *session, const qi::Url &url);
    virtual ~TransportServer();

    void addCallbacks(TransportServerInterface *delegate);
    void removeCallbacks(TransportServerInterface *delegate);

    bool listen();
    bool listen(qi::Session *session, const qi::Url &url);
    void join();
    bool close();

    qi::Url listenUrl() const;
    std::vector<qi::Url> endpoints() const;

    TransportServerPrivate *_p;
  };

}

#endif  // _QIMESSAGING_TRANSPORT_SERVER_HPP_
