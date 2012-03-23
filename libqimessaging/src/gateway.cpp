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
#include <boost/bind.hpp>
#include <qi/log.hpp>

#include "src/transport_socket_libevent_p.hpp"
#include "src/network_thread.hpp"
#include "src/session_p.hpp"

namespace qi
{

class GatewayPrivate: public TransportServerInterface, public TransportSocketInterface
{
public:

  typedef std::vector< std::pair<qi::Message*, TransportSocket *> > PendingMessageVector;
  typedef std::map< unsigned int, PendingMessageVector >           PendingMessageMap;

  typedef std::map< int, std::pair<int, TransportSocket *> > ClientRequestIdMap;
  typedef std::map< TransportSocket *, ClientRequestIdMap >  ServiceRequestIdMap;

  typedef std::map< unsigned int, qi::TransportSocket * > ServiceSocketMap;

protected:

  void handleClientRead(TransportSocket     *client,  qi::Message *msg);
  void handleServiceRead(TransportSocket    *service, qi::Message *msg);
  void forwardClientMessage(TransportSocket *client,
                            TransportSocket *service,
                            qi::Message     *msg);
  //ServerInterface
  virtual void newConnection();

  //SocketInterface
  virtual void onSocketReadyRead(TransportSocket    *client, int id);
  virtual void onSocketConnected(TransportSocket    *client);


public:
  ServiceSocketMap                   _services;
  std::vector<qi::TransportSocket *> _clients;
  std::vector<std::string>           _endpoints;
  TransportServer                    _transportServer;
  TransportSocket                   *_socketToServiceDirectory;
  qi::Session                        _session;

