/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <qimessaging/qisession.h>

#include <qimessaging/session.hpp>

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
  _p.session.connect(masterAddress::toUtf8());
}

bool QiSession::waitForConnected(int msecs)
{
  _p.session.connect(masterAddress::toUtf8());
}

bool QiSession::waitForDisconnected(int msecs)
{
  _p.session.connect(masterAddress::toUtf8());
}


QObject *QiSession::service(const QString &name, const QString &type)
{

}


QiTransportSocket *QiSession::serviceSocket(const QString &name, const QString &type)
{
}

QVector<QString> services()
{
    return QVector<QString>::fromStdVector(_p.session.services());
}
