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
#include "src/session_p.hpp"
#include "src/network_thread.hpp"

QiSessionPrivate::QiSessionPrivate() {
  _networkThread = new qi::NetworkThread();
  _serviceSocket = new qi::TransportSocket();
  _serviceSocket->setDelegate(this);
}

QiSessionPrivate::~QiSessionPrivate() {
  _serviceSocket->disconnect();
  delete _networkThread;
  delete _serviceSocket;
}

void QiSessionPrivate::onConnected(qi::TransportSocket *client) {
  QMap<void *, ServiceRequest>::iterator it = _futureConnect.find(client);

  if (it == _futureConnect.end())
    return;

  ServiceRequest &sr = it.value();
  qi::Message msg;
  msg.setType(qi::Message::Type_Call);
  msg.setService(sr.serviceId);
  msg.setPath(qi::Message::Path_Main);
  msg.setFunction(qi::Message::Function_MetaObject);
  _futureService[msg.id()] = sr;
  _futureConnect.remove(client);
  client->send(msg);
}

//TODO: handle connection error
void QiSessionPrivate::onDisconnected(qi::TransportSocket *client) {

}

void QiSessionPrivate::onWriteDone(qi::TransportSocket *client) {
}

void QiSessionPrivate::onReadyRead(qi::TransportSocket *client, int id) {
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
void QiSessionPrivate::service_ep_end(int id, qi::TransportSocket *client, qi::Message *msg, ServiceRequest &sr) {
  qi::ServiceInfo      si;
  qi::DataStream       d(msg->buffer());
  qi::TransportSocket* ts = NULL;
  d >> si;

  sr.serviceId = si.serviceId();
  for (std::vector<std::string>::const_iterator it = si.endpoints().begin(); it != si.endpoints().end(); ++it)
  {
    qi::Url url(*it);
    if (sr.protocol == qi::Url::Protocol_Any || sr.protocol == url.protocol())
    {
      qi::TransportSocket* ts = NULL;
      ts = new qi::TransportSocket();
      ts->setDelegate(this);
      _futureConnect[ts] = sr;
      _futureService.remove(id);
      ts->connect(url, _networkThread->getEventBase());
      return;
    }
    _futureService.remove(id);
    sr.fu.reportCanceled();
    return;
  }
}

void QiSessionPrivate::service_mo_end(int id, qi::TransportSocket *client, qi::Message *msg, ServiceRequest &sr) {
  qi::MetaObject mo;
  qi::DataStream ds(msg->buffer());
  ds >> mo;
  //remove the delegate on us, now the socket is owned by QiRemoteObject
  client->setDelegate(0);
  QiRemoteObject *robj = new QiRemoteObject(client, sr.name.toUtf8().constData(), sr.serviceId, mo);
  sr.fu.reportResult(robj);
}

void QiSessionPrivate::services_end(qi::TransportSocket *client, qi::Message *msg, QFutureInterface< QVector<qi::ServiceInfo> > &fut) {
  QVector<qi::ServiceInfo> result;
  qi::DataStream d(msg->buffer());
  d >> result;
  fut.reportResult(result);
}


QiSession::QiSession()
  : _p(new QiSessionPrivate)
{
}


QiSession::~QiSession()
{
  delete _p;
}


bool QiSession::connect(const QString &masterAddress)
{
  return _p->_serviceSocket->connect(masterAddress.toUtf8().constData(), _p->_networkThread->getEventBase());
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
QFuture<QObject *> QiSession::service(const QString &name, qi::Url::Protocol type) {
  ServiceRequest sr;
  qi::Message    msg;

  sr.fu.reportStarted();
  sr.name      = name;
  sr.protocol  = type;

  msg.setType(qi::Message::Type_Call);
  msg.setService(qi::Message::Service_ServiceDirectory);
  msg.setPath(qi::Message::Path_Main);
  msg.setFunction(qi::Message::ServiceDirectoryFunction_Service);
  qi::DataStream dr(msg.buffer());
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
  msg.setPath(qi::Message::Path_Main);
  msg.setFunction(qi::Message::ServiceDirectoryFunction_Services);
  _p->_futureServices[msg.id()] = futi;
  _p->_serviceSocket->send(msg);
  return futi.future();
}
