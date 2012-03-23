/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <QtCore/qurl.h>

#include <qimessaging/qt/qiserver.h>
#include <qimessaging/qt/qisession.h>
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

void QiServer::listen(QiSession *QI_UNUSED(session), const QVector<QUrl> &QI_UNUSED(url))
{
}

void QiServer::stop()
{
}

void QiServer::registerService(const QString &QI_UNUSED(name), QObject *QI_UNUSED(obj))
{
}
