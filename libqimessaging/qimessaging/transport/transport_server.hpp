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
# include <qimessaging/message.hpp>
# include <event2/event.h>
# include <event2/bufferevent.h>

# include <qimessaging/transport/transport_socket.hpp>

namespace qi {

class TransportServerInterface {
public:
  virtual void newConnection() = 0;
};

class TransportServerPrivate;
class TransportServer
{
  QI_DISALLOW_COPY_AND_ASSIGN(TransportServer);

public:
  TransportServer();
  virtual ~TransportServer();

  void setDelegate(TransportServerInterface *delegate);

  void start(const std::string &address,
             unsigned short port,
             struct event_base *base);


  TransportSocket *nextPendingConnection();

  TransportServerPrivate *_p;
};

}

#endif	    /* !TRANSPORT_SERVER_HPP_ */
