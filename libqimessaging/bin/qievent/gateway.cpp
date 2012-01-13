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

#include "gateway.hpp"

Gateway::Gateway()
{
  _ts = new TransportServer("127.0.0.1", 9559);
  _ts->setDelegate(this);
}

Gateway::~Gateway()
{
  delete _ts;
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
  Message m = parseMessage(msg);

  switch (m.type)
  {
  case Message::call:
    break;
  case Message::answer:
    break;
  case Message::event:
    break;
  case Message::error:
  default:
    break;
  }
}

Message Gateway::parseMessage(const std::string &msg)
{
  std::string res;
  size_t begin = 0;
  size_t end;

  Message m;

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
