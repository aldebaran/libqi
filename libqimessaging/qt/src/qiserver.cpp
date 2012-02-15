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

void QiServer::listen(QiSession *session, const QUrl &url)
{
  _p->server.listen(&(session->_p->session), url.toString().toUtf8().constData());
}

void QiServer::stop()
{
}

void QiServer::advertiseService(const QString &name, QObject *obj)
{

}
