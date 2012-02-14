/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <qimessaging/qiserver.h>
#include <qimessaging/server.hpp>

class QiServerPrivate
{
public:
  qi::Server server;
};


QiServer::QiServer()
  : _p(new QiServerPrivate)
{
}

QiServer::~QiServer()
{
  delete _p;
}


void QiServer::start(const QString &addr, unsigned short port, NetworkThread *base)
{
  //_p->server.start(addr, port, base);
}

void QiServer::advertiseService(const QString &name, QObject *obj)
{

}


