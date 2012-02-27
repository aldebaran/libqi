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

void QiTransportServer::start(const QUrl        &url,
                              struct event_base *base)
{
  qi::Url urlo(url.toString().toUtf8().constData());
  _p->server.start(urlo, base);
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
