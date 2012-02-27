/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/object.hpp>
#include <qimessaging/gateway.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/transport_server.hpp>
#include "src/network_thread.hpp"
#include <boost/bind.hpp>
#include <qi/log.hpp>

static int reqid = 500;

namespace qi
{

class GatewayPrivate: public TransportServerInterface, public TransportSocketInterface
{
public:

  typedef std::vector< std::pair<qi::Message, TransportSocket *> > PendingMessageVector;
  typedef std::map< unsigned int, PendingMessageVector >           PendingMessageMap;

  typedef std::map< int, std::pair<int, TransportSocket *> > ClientRequestIdMap;
  typedef std::map< TransportSocket *, ClientRequestIdMap >  ServiceRequestIdMap;

  typedef std::map< unsigned int, qi::TransportSocket * > ServiceSocketMap;

protected:

  void handleClientRead(TransportSocket     *client,  qi::Message &msg);
  void handleServiceRead(TransportSocket    *service, qi::Message &msg);
  void forwardClientMessage(TransportSocket *client,
                            TransportSocket *service,
                            qi::Message &msg);
  //ServerInterface
  virtual void newConnection();

  //SocketInterface
  virtual void onReadyRead(TransportSocket    *client, qi::Message &msg);
  virtual void onWriteDone(TransportSocket    *client);
  virtual void onConnected(TransportSocket    *client);
  virtual void onDisconnected(TransportSocket *client);


public:
  ServiceSocketMap                             _services;
  std::vector<qi::TransportSocket *>           _clients;
  std::vector<std::string>                     _endpoints;
  TransportServer                              _transportServer;
  TransportSocket                             *_socketToServiceDirectory;
  qi::Session                                 *_session;

