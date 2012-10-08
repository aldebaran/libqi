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
#include <qimessaging/genericobject.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/qt/qidatastream.h>
#include "src/qiremoteobject_p.h"
#include "src/qisession_p.h"

QiSessionPrivate::QiSessionPrivate(QiSession *self) {
  _session = new qi::Session();
  //_session->addCallbacks(this, 0);
  _self = self;
}

QiSessionPrivate::~QiSessionPrivate() {
  _serviceSocket->disconnect();
  delete _session;
}

void QiSessionPrivate::onServiceRegistered(qi::Session *QI_UNUSED(session),
                                           const std::string &serviceName,
                                           void *data)
{
  Q_EMIT _self->serviceRegistered(QString::fromUtf8(serviceName.c_str()));
}

void QiSessionPrivate::onServiceUnregistered(qi::Session *QI_UNUSED(session),
                                             const std::string &serviceName,
                                             void *data)
{
  Q_EMIT _self->serviceUnregistered(QString::fromUtf8(serviceName.c_str()));
}

void QiSessionPrivate::onSocketConnected(qi::TransportSocketPtr client) {
  QMap<qi::TransportSocketPtr, ServiceRequest>::iterator it = _futureConnect.find(client);

  if (it == _futureConnect.end())
    return;

  ServiceRequest &sr = it.value();
  qi::Message msg;
  msg.setType(qi::Message::Type_Call);
  msg.setService(qi::Message::Service_Server);
  msg.setObject(qi::Message::GenericObject_Main);
  msg.setFunction(qi::Message::BoundObjectFunction_MetaObject);
  qi::Buffer      buf;
  qi::ODataStream ds(buf);
  ds << sr.serviceId;
  ds << qi::Message::GenericObject_Main;
  msg.setBuffer(buf);
  _futureService[msg.id()] = sr;
  _futureConnect.remove(client);
  client->send(msg);
}

void QiSessionPrivate::onSocketDisconnected(qi::TransportSocketPtr client, int error) {
  QMap<qi::TransportSocketPtr, ServiceRequest>::iterator it = _futureConnect.find(client);

  if (it == _futureConnect.end())
    return;

  it.value().fu.reportCanceled();
  _futureConnect.remove(client);
}

void QiSessionPrivate::onMessageReady(const qi::Message &msg, qi::TransportSocketPtr client) {
  QMap<int, ServiceRequest>::iterator                                it;
  QMap<int, QFutureInterface< QVector<qi::ServiceInfo> > >::iterator it2;

  it = _futureService.find(msg.id());
  if (it != _futureService.end()) {
    if (client == _serviceSocket)
      service_ep_end(msg.id(), client, &msg, it.value());
    else
      service_mo_end(msg.id(), client, &msg, it.value());
    return;
  }
  it2 = _futureServices.find(msg.id());
  if (it2 != _futureServices.end()) {
    services_end(client, &msg, it2.value());
    _futureServices.erase(it2);
    return;
  }
}

//rep from master (endpoint)
void QiSessionPrivate::service_ep_end(int id, qi::TransportSocketPtr QI_UNUSED(client), const qi::Message *msg, ServiceRequest &sr) {
  qi::ServiceInfo      si;
  qi::IDataStream       d(msg->buffer());
  d >> si;

  sr.serviceId = si.serviceId();
  for (std::vector<std::string>::const_iterator it = si.endpoints().begin(); it != si.endpoints().end(); ++it)
  {
    qi::Url url(*it);
    if (sr.protocol == "any" || sr.protocol == QString::fromStdString(url.protocol()))
    {
      qi::TransportSocketPtr socket;
      socket = qi::makeTransportSocket(url.protocol());
      socket->connected.connect(boost::bind<void>(&QiSessionPrivate::onSocketConnected, this, _serviceSocket));
      socket->disconnected.connect(boost::bind<void>(&QiSessionPrivate::onSocketDisconnected, this, _serviceSocket, _1));
      socket->messageReady.connect(boost::bind<void>(&QiSessionPrivate::onMessageReady, this, _1, _serviceSocket));

      _futureConnect[socket] = sr;
      _futureService.remove(id);
      socket->connect(url);
      return;
    }
    _futureService.remove(id);
    sr.fu.reportCanceled();
    return;
  }
}

void QiSessionPrivate::service_mo_end(int QI_UNUSED(id), qi::TransportSocketPtr client, const qi::Message *msg, ServiceRequest &sr) {
  qi::MetaObject mo;
  qi::IDataStream ds(msg->buffer());
  ds >> mo;
  //remove the delegate on us, now the socket is owned by QiRemoteObject
  //TODO: client->removeCallbacks(this);
  QiRemoteObject *robj = new QiRemoteObject(client, sr.name.toUtf8().constData(), sr.serviceId, mo);
  sr.fu.reportResult(robj);
}

void QiSessionPrivate::services_end(qi::TransportSocketPtr QI_UNUSED(client), const qi::Message *msg, QFutureInterface< QVector<qi::ServiceInfo> > &fut) {
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
  _p->_serviceSocket = qi::makeTransportSocket(qi::Url(masterAddress.toStdString()).protocol());
  _p->_serviceSocket->connected.connect(boost::bind<void>(&QiSessionPrivate::onSocketConnected, this->_p, _p->_serviceSocket));
  _p->_serviceSocket->disconnected.connect(boost::bind<void>(&QiSessionPrivate::onSocketDisconnected, this->_p, _p->_serviceSocket, _1));
  _p->_serviceSocket->messageReady.connect(boost::bind<void>(&QiSessionPrivate::onMessageReady, this->_p, _1, _p->_serviceSocket));

  return _p->_serviceSocket->connect(masterAddress.toUtf8().constData());
}

bool QiSession::disconnect()
{
  _p->_serviceSocket->disconnect();
  return true;
}

bool QiSession::waitForConnected(int msecs)
{
  return true; // Dead class
}

bool QiSession::waitForDisconnected(int msecs)
{
  return true; // dead class
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
  msg.setObject(qi::Message::GenericObject_Main);
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
  msg.setObject(qi::Message::GenericObject_Main);
  msg.setFunction(qi::Message::ServiceDirectoryFunction_Services);
  _p->_futureServices[msg.id()] = futi;
  _p->_serviceSocket->send(msg);
  return futi.future();
}

QUrl QiSession::url() const {
  QUrl url(QString::fromUtf8(_p->_session->url().str().c_str()));
  return url;
}
