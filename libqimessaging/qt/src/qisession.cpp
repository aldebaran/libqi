/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>
#include <qimessaging/qt/qisession.h>
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

QObject *QiSession::service(const QString &name, qi::Url::Protocol type)
{
  unsigned int idx = 0;
  qi::TransportSocket *ts = _p->session.serviceSocket(name.toUtf8().constData(), &idx, type);

  if (!ts)
    return 0;
  qi::Message msg;
  msg.setType(qi::Message::Type_Call);
  msg.setService(idx);
  msg.setPath(qi::Message::Path_Main);
  msg.setFunction(qi::Message::Function_MetaObject);

  ts->send(msg);
  ts->waitForId(msg.id());

  qi::Message *ret;
  ts->read(msg.id(), &ret);

  qi::MetaObject mo;

  qi::DataStream ds(ret->buffer());

  ds >> mo;

  QiRemoteObject *robj = new QiRemoteObject(ts, name.toUtf8().constData(), idx, mo);

  return static_cast<QObject *>(robj);

}

QVector<qi::ServiceInfo> QiSession::services()
{
  return QVector<qi::ServiceInfo>::fromStdVector(_p->session.services());
}
