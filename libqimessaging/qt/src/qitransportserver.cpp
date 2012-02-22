/*
** Author(s):
**  - Laurent LEC   <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/qt/qitransportserver.h>
#include <qimessaging/transport_server.hpp>

class QiTransportServerPrivate
{
public:
  qi::TransportServer server;
};

void QiTransportServer::start(QString            address,
                              unsigned short     port,
                              struct event_base *base)
{
  _p->server.start(address.toUtf8().constData(), port, base);
  //_p->setDelegate(delegate);
}

QiTransportSocket *QiTransportServer::nextPendingConnection()
{
  qi::TransportSocket *sock = _p->server.nextPendingConnection();

  if (!sock)
  {
    return 0;
  }

  QiTransportSocket *ret = new QiTransportSocket();
  ret->_p->socket = sock;
  return ret;
}
