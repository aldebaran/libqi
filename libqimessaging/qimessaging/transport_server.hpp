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

  class QIMESSAGING_API TransportServerInterface {
  public:
    virtual ~TransportServerInterface() = 0;
    virtual void newConnection() = 0;
  };

  class Session;
  class TransportSocket;
  class TransportServerPrivate;

  class QIMESSAGING_API TransportServer
  {
    QI_DISALLOW_COPY_AND_ASSIGN(TransportServer);

  public:
    TransportServer();
    virtual ~TransportServer();

    void setCallbacks(TransportServerInterface *delegate);

    bool start(qi::Session *session, const qi::Url &url);

    TransportSocket *nextPendingConnection();

    TransportServerPrivate *_p;
  };

}

#endif  // _QIMESSAGING_TRANSPORT_SERVER_HPP_
