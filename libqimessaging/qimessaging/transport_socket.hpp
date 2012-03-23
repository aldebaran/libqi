/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_TRANSPORT_SOCKET_HPP_
#define _QIMESSAGING_TRANSPORT_SOCKET_HPP_

# include <qimessaging/api.hpp>
# include <qimessaging/message.hpp>
# include <qimessaging/url.hpp>
# include <string>

namespace qi
{
  class TransportSocket;
  class QIMESSAGING_API TransportSocketInterface
  {
  public:
    virtual ~TransportSocketInterface() = 0;
    inline virtual void onSocketConnected(TransportSocket *QI_UNUSED(client))         {};
    inline virtual void onSocketConnectionError(TransportSocket *QI_UNUSED(client))   {};
    inline virtual void onSocketDisconnected(TransportSocket *QI_UNUSED(client))      {};
    inline virtual void onSocketWriteDone(TransportSocket *QI_UNUSED(client))         {};
    inline virtual void onSocketReadyRead(TransportSocket *QI_UNUSED(client),
                                          int QI_UNUSED(id)) {};
  };

  class Session;
  class TransportSocketPrivate;
  class QIMESSAGING_API TransportSocket
  {
    QI_DISALLOW_COPY_AND_ASSIGN(TransportSocket);

  public:
    TransportSocket();
    virtual ~TransportSocket();

    bool connect(qi::Session *session, const qi::Url &url);
    void disconnect();


    // if msecs < 0 no timeout
    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);
    bool waitForId(int id, int msecs = 30000);

    bool read(int id, qi::Message *msg);
    bool send(const qi::Message &msg);

    void setCallbacks(TransportSocketInterface *delegate);
    bool isConnected();

    TransportSocketPrivate *_p;
  };

}

#endif  // _QIMESSAGING_TRANSPORT_SOCKET_HPP_
