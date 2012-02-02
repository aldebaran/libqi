/*
** gateway.cpp
** Login : <hcuche@hcuche-de>
** Started on  Thu Jan 12 19:18:32 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#include <iostream>
#include <cstdlib>
#include <map>

#include <event2/bufferevent.h>

#include <qimessaging/transport/gateway.hpp>
#include <qimessaging/transport/transport_socket.hpp>

namespace qi {

class GatewayPrivate
{
public:
  GatewayPrivate() {}
  ~GatewayPrivate() {}

  void* callMessage(const Message &msg,
                    const std::string &origMsg);
  void* answerMessage(const Message &msg,
                      const std::string &origMsg);
  void* errorMessage(const Message &msg,
                     const std::string &origMsg);
  void* eventMessage(const Message &msg,
                     const std::string &origMsg);

  Message parseMessage(const std::string &msg);

  NetworkThread     *nthd;
  TransportServer   *ts;
  TransportSocketMap clientMap;
};

Gateway::Gateway()
{
  _p = new GatewayPrivate();
  _p->ts = new TransportServer();
  _p->ts->setDelegate(this);
}

Gateway::~Gateway()
{
  delete _p->ts;
  delete _p;
}

void Gateway::onConnected(const qi::Message &msg)
{
  std::cout << "onConnect: " << msg.str() << std::endl;
}

void Gateway::onWrite(const qi::Message &msg)
{
  std::cout << "onWrite: " << msg.str() << std::endl;
}

void Gateway::onRead(const qi::Message &msg)
{
  // Parse msg
  std::cout << "onRead: " << msg.str() << std::endl;
  Message m = _p->parseMessage(msg.str());

  switch (m.type)
  {
  case Message::Call:
    _p->callMessage(m, msg.str());
    break;
  case Message::Answer:
    _p->answerMessage(m, msg.str());
    break;
  case Message::Event:
    _p->eventMessage(m, msg.str());
    break;
  case Message::Error:
    _p->errorMessage(m, msg.str());
  default:
    break;
  }
}


void Gateway::start(const std::string &address,
                    unsigned short port,
                    NetworkThread *n)
{
  _p->nthd = n;
  _p->ts->start(address, port, _p->nthd->getEventBase());
}

Message GatewayPrivate::parseMessage(const std::string &msg)
{
  std::string res;
  size_t begin = 0;
  size_t end;

  Message m;

  end = msg.find(".", begin);
  res = msg.substr(begin, end - begin);
  begin = end + 1;
  m.idCaller = res;

  end = msg.find(".", begin);
  res = msg.substr(begin, end - begin);
  begin = end + 1;
  if (res == "call")
    m.type = Message::Call;
  else if (res == "answer")
    m.type = Message::Answer;
  else if (res == "event")
    m.type = Message::Event;
  else if (res == "error")
    m.type = Message::Error;
  else
    return m;

  end = msg.find(".", begin);
  res = msg.substr(begin, end - begin);
  begin = end + 1;
  m.size = atoi(res.c_str());

  end = msg.find(".", begin);
  res = msg.substr(begin, end - begin);
  begin = end + 1;
  m.idModule = res;

  end = msg.find(".", begin);
  res = msg.substr(begin, end - begin);
  begin = end + 1;
  m.idObject = res;

  res = msg.substr(begin, std::string::npos);
  m._data = res;

  return m;
}

void* GatewayPrivate::callMessage(const Message &msg,
                                  const std::string &origMsg)
{
  TransportSocketMapIterator it;
  it = clientMap.find(msg.idModule);
  if (it != clientMap.end())
  {
    TransportSocket* tc = it->second;
    tc->send(origMsg);
  }
  else
  {
    // Get the module server from the master.
  }
}

void* GatewayPrivate::answerMessage(const Message &msg,
                                    const std::string &origMsg)
{
  // Get client id to send back call answer
  ClientConnectionMapIterator it;
  it = ts->clientConnected.find(msg.idCaller);
  if (it != ts->clientConnected.end())
  {
    ClientConnection* cc = it->second;
    if (!bufferevent_write(cc->_bev, origMsg.c_str(), origMsg.size()))
      return NULL;
  }
  return NULL;
}

void* GatewayPrivate::errorMessage(const Message &msg,
                                   const std::string &origMsg)
{
}

void* GatewayPrivate::eventMessage(const Message &msg,
                                   const std::string &origMsg)
{
}

}
