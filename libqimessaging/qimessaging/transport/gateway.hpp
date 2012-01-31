/*
** gateway.hpp
** Login : <hcuche@hcuche-de>
** Started on  Thu Jan 12 19:18:32 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#ifndef   	GATEWAY_HPP_
# define   	GATEWAY_HPP_

# include <string>
# include <map>

# include <qimessaging/transport/transport_socket.hpp>
# include <qimessaging/transport/transport_server.hpp>
# include <qimessaging/transport/network_thread.hpp>

namespace qi {

struct Message
{
  enum MessageType
  {
    call,
    answer,
    event,
    error,
    none
  };

  Message()
  {
    type = none;
    size = 0;
    idCaller = "";
    idModule = "";
    idObject = "";
    msg = "";
  }

  MessageType  type;
  unsigned int size;
  std::string  idCaller;
  std::string  idModule;
  std::string  idObject;
  std::string  msg;
};

typedef std::map<std::string, TransportSocket*>           TransportSocketMap;
typedef std::map<std::string, TransportSocket*>::iterator TransportSocketMapIterator;

class GatewayPrivate;

class Gateway: public TransportServerDelegate
{
public:
  Gateway();
  ~Gateway();

  void onConnected(const std::string &msg = "");
  void onWrite(const std::string &msg = "");
  void onRead(const std::string &msg = "");

  void start(const std::string &address,
             unsigned short port,
             NetworkThread *n);

private:
  GatewayPrivate *_p;
};

}

#endif	    /* !GATEWAY_HPP_ */
