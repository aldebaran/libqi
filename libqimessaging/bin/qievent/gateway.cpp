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

#include "gateway.hpp"
#include "transport-client.hpp"


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
  TransportClientMap clientMap;
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

void Gateway::onConnected(const std::string &msg)
{
  std::cout << "onConnect: " << msg << std::endl;
}

void Gateway::onWrite(const std::string &msg)
{
  std::cout << "onWrite: " << msg << std::endl;
}

void Gateway::onRead(const std::string &msg)
{
  // Parse msg
  std::cout << "onRead: " << msg << std::endl;
  Message m = _p->parseMessage(msg);

  switch (m.type)
  {
  case Message::call:
    _p->callMessage(m, msg);
    break;
  case Message::answer:
    _p->answerMessage(m, msg);
    break;
  case Message::event:
    _p->eventMessage(m, msg);
    break;
  case Message::error:
    _p->errorMessage(m, msg);
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
    m.type = Message::call;
  else if (res == "answer")
    m.type = Message::answer;
  else if (res == "event")
    m.type = Message::event;
  else if (res == "error")
    m.type = Message::error;
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
  m.msg = res;

  return m;
}

void* GatewayPrivate::callMessage(const Message &msg,
                                  const std::string &origMsg)
{
  TransportClientMapIterator it;
  it = clientMap.find(msg.idModule);
  if (it != clientMap.end())
  {
    TransportClient* tc = it->second;
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

