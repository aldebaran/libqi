/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <qimessaging/qiserver.h>
#include <qimessaging/qisession.h>
#include <qimessaging/server.hpp>
#include "src/qisession_p.h"

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


void QiServer::start(const QString &addr, unsigned short port, QiSession *session)
{
  _p->server.start(addr.toUtf8().constData(), port, &(session->_p->session));
}

void QiServer::advertiseService(const QString &name, QObject *obj)
{

}
