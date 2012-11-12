/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "qitransportserver_p.h"
#include "qitransportsocket_p.h"
#include <QTcpSocket>
#include <QSslSocket>
#include <QSslKey>

QTcpSocket* QTcpServerSsl::nextPendingConnection()
{
  return _pendingConnections.empty() ? 0 : _pendingConnections.dequeue();
}

void QTcpServerSsl::incomingConnection(int sd)
{
  QSslSocket* serverSocket = new QSslSocket;

  if (serverSocket->setSocketDescriptor(sd))
  {

    if (_ssl)
    {
      // ignore invalid certificates
      serverSocket->ignoreSslErrors();
      serverSocket->setProtocol(QSsl::AnyProtocol);
      serverSocket->setLocalCertificate(QSslCertificate(_certificate.constData()));
      serverSocket->setPrivateKey(QSslKey(_key.constData(), QSsl::Rsa));
      serverSocket->startServerEncryption();
    }

    _pendingConnections.enqueue(serverSocket);
  }
  else
  {
    delete serverSocket;
  }
}

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
  if (_listeningUrl.scheme() == "tcp" || _listeningUrl.scheme() == "tcps")
  {
    QTcpSocket* qsocket = _server.nextPendingConnection();
    if (qsocket == 0) // shouldn't happen
    {
      qiLogError("QiGateway") << "no pending socket from backend!";
      return;
    }

    QiTransportSocket* socket = new QiTransportSocket();
    connect(qsocket, SIGNAL(disconnected()), socket, SIGNAL(disconnected()));
    connect(qsocket, SIGNAL(readyRead()), socket->_p, SLOT(read()));
    socket->_p->_device = qsocket;
    _pendingConnections.enqueue(socket);
  }
  else
  {
    qiLogError("QiTransportServer") << "new incoming connection, but protocol `"
                                    << _listeningUrl.scheme().toUtf8().constData() << " is unknown";
    return;
  }

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
  qiLogDebug("QiTransportSocket") << "Will listen on " << listenUrl.toString().toUtf8().constData();

  if (listenUrl.scheme() == "tcp" || listenUrl.scheme() == "tcps")
  {
    QUrl listeningUrl(listenUrl);

    if (listeningUrl.port() == 0)
    {
      listeningUrl.setPort(qi::os::findAvailablePort(0));
    }

    _p->_listeningUrl = listeningUrl;
    QHostAddress host(listeningUrl.host());
    QObject::connect(&_p->_server, SIGNAL(newConnection()),
                     _p, SLOT(acceptConnection()));
    QObject::connect(_p, SIGNAL(newConnection()),
                     _p->_self, SIGNAL(newConnection()));

    _p->_server._ssl = listenUrl.scheme() == "tcps";
    return _p->_server.listen(host, listeningUrl.port());
  }

  qiLogError("QiTransportServer") << "Protocol `"
                                  << listenUrl.scheme().toUtf8().constData()
                                  << "' is not supported, can't listen";

  return false;
}

QUrl QiTransportServer::listeningUrl() const
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

void QiTransportServer::setIdentity(const QByteArray& key,
                                    const QByteArray& certificate)
{
  _p->_server._key = key;
  _p->_server._certificate = certificate;
}
