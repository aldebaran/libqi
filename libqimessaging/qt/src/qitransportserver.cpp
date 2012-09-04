/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "src/qitransportserver_p.h"
#include "src/qitransportsocket_p.h"
#include <QTcpSocket>

QiTransportServerPrivate::QiTransportServerPrivate(QObject* parent,
                                                   QiTransportServer* self)
  : QObject(parent)
  , _self(self)
{
}

QiTransportServerPrivate::~QiTransportServerPrivate()
{
  _server.close();

  while (!_pendingConnections.empty())
  {
    delete _pendingConnections.dequeue();
  }
}

void QiTransportServerPrivate::acceptConnection()
{
  QTcpSocket* qsocket = _server.nextPendingConnection();
  if (qsocket == 0) // shouldn't happen
  {
    return;
  }

  QiTransportSocket* socket = new QiTransportSocket();
  connect(qsocket, SIGNAL(disconnected()), socket, SIGNAL(disconnected()));
  connect(qsocket, SIGNAL(readyRead()), socket->_p, SLOT(read()));
  socket->_p->_device = qsocket;
  _pendingConnections.enqueue(socket);

  emit newConnection();
}

QiTransportServer::QiTransportServer(QObject *parent)
  : QObject(parent)
  , _p(new QiTransportServerPrivate(this, this))
{
}

QiTransportServer::~QiTransportServer()
{
  delete _p;
}

bool QiTransportServer::listen(const QUrl &listenUrl)
{
  if (listenUrl.scheme() == "tcp")
  {
    QUrl listenUrl(listenUrl);

    if (listenUrl.port() == 0)
    {
      listenUrl.setPort(qi::os::findAvailablePort(0));
    }

    _p->_listeningUrl = listenUrl;
    QHostAddress host(listenUrl.host());
    QObject::connect(&_p->_server, SIGNAL(newConnection()),
                     _p, SLOT(acceptConnection()));
    QObject::connect(_p, SIGNAL(newConnection()),
                     _p->_self, SIGNAL(newConnection()));
    return _p->_server.listen(host, listenUrl.port());
  }

  qiLogError("QiTransportServer") << "Protocol `"
                                  << listenUrl.scheme().toUtf8().constData()
                                  << "' is not supported.";

  return false;
}

QUrl QiTransportServer::listeningUrl()
{
  return _p->_listeningUrl;
}

void QiTransportServer::close()
{
  _p->_server.close();
}

bool QiTransportServer::hasPendingConnections() const
{
  return !_p->_pendingConnections.empty();
}

QiTransportSocket* QiTransportServer::nextPendingConnection()
{
  return _p->_pendingConnections.empty() ? 0 : _p->_pendingConnections.dequeue();
}

bool QiTransportServer::isListening() const
{
  return _p->_server.isListening();
}
