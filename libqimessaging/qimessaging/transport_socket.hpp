/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_TRANSPORT_SOCKET_HPP_
#define _QIMESSAGING_TRANSPORT_SOCKET_HPP_

# include <string>
# include <qi/macro.hpp>

# include <event2/event.h>
# include <event2/bufferevent.h>
# include <qimessaging/message.hpp>
# include <qimessaging/url.hpp>

namespace qi {

class TransportSocket;

class TransportSocketInterface
{
public:
  virtual ~TransportSocketInterface()                                    {};
  inline virtual void onSocketConnected(TransportSocket *client)         {};
  inline virtual void onSocketConnectionError(TransportSocket *client)   {};
  inline virtual void onSocketDisconnected(TransportSocket *client)      {};
  inline virtual void onSocketWriteDone(TransportSocket *client)         {};
  inline virtual void onSocketReadyRead(TransportSocket *client, int id) {};
};

struct TransportSocketPrivate;

class TransportSocket
{
  QI_DISALLOW_COPY_AND_ASSIGN(TransportSocket);

public:
  TransportSocket();
  TransportSocket(int fd, struct event_base *base);
  virtual ~TransportSocket();

  bool connect(const qi::Url     &url,
               struct event_base *base);
  void disconnect();

  bool isConnected();

  // if msecs < 0 no timeout
  bool waitForConnected(int msecs = 30000);
  bool waitForDisconnected(int msecs = 30000);
  bool waitForId(int id, int msecs = 30000);

  bool read(int id, qi::Message *msg);

  bool send(qi::Message &msg);

  void setDelegate(TransportSocketInterface *delegate);

  void readcb(struct bufferevent *bev,
              void *context);
  void writecb(struct bufferevent* bev,
               void* context);
  void eventcb(struct bufferevent *bev,
               short error,
               void *context);
private:
  TransportSocketPrivate  *_p;
};

}

#endif  // _QIMESSAGING_TRANSPORT_SOCKET_HPP_
