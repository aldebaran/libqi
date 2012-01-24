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

#ifndef   	TRANSPORT_SERVER_HPP_
# define   	TRANSPORT_SERVER_HPP_

# include <string>
# include <map>
# include <qi/macro.hpp>

# include <event2/event.h>
# include <event2/bufferevent.h>

class TransportServer;
class TransportServerPrivate;

struct ClientConnection
{
  explicit ClientConnection(const std::string &id,
                            struct bufferevent *bev,
                            TransportServer *parent)
    : _id(id)
    , _bev(bev)
    , _parent(parent)
  {
  }

  ~ClientConnection()
  {
    bufferevent_free(_bev);
  }

  std::string         _id;
  struct bufferevent *_bev;
  // We need the parent to call the differents callback
  // with the right client connnection ID
  // (we don't want to loose some connected client)
  TransportServer    *_parent;
};

typedef std::map<std::string, ClientConnection*>           ClientConnectionMap;
typedef std::map<std::string, ClientConnection*>::iterator ClientConnectionMapIterator;

class TransportServerDelegate
{
public:
  virtual void onConnected(const std::string &msg = "") = 0;
  virtual void onWrite(const std::string &msg = "")     = 0;
  virtual void onRead(const std::string &msg = "")      = 0;
};

class TransportServer
{
  QI_DISALLOW_COPY_AND_ASSIGN(TransportServer);

public:
  TransportServer();
  virtual ~TransportServer();

  void start(const std::string &address,
             unsigned short port,
             struct event_base *base);
  void setDelegate(TransportServerDelegate *delegate);

  void accept(evutil_socket_t listener,
              short events,
              void *context);
  void readcb(struct bufferevent *bev,
              void *context);
  void writecb(struct bufferevent* bev,
               void* context);
  void eventcb(struct bufferevent *bev,
               short events,
               void *context);

  // Map of connected client
  ClientConnectionMap  clientConnected;

private:
  TransportServerPrivate *_p;
};


#endif	    /* !TRANSPORT_SERVER_HPP_ */
