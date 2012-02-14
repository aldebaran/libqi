/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <QString>
#include <qimessaging/qisession.h>
#include <qimessaging/session.hpp>
#include "src/qiremoteobject_p.h"

class QiSessionPrivate {
public:
  qi::Session session;
};


QiSession::QiSession()
  : _p(new QiSessionPrivate)
{
}

QiSession::~QiSession()
{
  delete _p;
}


void QiSession::connect(const QString &masterAddress)
{
  _p->session.connect(masterAddress.toUtf8().constData());
}

bool QiSession::waitForConnected(int msecs)
{
  return _p->session.waitForConnected(msecs);
}

bool QiSession::waitForDisconnected(int msecs)
{
  return _p->session.waitForDisconnected(msecs);
}

QObject *QiSession::service(const QString &name, const QString &type)
{
  qi::TransportSocket *ts = _p->session.serviceSocket(name.toUtf8().constData(), type.toUtf8().constData());
  QiRemoteObject *robj = new QiRemoteObject(ts, name.toUtf8().constData());
  QObject *obj = robj;
  return obj;
}

QVector<QString> QiSession::services()
{
  QVector<QString> ret;

  foreach (std::string str, _p->session.services())
    ret.push_back(QString::fromStdString(str));

  return ret;
}
