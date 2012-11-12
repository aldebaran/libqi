/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#if 0
#include <iostream>
#include <QtCore/qurl.h>

#include <qimessaging/qt/qiserver.h>
#include <qimessaging/qt/qisession.h>

#include "qisession_p.h"

class QiServerPrivate
{
public:
  qi::Server  server;
  QiSession  *session;
};

QiServer::QiServer()
  : _p(new QiServerPrivate)
{
}

QiServer::~QiServer()
{
  delete _p;
}

bool QiServer::listen(QiSession *session, const QUrl &url)
{
  qi::Url qurl(url.toString().toUtf8().data());
  std::cout << "url:" << qurl.str() << std::endl;
  _p->session = session;
  return _p->server.listen(_p->session->_p->_session, qurl.str());
}

void QiServer::close()
{
  _p->server.close();
}

QFuture<unsigned int> QiServer::registerService(const QString &QI_UNUSED(name), QObject *QI_UNUSED(obj))
{
  QFutureInterface<unsigned int> fut;
  return fut.future();
}

QUrl QiServer::listenUrl() const {
  QUrl url;
  return url;
}
#endif
