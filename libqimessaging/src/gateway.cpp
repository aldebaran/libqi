/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**  - Laurent Lec <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/object.hpp>
#include <qimessaging/gateway.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/transport_server.hpp>
#include <boost/bind.hpp>
#include <qi/log.hpp>

#include "src/transport_socket_libevent_p.hpp"
#include "src/session_p.hpp"

namespace qi
{

class GatewayPrivate: public TransportServerInterface, public TransportSocketInterface
{
public:
  enum Type
  {
    Type_LocalGateway   = 1,
    Type_ReverseGateway = 2,
    Type_RemoteGateway  = 3,
  };

  GatewayPrivate();
  ~GatewayPrivate();

  bool attachToServiceDirectory(const Url &address);
  bool listen(const Url &address);
  bool connect(const Url &address);
  void addCallbacks(TransportServerInterface *tsrvi,
                    TransportSocketInterface *tscki);

protected:
  void handleMsgFromClient(TransportSocket *client, qi::Message *msg);
  void handleMsgFromService(TransportSocket *service, qi::Message *msg);
  void forwardClientMessage(TransportSocket *client, TransportSocket *service, Message *msg);

  //ServerInterface
  virtual void newConnection(TransportServer* server, TransportSocket *socket);

  //SocketInterface
  virtual void onSocketReadyRead(TransportSocket *client, int id);
  virtual void onSocketConnected(TransportSocket *client);
  virtual void onSocketDisconnected(TransportSocket *socket);

public:
  Type                               _type;
  TransportServer                   *_transportServer;
  Session                            _session;

  /* Map from ServiceId to associated TransportSocket */
  std::map< unsigned int, qi::TransportSocket* > _services;

  /* Vector of all the TransportSocket of the clients */
  std::list<TransportSocket*>                    _clients;

  /* For each service, map a received Message and its TransportSocket to the rewritten id */
  std::map< TransportSocket*, std::map< int, std::pair<int, TransportSocket*> > > _serviceToClient;

  /* Map of vectors of pending messages for each service */
  std::map< unsigned int, std::vector< std::pair<Message*, TransportSocket*> >  >  _pendingMessage;

  std::list<TransportSocket*> _remoteGateways;

  std::list<TransportSocketInterface*> _transportSocketCallbacks;