  //for each service socket, associated if request to a client socket. 0 mean gateway
  ServiceRequestIdMap                         _serviceToClient;
  PendingMessageMap                           _pendingMessage;
}; // !GatewayPrivate

void GatewayPrivate::newConnection()
{
  TransportSocket *socket = _transportServer.nextPendingConnection();
  if (!socket)
    return;
  socket->setDelegate(this);
  _clients.push_back(socket);
}

void GatewayPrivate::forwardClientMessage(TransportSocket *client,
                                          TransportSocket *service,
                                          qi::Message &msg)
{
  // Create new message with unique ID
  qi::Message  msgToService(msg.buffer());
  msgToService.buildForwardFrom(msg);

  // Store message to map call msg with return msg from the service
  ClientRequestIdMap &reqIdMap = _serviceToClient[service];
  reqIdMap[msgToService.id()] = std::make_pair(msg.id(), client);

  // Send to the service
  service->send(msgToService);
}

//From Client
// C.1/ new message from client to know services         => forward to service, enter S.3 or S.1
// C.2/ new message from client to unknown destination   => put msg in pending queue, enter S.2
void GatewayPrivate::handleClientRead(TransportSocket *client,
                                      qi::Message &msg)
{
  // Search service
  std::map<unsigned int, qi::TransportSocket*>::iterator it;
  it = _services.find(msg.service());

  //// C.1/
  if (it != _services.end() && it->second->isConnected())
  {
    forwardClientMessage(client, it->second, msg);
    return;
  }
  else  //// C.2/
  {
    // request to gateway to have the endpoint
    qi::Message masterMsg;
    qi::DataStream d(masterMsg.buffer());
    d << msg.service();

    // associate the transportSoket client = 0
    // this will allow S.1 to be handle correctly
    masterMsg.setType(qi::Message::Type_Call);
    masterMsg.setService(qi::Message::Service_ServiceDirectory);
    masterMsg.setPath(qi::Message::Path_Main);
    masterMsg.setFunction(qi::Message::ServiceDirectoryFunction_Service);

    ClientRequestIdMap &reqIdMap = _serviceToClient[_socketToServiceDirectory];
    reqIdMap[masterMsg.id()] = std::make_pair(0, (qi::TransportSocket*)NULL);

    // store the pending message
    PendingMessageMap::iterator itPending;
    itPending = _pendingMessage.find(msg.service());
    if (itPending == _pendingMessage.end())
    {
      PendingMessageVector pendingMsg;
      pendingMsg.push_back(std::make_pair(msg, client));
      _pendingMessage[msg.service()] = pendingMsg;
    }
    else
    {
      PendingMessageVector &pendingMsg = itPending->second;
      pendingMsg.push_back(std::make_pair(msg, client));
    }
    _socketToServiceDirectory->send(masterMsg);

    return;
  }
}

//From Service
// S.1/ New message from master for us => Change endpoint (gateway), enter S.3
// S.2/ New service connected          => forward pending msg to service, enter S.3
// S.3/ New message from service       => forward to client, (end)
void GatewayPrivate::handleServiceRead(TransportSocket *service, qi::Message &msg)
{
  // get the map of request => client
  ServiceRequestIdMap::iterator it;
  it = _serviceToClient.find(service);
  // Must not fail
  if (it == _serviceToClient.end())
  {
    qiLogError("Gateway", "Cannot find Client request for Service reply.\n");
    return;
  }

  ClientRequestIdMap &request = it->second;
  ClientRequestIdMap::const_iterator itReq;
  itReq = request.find(msg.id());
  if (itReq != request.end())
  {
    //// S.1/
    if (msg.service() == qi::Message::Service_ServiceDirectory &&
        msg.function() == qi::Message::ServiceDirectoryFunction_Service &&
        msg.type() == qi::Message::Type_Reply)
    {
      // Get serviceId
      std::vector<std::string> result;
      qi::DataStream ds(msg.buffer());
      ds >> result;


      // Construct reply with serviceId
      // and gateway endpoint
      std::vector<std::string> reply;
      reply.push_back(result[0]);

      std::vector<std::string>::const_iterator endpointIt;
      for (endpointIt = _endpoints.begin(); endpointIt != _endpoints.end(); ++endpointIt)
        reply.push_back(*endpointIt);

      // create new message for the client
      qi::Message ans;
      ans.buildReplyFrom(msg);
      qi::DataStream dsAns(ans.buffer());
      dsAns << reply;

      // id should be rewritten then sent to the client
      ans.setId(itReq->second.first);
      itReq->second.second->send(ans);

      unsigned int serviceId;
      std::stringstream ss(result[0]);
      ss >>  serviceId;

      // Check if the gateway is connected to the requested service
      std::map<unsigned int, qi::TransportSocket*>::const_iterator it;
      it = _services.find(serviceId);
      // Service connected
      if (it != _services.end())
        return;

      // Connected to the service
      qi::Url url(result[1]);
      qi::TransportSocket *servSocket = new qi::TransportSocket();
      servSocket->setDelegate(this);
      servSocket->connect(url,
                          _session->_nthd->getEventBase());
      _services[serviceId] = servSocket;

      return; //// jump to S.2
    }
    else //// S.3/
    {
      // id should be rewritten then sent to the client
      qi::Message ans(msg.buffer());
      ans.buildReplyFrom(msg);
      ans.setId(itReq->second.first);
      itReq->second.second->send(ans);
    }
  }
}

void GatewayPrivate::onReadyRead(TransportSocket *client, qi::Message &msg)
{
  // Dispatch request coming from client or service
  if (std::find(_clients.begin(), _clients.end(), client) != _clients.end())
    handleClientRead(client, msg);  // Client
  else
    handleServiceRead(client, msg); // Service
}

void GatewayPrivate::onWriteDone(TransportSocket *client)
{
}

// S.2/
void GatewayPrivate::onConnected(TransportSocket *service)
{
  if (service == _socketToServiceDirectory)
    return;

  unsigned int serviceId;
  ServiceSocketMap::const_iterator it;

  //TODO: optimise?  O(log(n)) instead O(n)
  for (it = _services.begin(); it != _services.end(); ++it)
  {
    if (static_cast<TransportSocket *>(it->second) == service)
    {
      serviceId = it->first;
      break;
    }
  }

  if (it == _services.end())
  {
    qiLogError("Gateway", "Cannot find any service!\n");
    return;
  }

  //handle pending message
  PendingMessageVector &pmv = _pendingMessage[serviceId];
  PendingMessageVector::iterator itPending;

  for (itPending = pmv.begin(); itPending != pmv.end(); ++itPending)
  {
    TransportSocket *client = itPending->second;
    qi::Message &msg = itPending->first;
    forwardClientMessage(client, service, msg);
  }
}

void GatewayPrivate::onDisconnected(TransportSocket *client)
{
}

Gateway::Gateway()
  : _p(new GatewayPrivate())
{
}

Gateway::~Gateway()
{
  delete _p;
}

void Gateway::listen(qi::Session *session, const std::string &addr)
{
  qi::Url url(addr);
  qi::Url masterUrl("tcp://127.0.0.1:5555");
  _p->_session = session;
  _p->_socketToServiceDirectory = new qi::TransportSocket();
  _p->_socketToServiceDirectory->setDelegate(_p);
  _p->_socketToServiceDirectory->connect(masterUrl, _p->_session->_nthd->getEventBase());
  _p->_socketToServiceDirectory->waitForConnected();
  _p->_services[qi::Message::Service_ServiceDirectory] = _p->_socketToServiceDirectory;
  _p->_endpoints.push_back(addr);
  _p->_transportServer.setDelegate(_p);
  _p->_transportServer.start(url, session->_nthd->getEventBase());
}
} // !qi
