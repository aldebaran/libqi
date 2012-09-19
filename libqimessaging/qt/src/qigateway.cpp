/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include "src/qigateway_p.h"
#include <qimessaging/message.hpp>
#include <qimessaging/serviceinfo.hpp>


QiGatewayPrivate::QiGatewayPrivate(QObject *parent, Type type)
  : QObject(parent)
  , _type(type)
{
  connect(&_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

QiGatewayPrivate::~QiGatewayPrivate()
{
  close();
}

// Socket created by the QiTransportServer
void QiGatewayPrivate::newConnection()
{
  qiLogFatal("QiGateway") << "newConnection";
  QiTransportSocket* socket = _server.nextPendingConnection();
  _sockets.push_back(socket);

  connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
  connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
}

/*
 * Callback triggered when Gateway or ReverseGateway have established
 * a connection to the ServiceDirectory or to another service, or when
 * the ReverseGateway has reached a RemoteGateway.
 */
void QiGatewayPrivate::socketConnected()
{
  qiLogFatal("QiGateway") << "socketConnected";

  // flush pending messages
  for (QMap<unsigned int, RemoteService>::iterator servicesIt = _services.begin();
       servicesIt != _services.end();
       ++servicesIt)
  {
    if (servicesIt->socket->state() != QiTransportSocket::SocketState_Connected ||
        servicesIt->pendingMessages.empty())
    {
      continue;
    }

    for (QList<QPair<QiTransportSocket*, const qi::Message*> >::iterator pendingMessagesIt = servicesIt->pendingMessages.begin();
         pendingMessagesIt != servicesIt->pendingMessages.begin();
         ++pendingMessagesIt)
    {
      routeMessage(pendingMessagesIt->first, servicesIt->socket, *pendingMessagesIt->second);
    }
  }

  // was it a remote gateway? Send connect msg
}

void QiGatewayPrivate::socketDisconnected()
{
  qiLogFatal("QiGateway") << "socketDisconnected";
}

void QiGatewayPrivate::processClientMessage(QiTransportSocket* socket,
                                            const qi::Message* msg)
{
  QMap<unsigned int, RemoteService>::iterator service = _services.find(msg->service());
  if (service == _services.end())
  {
    qiLogError("QiGateway") << "The gateway doesn't know' service #"
                            << msg->id();
    return;
  }

  if (service->socket->state() == QiTransportSocket::SocketState_Unconnected)
  {
    qiLogError("QiGateway") << "The socket to service #" << service->serviceId
                            << " is not connected, will try to reconnect to "
                            << service->socket->peer().toString().toUtf8().constData();
    service->socket->connectToHost(service->socket->peer());
    service->pendingMessages.push_back(qMakePair(socket, msg));
    return;
  }

  routeMessage(socket, service->socket, *msg);

  if (msg->service() == qi::Message::Service_ServiceDirectory &&
      msg->function() == qi::Message::ServiceDirectoryFunction_Service &&
      msg->type() == qi::Message::Type_Call)
  {
    _services[qi::Message::Service_ServiceDirectory].pendingMessages.push_back(qMakePair<QiTransportSocket*, const qi::Message*>(socket, msg));
  }
}

void QiGatewayPrivate::processServiceMessage(QiTransportSocket* socket,
                                             const qi::Message& msg)
{
  if (msg.service() == qi::Message::Service_ServiceDirectory &&
      msg.function() == qi::Message::ServiceDirectoryFunction_Service &&
      msg.type() == qi::Message::Type_Reply)
  {
    qi::ServiceInfo    serviceInfo;
    qi::IDataStream ds(msg.buffer());
    ds >> serviceInfo;

    QUrl url = QString(serviceInfo.endpoints()[0].c_str());
    connectToService(serviceInfo.serviceId(), url);
    _services[serviceInfo.serviceId()].serviceName = QString(serviceInfo.name().c_str());

    std::vector<std::string> eps;
    eps.push_back(_server.listeningUrl().toString().toUtf8().constData());
    serviceInfo.setEndpoints(eps);
    serviceInfo.setMachineId(qi::os::getMachineId());

    qi::Message ans;
    qi::Buffer buf;
    ans.setBuffer(buf);
    ans.buildReplyFrom(msg);
    qi::ODataStream dsAns(buf);
    dsAns << serviceInfo;

    for (QList<QPair<QiTransportSocket*, const qi::Message*> >::iterator it = _services[qi::Message::Service_ServiceDirectory].pendingMessages.begin();
         it != _services[qi::Message::Service_ServiceDirectory].pendingMessages.end();
         ++it)
    {
      const qi::Message* msg = it->second;
      if (msg->service() == qi::Message::Service_ServiceDirectory &&
          msg->function() == qi::Message::ServiceDirectoryFunction_Service &&
          msg->type() == qi::Message::Type_Call)
      {
        std::string serviceName;
        qi::IDataStream ds(msg->buffer());
        ds >> serviceName;

        if (serviceInfo.name() == serviceName.c_str())
        {
          ans.setId(msg->id());
          it->first->write(ans);
        }
      }

      _services[qi::Message::Service_ServiceDirectory].pendingMessages.erase(it);
    }

    return;
  }

  QPair<unsigned int, QiTransportSocket*>& src = _routingTable[socket][msg.id()];
  qi::Message ans;
  ans.setBuffer(msg.buffer());
  ans.buildReplyFrom(msg);
  ans.setId(src.first);
  src.second->write(ans);
}

void QiGatewayPrivate::connectToService(unsigned int serviceId,
                                        const QUrl& address)
{
  QiTransportSocket* socket = 0;

  if (_services.find(serviceId) == _services.end())
  {
    socket = new QiTransportSocket(this);
    _services[serviceId].socket = socket;
    _services[serviceId].serviceId = serviceId;
    _sockets.push_back(socket);

    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
  }
  else
  {
    socket = _services[serviceId].socket;
    if (socket->peer() == address)
    {
      if (serviceId == qi::Message::Service_ServiceDirectory)
      {
        qiLogInfo("QiGateway") << "The gateway is already attached to service #"
                               << serviceId << " on "
                               << socket->peer().toString().toUtf8().constData();
      }
      return;
    }

    qiLogWarning("QiGateway") << "The gateway was already attached to service #"
                              << serviceId << " on "
                              << socket->peer().toString().toUtf8().constData();
    socket->close();
  }

  qiLogInfo("QiGateway") << "The gateway is now attached to service #"
                         << serviceId << " on "
                         << address.toString().toUtf8().constData();

  socket->connectToHost(address);
}

void QiGatewayPrivate::routeMessage(QiTransportSocket* src,
                                    QiTransportSocket* dst,
                                    const qi::Message& msg)
{
  // Create new message with unique ID
  qi::Message  fmsg;
  fmsg.setBuffer(msg.buffer());
  fmsg.buildForwardFrom(msg);

  // Store message to map call msg with return msg from the service
  _routingTable[dst][fmsg.id()] = qMakePair(msg.id(), src);

  dst->write(fmsg);
}

void QiGatewayPrivate::socketReadyRead()
{
  qiLogFatal("QiGateway") << "socketReadyRead";
  foreach (QiTransportSocket* socket, _sockets)
  {
    qi::Message* msg;
    while ((msg = socket->read()))
    {
      // TODO: {Remote,Reverse}Gateway

      QMap<unsigned int, RemoteService>::iterator it = _services.find(msg->service());
      if (it != _services.end() && it->socket == socket)
      {
        processServiceMessage(socket, *msg);
      }
      else
      {
        processClientMessage(socket, msg);
      }
    }
  }
}

void QiGatewayPrivate::attachToServiceDirectory(const QUrl &address)
{
  connectToService(qi::Message::Service_ServiceDirectory, address);
}

bool QiGatewayPrivate::listen(const QUrl &address)
{
  qiLogInfo("QiGateway") << "The gateway is listening on "
                         << address.toString().toUtf8().constData();
  return _server.listen(address);
}

void QiGatewayPrivate::close()
{
  _server.close();

  foreach (QiTransportSocket* socket, _sockets)
  {
    socket->close();
    delete socket;
  }
}

/*
 * Gateway
 */
QiGateway::QiGateway()
  : _p(new QiGatewayPrivate(this, QiGatewayPrivate::Type_Local))
{
}

QiGateway::~QiGateway()
{
  delete _p;
}

void QiGateway::attachToServiceDirectory(const QUrl& address)
{
  return _p->attachToServiceDirectory(address);
}

bool QiGateway::listen(const QUrl& address)
{
  return _p->listen(address);
}

void QiGateway::close()
{
  _p->close();
}
