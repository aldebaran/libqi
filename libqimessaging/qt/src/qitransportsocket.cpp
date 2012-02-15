/*
** Author(s):
**  - Laurent LEC   <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/qitransportsocket.h>


QiTransportSocket::QiTransportSocket()
{
  _p = new QiTransportSocketPrivate();
  _p->socket = new qi::TransportSocket();
}

QiTransportSocket::QiTransportSocket(int fd, struct event_base *base)
{
  _p = new QiTransportSocketPrivate();
  _p->socket = new qi::TransportSocket(fd, base);
}

QiTransportSocket::~QiTransportSocket()
{
  delete _p->socket;
  delete _p;
}

bool QiTransportSocket::connect(const std::string &address,
                                unsigned short port,
                                struct event_base *base)
{
  return _p->socket->connect(address, port, base);
}

void QiTransportSocket::disconnect()
{
  _p->socket->disconnect();
}
