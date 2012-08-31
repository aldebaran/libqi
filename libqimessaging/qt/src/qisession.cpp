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
#include <qimessaging/qt/qidatastream.h>
#include "src/qiremoteobject_p.h"
#include "src/qisession_p.h"

QiSessionPrivate::QiSessionPrivate(QiSession *self) {
  _session = new qi::Session();
  _session->addCallbacks(this);
  _self = self;
  _serviceSocket = new qi::TransportSocket();
  _serviceSocket->addCallbacks(this);
}

QiSessionPrivate::~QiSessionPrivate() {
  _serviceSocket->disconnect();
  delete _session;
  delete _serviceSocket;
}

void QiSessionPrivate::onServiceRegistered(qi::Session *QI_UNUSED(session),
                                           const std::string &serviceName)
{
  emit(_self->serviceRegistered(QString::fromUtf8(serviceName.c_str())));
}

void QiSessionPrivate::onServiceUnregistered(qi::Session *QI_UNUSED(session),
                                             const std::string &serviceName)
{
  emit(_self->serviceUnregistered(QString::fromUtf8(serviceName.c_str())));
}

void QiSessionPrivate::onSocketConnected(qi::TransportSocket *client, void *data) {
  QMap<void *, ServiceRequest>::iterator it = _futureConnect.find(client);

  if (it == _futureConnect.end())
    return;

  ServiceRequest &sr = it.value();
  qi::Message msg;
  msg.setType(qi::Message::Type_Call);
  msg.setService(sr.serviceId);
  msg.setObject(qi::Message::Object_Main);
  msg.setFunction(qi::Message::Function_MetaObject);
  _futureService[msg.id()] = sr;
  _futureConnect.remove(client);
  client->send(msg);
}

void QiSessionPrivate::onSocketConnectionError(qi::TransportSocket *client, void *data) {
  QMap<void *, ServiceRequest>::iterator it = _futureConnect.find(client);

  if (it == _futureConnect.end())
    return;

  it.value().fu.reportCanceled();
  _futureConnect.remove(client);
}

void QiSessionPrivate::onSocketReadyRead(qi::TransportSocket *client, int id, void *data) {
  qi::Message                                                        msg;
  QMap<int, ServiceRequest>::iterator                                it;
  QMap<int, QFutureInterface< QVector<qi::ServiceInfo> > >::iterator it2;
  client->read(id, &msg);

  it = _futureService.find(id);
  if (it != _futureService.end()) {
    if (client == _serviceSocket)
      service_ep_end(id, client, &msg, it.value());
    else
      service_mo_end(id, client, &msg, it.value());
    return;
  }
  it2 = _futureServices.find(id);
  if (it2 != _futureServices.end()) {
    services_end(client, &msg, it2.value());
    _futureServices.erase(it2);
    return;
  }
}

//rep from master (endpoint)
void QiSessionPrivate::service_ep_end(int id, qi::TransportSocket *QI_UNUSED(client), qi::Message *msg, ServiceRequest &sr) {
  qi::ServiceInfo      si;
  qi::IDataStream       d(msg->buffer());
  d >> si;

  sr.serviceId = si.serviceId();
  for (std::vector<std::string>::const_iterator it = si.endpoints().begin(); it != si.endpoints().end(); ++it)
  {
    qi::Url url(*it);
    if (sr.protocol == "any" || sr.protocol == QString::fromStdString(url.protocol()))
    {
      qi::TransportSocket* ts = NULL;
      ts = new qi::TransportSocket();
      ts->addCallbacks(this);
      _futureConnect[ts] = sr;
      _futureService.remove(id);
      ts->connect(url);
      return;
    }
    _futureService.remove(id);
    sr.fu.reportCanceled();
    return;
  }
}

void QiSessionPrivate::service_mo_end(int QI_UNUSED(id), qi::TransportSocket *client, qi::Message *msg, ServiceRequest &sr) {
  qi::MetaObject mo;
  qi::IDataStream ds(msg->buffer());
  ds >> mo;
  //remove the delegate on us, now the socket is owned by QiRemoteObject
  client->removeCallbacks(this);
  QiRemoteObject *robj = new QiRemoteObject(client, sr.name.toUtf8().constData(), sr.serviceId, mo);
  sr.fu.reportResult(robj);
}

void QiSessionPrivate::services_end(qi::TransportSocket *QI_UNUSED(client), qi::Message *msg, QFutureInterface< QVector<qi::ServiceInfo> > &fut) {
  QVector<qi::ServiceInfo> result;
  qi::IDataStream d(msg->buffer());
  d >> result;
  fut.reportResult(result);
}


QiSession::QiSession()
  : _p(new QiSessionPrivate(this))
{
}


QiSession::~QiSession()
{
  delete _p;
}

bool QiSession::connect(const QString &masterAddress)
{
  return _p->_serviceSocket->connect(masterAddress.toUtf8().constData());
}

bool QiSession::disconnect()
{
  _p->_serviceSocket->disconnect();
  return true;
}

bool QiSession::waitForConnected(int msecs)
{
  return _p->_serviceSocket->waitForConnected(msecs);
}

bool QiSession::waitForDisconnected(int msecs)
{
  return _p->_serviceSocket->waitForDisconnected(msecs);
}

//1 call: req epinfo to the master
//2 msg from serv: create a socket, connect
//3 socket co: send msg for metadata
//4
QFuture<QObject *> QiSession::service(const QString &name, const QString &type) {
  ServiceRequest sr;
  qi::Message    msg;
  qi::Buffer     buf;

  msg.setBuffer(buf);
  sr.fu.reportStarted();
  sr.name      = name;
  sr.protocol  = type;

  msg.setType(qi::Message::Type_Call);
  msg.setService(qi::Message::Service_ServiceDirectory);
  msg.setObject(qi::Message::Object_Main);
  msg.setFunction(qi::Message::ServiceDirectoryFunction_Service);
  qi::ODataStream dr(buf);
  dr << name.toUtf8().constData();
  _p->_futureService[msg.id()] = sr;
  _p->_serviceSocket->send(msg);
  return sr.fu.future();
}

QFuture< QVector<qi::ServiceInfo> > QiSession::services()
{
  QFutureInterface< QVector<qi::ServiceInfo> > futi;
  qi::Message msg;

  futi.reportStarted();
  msg.setType(qi::Message::Type_Call);
  msg.setService(qi::Message::Service_ServiceDirectory);
  msg.setObject(qi::Message::Object_Main);
  msg.setFunction(qi::Message::ServiceDirectoryFunction_Services);
  _p->_futureServices[msg.id()] = futi;
  _p->_serviceSocket->send(msg);
  return futi.future();
}

QUrl QiSession::url() const {
  QUrl url(QString::fromUtf8(_p->_session->url().str().c_str()));
  return url;
}