  Url _attachAddress;
};

GatewayPrivate::GatewayPrivate()
: _transportServer(0)
{
  _transportSocketCallbacks.push_back(this);
}

GatewayPrivate::~GatewayPrivate()
{
  _transportSocketCallbacks.clear();
  for (std::list<TransportSocket*>::iterator clientIt = _clients.begin();
       clientIt != _clients.end();
       ++clientIt)
  {
    delete *clientIt;
  }

  for (std::map< unsigned int, qi::TransportSocket* >::iterator servicesIt = _services.begin();
       servicesIt != _services.end();
       ++servicesIt)
  {
    delete servicesIt->second;
  }

  for (std::list<TransportSocket*>::iterator remoteGatewaysIt = _remoteGateways.begin();
       remoteGatewaysIt != _remoteGateways.end();
       ++remoteGatewaysIt)
  {
    delete *remoteGatewaysIt;
  }

  for (std::map< unsigned int, std::vector< std::pair<Message*, TransportSocket*> > >::iterator _pendingMessageIt = _pendingMessage.begin();
       _pendingMessageIt != _pendingMessage.end();
       _pendingMessageIt++)
  {
    for (std::vector< std::pair<Message*, TransportSocket*> >::iterator msgTsVecIt = _pendingMessageIt->second.begin();
         msgTsVecIt != _pendingMessageIt->second.end();
         msgTsVecIt++)
    {
      delete msgTsVecIt->first;
    }
  }

  delete _transportServer;
}

void GatewayPrivate::newConnection(TransportServer* server, TransportSocket *socket)
{
  if (!socket)
    return;

  for (std::list<TransportSocketInterface*>::iterator it = _transportSocketCallbacks.begin();
       it != _transportSocketCallbacks.end();
       ++it)
  {
    socket->addCallbacks(*it);
  }

  _clients.push_back(socket);
}

void GatewayPrivate::forwardClientMessage(TransportSocket *client, TransportSocket *service, Message *msg)
{
  // Create new message with unique ID
  Message  msgToService;
  msgToService.setBuffer(msg->buffer());
  msgToService.buildForwardFrom(*msg);

  // Store message to map call msg with return msg from the service
  std::map< int, std::pair<int, TransportSocket *> > &reqIdMap = _serviceToClient[service];
  reqIdMap[msgToService.id()] = std::make_pair(msg->id(), client);

  // Send to the service
  service->send(msgToService);
}

/*
 * The message comes from a client. Two cases:
 * C1: the destination service is already known, we can forward the message.
 * C2: the destination service is unknown, we try to establish connection,
 *     and we enqueue the message, which will be sent in S2.
 */
void GatewayPrivate::handleMsgFromClient(TransportSocket *client, Message *msg)
{
  // Search service
  std::map<unsigned int, TransportSocket*>::iterator it = _services.find(msg->service());

  /* C1 */
  if (it != _services.end() && it->second->isConnected())
  {
    forwardClientMessage(client, it->second, msg);
    return;
  }
  /* C2 */
  else
  {
    /*
     * The service is unknown to the Gateway. We will have to query
     * the Service Directory.
     */
    // store the pending message until connection to the service is established (S2)
    _pendingMessage[msg->service()].push_back(std::make_pair(msg, client));

    if (_services.find(Message::Service_ServiceDirectory) == _services.end())
    {
      qiLogError("gateway") << "Not connected to Service Directory";
      if (_attachAddress.isValid())
      {
        qiLogInfo("gateway") << "Retry to connect to Service Directory on "
                             << _attachAddress.str();
        TransportSocket *sdSocket = new qi::TransportSocket();
        _services[qi::Message::Service_ServiceDirectory] = sdSocket;
        for (std::list<TransportSocketInterface*>::iterator it = _transportSocketCallbacks.begin();
             it != _transportSocketCallbacks.end();
             ++it)
        {
          sdSocket->addCallbacks(*it);
        }
        sdSocket->connect(_attachAddress);
      }
      return;
    }

    Message sdMsg;
    Buffer buf;
    ODataStream d(buf);
    d << msg->service();
    sdMsg.setBuffer(buf);

    // associate the transportSoket client = 0
    // this will allow S.1 to be handle correctly
    sdMsg.setType(Message::Type_Call);
    sdMsg.setService(Message::Service_ServiceDirectory);
    sdMsg.setObject(Message::Object_Main);
    sdMsg.setFunction(Message::ServiceDirectoryFunction_Service);

    _serviceToClient[_services[Message::Service_ServiceDirectory]][sdMsg.id()] = std::make_pair(0, (TransportSocket*) 0);

    _services[Message::Service_ServiceDirectory]->send(sdMsg);

    return;
  }
}

// S.1/ New message from sd for us => Change endpoint (gateway), enter S.3
// S.2/ New service connected          => forward pending msg to service, enter S.3
// S.3/ New message from service       => forward to client, (end)
void GatewayPrivate::handleMsgFromService(TransportSocket *service, Message *msg)
{
  // get the map of request => client
  std::map< TransportSocket *, std::map< int, std::pair<int, TransportSocket *> > >::iterator it;
  it = _serviceToClient.find(service);
  // Must not fail
  if (it == _serviceToClient.end())
  {
    qiLogError("Gateway", "Cannot find Client request for Service reply.\n");
    return;
  }

  std::map< int, std::pair<int, TransportSocket *> > &request = it->second;
  std::map< int, std::pair<int, TransportSocket *> >::const_iterator itReq;
  itReq = request.find(msg->id());
  if (itReq != request.end())
  {
    //// S.1/
    if (msg->service() == Message::Service_ServiceDirectory &&
        msg->function() == Message::ServiceDirectoryFunction_Service &&
        msg->type() == Message::Type_Reply)
    {
      // Get serviceId
      ServiceInfo    result;
      qi::IDataStream ds(msg->buffer());
      ds >> result;

      if (result.name() == "")
      {
        qiLogError("gateway") << "Could not find requested service";
        Message ans;
        ans.buildReplyFrom(*msg);
        ans.setType(qi::Message::Type_Error);
        ans.setId(itReq->second.first);
        itReq->second.second->send(ans);
        return;
      }

      // save address of the new service
      std::vector<std::string> endpoints = result.endpoints();
      // Construct reply with serviceId
      // and gateway endpoint
      {
        std::vector<std::string> eps;
        std::vector<qi::Url> tsEps = _transportServer->endpoints();
        for (std::vector<qi::Url>::const_iterator tsEpsIt = tsEps.begin();
             tsEpsIt != tsEps.end();
             tsEpsIt++)
        {
          eps.push_back((*tsEpsIt).str());
        }
        result.setEndpoints(eps);
      }

      // create new message for the client
      Message  ans;
      Buffer   buf;
      ans.setBuffer(buf);
      ans.buildReplyFrom(*msg);
      ODataStream dsAns(buf);
      dsAns << result;

      // id should be rewritten then sent to the client
      ans.setId(itReq->second.first);
      itReq->second.second->send(ans);

      unsigned int serviceId = result.serviceId();

      // Check if the gateway is connected to the requested service
      std::map<unsigned int, TransportSocket*>::const_iterator it;
      it = _services.find(serviceId);
      // Service connected
      if (it != _services.end())
        return;

      if (_type == Type_RemoteGateway)
      {
        _services[serviceId] = _services[Message::Service_ServiceDirectory];
      }
      else
      {
        qi::Url url(endpoints[0]);
        // Connect to the service
        TransportSocket *service = new TransportSocket();
        service->connect(url);

        for (std::list<TransportSocketInterface*>::iterator it = _transportSocketCallbacks.begin();
             it != _transportSocketCallbacks.end();
             ++it)
        {
          service->addCallbacks(*it);
        }

        _services[serviceId] = service;
      }

      // We will be called back when the connection is established (S2).
    }
    else //// S.3/
    {
      // id should be rewritten then sent to the client
      Message ans;
      ans.setBuffer(msg->buffer());
      ans.buildReplyFrom(*msg);
      ans.setId(itReq->second.first);
      itReq->second.second->send(ans);
    }
  }
}

/*
 * Called for any incoming message.
 */
void GatewayPrivate::onSocketReadyRead(TransportSocket *socket, int id)
{
  qi::Message msg;
  socket->read(id, &msg);

  /*
   * A ReverseGateway connected. This is our endpoint for the Service
   * Directory.
   * A RemoteGateway can be connected to only one ReverseGateway.
   */
  if (msg.service() == Message::Service_Server &&
      msg.function() == Message::ServerFunction_Connect)
  {
    if (_type == Type_RemoteGateway && msg.type() == Message::Type_Call)
    {
      /*
       * Since the ReverseGateway connected itself to the RemoteGateway,
       * it is known as a client. We need to fix it by removing its
       * TransportSocket fro the _clients vector.
       */
      std::list<TransportSocket *>::iterator it = std::find(_clients.begin(), _clients.end(), socket);
      _clients.erase(it);

      if (_services.find(Message::Service_ServiceDirectory) == _services.end())
      {
        qiLogInfo("gateway") << "Attached to ReverseGateway";

        _services[Message::Service_ServiceDirectory] = socket;
        qi::Buffer buf;
        qi::Message ans;
        ans.setBuffer(buf);
        ans.setService(qi::Message::Service_Server);
        ans.setType(qi::Message::Type_Reply);
        ans.setFunction(qi::Message::ServerFunction_Connect);
        ans.setObject(qi::Message::Object_Main);
        qi::ODataStream d(buf);
        d << "";
        socket->send(ans);
      }
      else
      {
        qiLogError("gateway") << "Already connected to Service Directory";
      }
    }
    else if (_type == Type_ReverseGateway && msg.type() == Message::Type_Reply)
    {
      std::string endpoint;
      IDataStream d(msg.buffer());
      d >> endpoint;
      if (endpoint != "")
      {
        connect(endpoint);
      }
    }

    return; // nothing more to do here
  }

  /*
   * Routing will depend on where the package comes from.
   */
  if (std::find(_clients.begin(), _clients.end(), socket) != _clients.end())
  {
    handleMsgFromClient(socket, &msg);
  }
  else
  {
    handleMsgFromService(socket, &msg);
  }
}

/*
 * Callback triggered when Gateway or ReverseGateway have established
 * a connection to the ServiceDirectory or to another service, or when
 * the ReverseGateway has reached a RemoteGateway.
 */
// S.2/
void GatewayPrivate::onSocketConnected(TransportSocket *service)
{
  for (std::map< unsigned int, TransportSocket * >::const_iterator it = _services.begin();
       it != _services.end();
       ++it)
  {
    // handle pending messages
    if (it->second == service)
    {
      unsigned int serviceId = it->first;
      qiLogInfo("gateway") << "Connected to service #" << serviceId;
      std::vector< std::pair<qi::Message*, TransportSocket*> >  &pmv = _pendingMessage[serviceId];
      std::vector< std::pair<qi::Message*, TransportSocket*> > ::iterator itPending;

      for (itPending = pmv.begin(); itPending != pmv.end(); ++itPending)
      {
        forwardClientMessage(itPending->second, service, itPending->first);
      }
      return;
    }
  }

  for (std::list<TransportSocket*>::iterator it = _remoteGateways.begin();
       it != _remoteGateways.end();
       ++it)
  {
    if (*it == service)
    {
      TransportSocket *ts = *it;

      qi::Message msg;
      msg.setService(qi::Message::Service_Server);
      msg.setType(qi::Message::Type_Call);
      msg.setFunction(qi::Message::ServerFunction_Connect);
      msg.setObject(qi::Message::Object_Main);

      ts->send(msg);
      _clients.push_back(ts);
      _remoteGateways.erase(it);
      break;
    }
  }

  if (_type != Type_ReverseGateway)
  {
    qiLogError("gateway") << "Unknown service TransportSocket " << service;
  }
}

void GatewayPrivate::onSocketDisconnected(TransportSocket *socket)
{
  // Was it a Service?
  for (std::map< unsigned int, qi::TransportSocket* >::iterator it = _services.begin();
       it != _services.end();
       )
  {
    if (it->second == socket)
    {
      unsigned int sid = it->first;
      if (sid == Message::Service_ServiceDirectory)
      {
        qiLogError("gateway") << "Connection to the Service Directory was lost!";
      }
      else
      {
        qiLogInfo("gateway") << "Connection to service #" << it->first
                             << " was lost!";
      }
      // Remove the service from the _services map
      _services.erase(it++);

      // Remove the corresponding message routing table
      std::map< TransportSocket*, std::map< int, std::pair<int, TransportSocket*> > >::iterator it2 = _serviceToClient.find(socket);
      if (it2 != _serviceToClient.end())
      {
        _serviceToClient.erase(it2);
        _pendingMessage[sid].clear();
      }
    }
    else
    {
      ++it;
    }
  }

  // Was it a Client?
  _clients.remove(socket); // this stupid STL doesn't have list::find()
}

bool GatewayPrivate::attachToServiceDirectory(const Url &address)
{
  _attachAddress = address;

  TransportSocket *sdSocket = new qi::TransportSocket();
  _services[qi::Message::Service_ServiceDirectory] = sdSocket;
  for (std::list<TransportSocketInterface*>::iterator it = _transportSocketCallbacks.begin();
       it != _transportSocketCallbacks.end();
       ++it)
  {
    sdSocket->addCallbacks(*it);
  }

  sdSocket->connect(address);
  sdSocket->waitForConnected();

  if (!sdSocket->isConnected())
  {
    qiLogError("gateway") << "Could not attach to Service Directory "
                          << address.str();
    return false;
  }

  return true;
}

bool GatewayPrivate::listen(const Url &address)
{
  _transportServer = new qi::TransportServer(address);
  _transportServer->addCallbacks(this);
  return _transportServer->listen();
}

bool GatewayPrivate::connect(const qi::Url &connectURL)
{
  qiLogInfo("gateway") << "Connecting to remote gateway: " << connectURL.str();

  qi::TransportSocket *ts = new qi::TransportSocket();
  for (std::list<TransportSocketInterface*>::iterator it = _transportSocketCallbacks.begin();
       it != _transportSocketCallbacks.end();
       ++it)
  {
    ts->addCallbacks(*it);
  }
  ts->connect(connectURL);
  _remoteGateways.push_back(ts);

  return true;
}

void GatewayPrivate::addCallbacks(qi::TransportServerInterface *tsrvi,
                                  qi::TransportSocketInterface *tscki)
{
  _transportServer->addCallbacks(tsrvi);
  _transportSocketCallbacks.push_back(tscki);
}

/* Gateway bindings */
Gateway::Gateway()
  : _p(new GatewayPrivate())
{
  _p->_type = GatewayPrivate::Type_LocalGateway;
}

Gateway::~Gateway()
{
  delete _p;
}

bool Gateway::attachToServiceDirectory(const qi::Url &address)
{
  return _p->attachToServiceDirectory(address);
}

bool Gateway::listen(const qi::Url &address)
{
  return _p->listen(address);
}

/* RemoteGateway bindings */
RemoteGateway::RemoteGateway()
  : _p(new GatewayPrivate())
{
  _p->_type = GatewayPrivate::Type_RemoteGateway;
}

RemoteGateway::~RemoteGateway()
{
  delete _p;
}

bool RemoteGateway::listen(const qi::Url &address)
{
  return _p->listen(address);
}

void RemoteGateway::addCallbacks(TransportServerInterface *tsrvi,
                                 TransportSocketInterface *tscki)
{
  _p->addCallbacks(tsrvi, tscki);
}

/* ReverseGateway bindings */
ReverseGateway::ReverseGateway()
  : _p(new GatewayPrivate())
{
  _p->_type = GatewayPrivate::Type_ReverseGateway;
}

ReverseGateway::~ReverseGateway()
{
  delete _p;
}

bool ReverseGateway::attachToServiceDirectory(const qi::Url &address)
{
  return _p->attachToServiceDirectory(address);
}

bool ReverseGateway::connect(const qi::Url &address)
{
  return _p->connect(address);
}

} // !qi
