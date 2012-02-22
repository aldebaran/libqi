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
#include <qimessaging/object.hpp>
#include <qimessaging/datastream.hpp>
#include "src/qiremoteobject_p.h"
#include "src/qisession_p.h"


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

void QiSession::disconnect()
{
  _p->session.disconnect();
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
  unsigned int idx = 0;
  qi::TransportSocket *ts = _p->session.serviceSocket(name.toUtf8().constData(), &idx, type.toUtf8().constData());

  if (!ts)
    return 0;
  qi::Message msg;
  msg.setType(qi::Message::Call);
  msg.setPath(qi::Message::ServiceDirectory);
  qi::DataStream dout(msg.buffer());
  dout << "__metaobject";
  dout << name.toUtf8().constData();

  ts->send(msg);
  ts->waitForId(msg.id());

  qi::Message ret;
  ts->read(msg.id(), &ret);

  qi::MetaObject mo;

  qi::DataStream ds(ret.buffer());

  ds >> mo;

  QiRemoteObject      *robj = new QiRemoteObject(ts, name.toUtf8().constData(), &mo);

  return static_cast<QObject *>(robj);

}

QVector<QString> QiSession::services()
{
  QVector<QString> ret;

  foreach (std::string str, _p->session.services())
    ret.push_back(QString::fromStdString(str));

  return ret;
}