  //for each service socket, associated if request to a client socket. 0 mean gateway
  ServiceRequestIdMap                _serviceToClient;
  PendingMessageMap                  _pendingMessage;
}; // !GatewayPrivate

void GatewayPrivate::newConnection()
{
  TransportSocket *socket = _transportServer.nextPendingConnection();
  if (!socket)
    return;
  socket->setCallbacks(this);
  _clients.push_back(socket);
}

void GatewayPrivate::forwardClientMessage(TransportSocket *client,
                                          TransportSocket *service,
                                          qi::Message     *msg)
{
  // Create new message with unique ID
  qi::Message  msgToService;
  msgToService.setBuffer(msg->buffer());
  msgToService.buildForwardFrom(*msg);

  // Store message to map call msg with return msg from the service
  ClientRequestIdMap &reqIdMap = _serviceToClient[service];
  reqIdMap[msgToService.id()] = std::make_pair(msg->id(), client);

  // Send to the service
  service->send(msgToService);
}

//From Client
// C.1/ new message from client to know services         => forward to service, enter S.3 or S.1
// C.2/ new message from client to unknown destination   => put msg in pending queue, enter S.2
void GatewayPrivate::handleClientRead(TransportSocket *client,
                                      qi::Message     *msg)
{
  // Search service
  std::map<unsigned int, qi::TransportSocket*>::iterator it;
  it = _services.find(msg->service());

  //// C.1/
  if (it != _services.end() && it->second->isConnected())
  {
    forwardClientMessage(client, it->second, msg);
    return;
  }
  else  //// C.2/
  {
    // request to gateway to have the endpoint
    qi::Message sdMsg;
    qi::DataStream d(sdMsg.buffer());
    d << msg->service();

    // associate the transportSoket client = 0
    // this will allow S.1 to be handle correctly
    sdMsg.setType(qi::Message::Type_Call);
    sdMsg.setService(qi::Message::Service_ServiceDirectory);
    sdMsg.setPath(qi::Message::Path_Main);
    sdMsg.setFunction(qi::Message::ServiceDirectoryFunction_Service);

    ClientRequestIdMap &reqIdMap = _serviceToClient[_socketToServiceDirectory];
    reqIdMap[sdMsg.id()] = std::make_pair(0, (qi::TransportSocket*)NULL);

    // store the pending message
    PendingMessageMap::iterator itPending;
    itPending = _pendingMessage.find(msg->service());
    if (itPending == _pendingMessage.end())
    {
      PendingMessageVector pendingMsg;
      pendingMsg.push_back(std::make_pair(msg, client));
      _pendingMessage[msg->service()] = pendingMsg;
    }
    else
    {
      PendingMessageVector &pendingMsg = itPending->second;
      pendingMsg.push_back(std::make_pair(msg, client));
    }
    _socketToServiceDirectory->send(sdMsg);

    return;
  }
}

//From Service
// S.1/ New message from sd for us => Change endpoint (gateway), enter S.3
// S.2/ New service connected          => forward pending msg to service, enter S.3
// S.3/ New message from service       => forward to client, (end)
void GatewayPrivate::handleServiceRead(TransportSocket *service, qi::Message *msg)
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
  itReq = request.find(msg->id());
  if (itReq != request.end())
  {
    //// S.1/
    if (msg->service() == qi::Message::Service_ServiceDirectory &&
        msg->function() == qi::Message::ServiceDirectoryFunction_Service &&
        msg->type() == qi::Message::Type_Reply)
    {
      // Get serviceId
      ServiceInfo    result;
      qi::DataStream ds(msg->buffer());
      ds >> result;

      // save address of the new service
      qi::Url url(result.endpoints()[0]);
      // Construct reply with serviceId
      // and gateway endpoint
      result.setEndpoints(_endpoints);

      // create new message for the client
      qi::Message  ans;
      qi::Buffer   buf;
      ans.setBuffer(buf);
      ans.buildReplyFrom(*msg);
      qi::DataStream dsAns(buf);
      dsAns << result;

      // id should be rewritten then sent to the client
      ans.setId(itReq->second.first);
      itReq->second.second->send(ans);

      unsigned int serviceId = result.serviceId();

      // Check if the gateway is connected to the requested service
      std::map<unsigned int, qi::TransportSocket*>::const_iterator it;
      it = _services.find(serviceId);
      // Service connected
      if (it != _services.end())
        return;

      // Connected to the service
      qi::TransportSocket *servSocket = new qi::TransportSocket();
      servSocket->connect(&_session, url);
      servSocket->setCallbacks(this);
      _services[serviceId] = servSocket;

      return; //// jump to S.2
    }
    else //// S.3/
    {
      // id should be rewritten then sent to the client
      qi::Message ans;
      ans.setBuffer(msg->buffer());
      ans.buildReplyFrom(*msg);
      ans.setId(itReq->second.first);
      itReq->second.second->send(ans);
    }
  }
}

void GatewayPrivate::onSocketReadyRead(TransportSocket *client, int id)
{
  qi::Message msg;
  client->read(id, &msg);
  // Dispatch request coming from client or service
  if (std::find(_clients.begin(), _clients.end(), client) != _clients.end())
    handleClientRead(client, &msg);  // Client
  else
    handleServiceRead(client, &msg); // Service
}

// S.2/
void GatewayPrivate::onSocketConnected(TransportSocket *service)
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
    qi::Message *msg = itPending->first;
    forwardClientMessage(client, service, msg);
  }
}

Gateway::Gateway()
  : _p(new GatewayPrivate())
{
}

Gateway::~Gateway()
{
  delete _p;
}

void Gateway::join()
{
  _p->_session.join();
}

bool Gateway::listen(const qi::Url &listenAddress,
                     const qi::Url &serviceDirectoryURL)
{
  _p->_session.connect(serviceDirectoryURL);
  _p->_session.waitForConnected();

  _p->_socketToServiceDirectory = new qi::TransportSocket();
  _p->_socketToServiceDirectory->connect(&(_p->_session), serviceDirectoryURL);
  _p->_socketToServiceDirectory->setCallbacks(_p);
  _p->_socketToServiceDirectory->waitForConnected();
  _p->_services[qi::Message::Service_ServiceDirectory] = _p->_socketToServiceDirectory;
  _p->_endpoints.push_back(listenAddress.str());
  _p->_transportServer.setCallbacks(_p);
  return _p->_transportServer.start(&(_p->_session), listenAddress);
}
} // !qi
