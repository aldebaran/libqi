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


void QiSessionPrivate::onConnected(qi::TransportSocket *client) {
}

void QiSessionPrivate::onDisconnected(qi::TransportSocket *client) {
}

void QiSessionPrivate::onWriteDone(qi::TransportSocket *client) {
}

void QiSessionPrivate::onReadyRead(qi::TransportSocket *client, int id) {
  qi::Message                         msg;
  QMap<int, ServiceRequest>::iterator it;

  client->read(id, &msg);

  it = _futureService.find(id);
  if (it != _futureService.end()) {
    service_end(client, &msg, it.value());
    return;
  }
//  fut = _future_services[msg.id()];
//  if (fut) {
//    services_end(id, fut);
//    return;
//  }
}

void QiSessionPrivate::service_end(qi::TransportSocket *client, qi::Message *msg, ServiceRequest &sr) {
  qi::MetaObject mo;
  qi::DataStream ds(msg->buffer());
  ds >> mo;
  //remove the delegate on us, now the socket is owned by QiRemoteObject
  client->setDelegate(0);
  QiRemoteObject *robj = new QiRemoteObject(client, sr.name.toUtf8().constData(), sr.serviceId, mo);
  //notify future
  sr.fu.reportResult(robj);
}


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

QFuture<QObject *> QiSession::service(const QString &name, qi::Url::Protocol type) {
  ServiceRequest sr;
  unsigned int idx = 0;
  //todo: this lock
  qi::TransportSocket *ts = _p->session.serviceSocket(name.toUtf8().constData(), &idx, type);
  ts->setDelegate(_p);
  sr.fu.reportStarted();
  if (!ts) {
    sr.fu.reportCanceled();
    return sr.fu.future();
  }
  sr.name = name;
  sr.serviceId = idx;
  sr.name = name;
  sr.serviceId = idx;
  qi::Message msg;
  msg.setType(qi::Message::Type_Call);
  msg.setService(idx);
  msg.setPath(qi::Message::Path_Main);
  msg.setFunction(qi::Message::Function_MetaObject);
  //TODO: remove on answer
  ts->send(msg);
  _p->_futureService[msg.id()] = sr;
  return sr.fu.future();
}

QVector<qi::ServiceInfo> QiSession::services()
{
  return QVector<qi::ServiceInfo>::fromStdVector(_p->session.services());
}
