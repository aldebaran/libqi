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

# include "transport-client.hpp"
# include "transport-server.hpp"

class Message
{
public:
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
    idModule = "";
    idObject = "";
    msg = "";
  };

  Message(const Message &m)
  {
    type = m.type;
    size = m.size;
    idModule = m.idModule;
    idObject = m.idObject;
    msg = m.msg;
  };

  MessageType  type;
  unsigned int size;
  std::string  idModule;
  std::string  idObject;
  std::string  msg;
};

class Gateway: public TransportServerDelegate
{
public:
  Gateway();
  ~Gateway();

  void onConnected(const std::string &msg = "");
  void onWrite(const std::string &msg = "");
  void onRead(const std::string &msg = "");

private:
  void* call(const std::string &msg);
  void* answer(const std::string &msg);
  void* error(const std::string &msg);
  void* event(const std::string &msg);

  Message parseMessage(const std::string &msg);

private:
  TransportServer *_ts;
  std::map<std::string, TransportClient*> _clientMap;
};


#endif	    /* !GATEWAY_HPP_ */
