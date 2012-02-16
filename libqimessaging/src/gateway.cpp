/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/object.hpp>
#include <qimessaging/gateway.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/transport/transport_server.hpp>
#include <qimessaging/transport/network_thread.hpp>
#include <boost/bind.hpp>
#include <qi/log.hpp>

static int reqid = 500;

namespace qi
{

class GatewayPrivate : public TransportServerInterface, public TransportSocketInterface
{
public:

  typedef std::vector< std::pair<qi::Message, TransportSocket *> > PendingMessageVector;
  typedef std::map< std::string, PendingMessageVector>             PendingMessageMap;

  typedef std::map< int, std::pair<int, TransportSocket *> >       RequestIdMap;
  typedef std::map<TransportSocket *, RequestIdMap>                ServiceRequestIdMap;

  typedef std::map<std::string, qi::TransportSocket *>             ServiceSocketMap;

  GatewayPrivate();

protected:

  void handleClientRead(TransportSocket *client, const qi::Message &msg);
  void handleGatewayServiceRead(TransportSocket *master, const qi::Message &msg);
  void handleServiceRead(TransportSocket *service, const qi::Message &msg);
  void forwardClientMessage(TransportSocket *client,
                            TransportSocket *service,
                            const qi::Message &msg);
  //ServerInterface
  virtual void newConnection();

  //SocketInterface
  virtual void onReadyRead(TransportSocket *client, const qi::Message &msg);
  virtual void onWriteDone(TransportSocket *client);
  virtual void onConnected(TransportSocket *client);
  virtual void onDisconnected(TransportSocket *client);


public:
  ServiceSocketMap                             _services;
  std::vector<qi::TransportSocket *>           _clients;
  std::vector<std::string>                     _endpoints;
  TransportServer                              _ts;
  TransportSocket                             *_tso;
  qi::NetworkThread                           *_nthd;

  //for each service socket, associated if request to a client socket. 0 mean gateway
  ServiceRequestIdMap                         _serviceToClient;
  PendingMessageMap                           _pendingMessage;
}; // !GatewayPrivate





GatewayPrivate::GatewayPrivate()
{
  _nthd = new qi::NetworkThread();

  _tso = new qi::TransportSocket();
  _tso->setDelegate(this);
  _tso->connect("127.0.0.1", 5555, _nthd->getEventBase());
  _tso->waitForConnected();
  _services["qi.master"] = _tso;
}

void GatewayPrivate::newConnection()
{
  TransportSocket *socket = _ts.nextPendingConnection();
  if (!socket)
    return;
  socket->setDelegate(this);
  _clients.push_back(socket);
}

void GatewayPrivate::forwardClientMessage(TransportSocket *client, TransportSocket *service, const qi::Message &msg)
{
  qi::Message   servMsg(msg);
  RequestIdMap &reqIdMap = _serviceToClient[service];

  servMsg.setId(reqid++);
  reqIdMap[servMsg.id()] = std::make_pair(msg.id(), client);
  service->send(servMsg);
}

//From Client
// C.1/ new client which ask master for a service        => return gateway endpoint, enter C.2 or C.3
// C.2/ new message from client to unknown destination   => ask master, enter S.1
// C.3/ new message from client to know services         => forward, enter S.3
void GatewayPrivate::handleClientRead(TransportSocket *client, const qi::Message &msg)
{
  // C.1/ We are the Master!
  // unique case: service always return gateway endpoint
  if (msg.destination() == "qi.master" && msg.function() == "service")
  {
    qi::Message retval;
    qi::DataStream d;
    std::vector<std::string> tmpEndPoint;
    tmpEndPoint.push_back(msg.destination());
    for (int i = 0; i < _endpoints.size(); ++i)
      tmpEndPoint.push_back(_endpoints[i]);

    d << tmpEndPoint;

    retval.setType(qi::Message::Reply);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setFunction(msg.function());
    retval.setData(d.str());
    client->send(retval);
    return;
  }

  std::map<std::string, qi::TransportSocket*>::iterator it;
  it = _services.find(msg.destination());

  //// C.3/
  if (it != _services.end())
  {
    forwardClientMessage(client, it->second, msg);
  }
  //// C.2/
  else
  {
    //request to gateway to have the endpoint
    qi::Message masterMsg;
    qi::DataStream d;
    d << msg.destination();

    //associate the transportSoket client = 0
    //this will allow S.1 to be handle correctly
    masterMsg.setType(qi::Message::Call);
    masterMsg.setDestination("qi.master");
    masterMsg.setFunction("service");
    masterMsg.setData(d.str());


    RequestIdMap &reqIdMap = _serviceToClient[_tso];
    reqIdMap[masterMsg.id()] = std::make_pair(0, (qi::TransportSocket*)NULL);

    //store the pending message
    PendingMessageMap::iterator itPending;
    itPending = _pendingMessage.find(msg.destination());
    if (itPending == _pendingMessage.end())
    {
      PendingMessageVector pendingMsg;
      pendingMsg.push_back(std::make_pair(msg, client));
      _pendingMessage[msg.destination()] = pendingMsg;
    }
    else
    {
      PendingMessageVector &pendingMsg = itPending->second;
      pendingMsg.push_back(std::make_pair(msg, client));
    }
    _tso->send(masterMsg);

    return;
  }
}

//Only be master request
//S1 answer from master for us
void GatewayPrivate::handleGatewayServiceRead(TransportSocket *master, const qi::Message &msg)
{
  ServiceRequestIdMap::iterator it;
  // get the map of request => client
  it = _serviceToClient.find(master);

  //assert it's really from master
  if (it == _serviceToClient.end())
    return;


  std::vector<std::string>      result;
  qi::DataStream                d(msg.data());

  d >> result;

  qi::EndpointInfo endpoint;
  size_t begin = 0;
  size_t end = 0;
  end = result[1].find(":");
  endpoint.type = result[1].substr(begin, end);
  begin = end + 3;
  end = result[1].find(":", begin);
  endpoint.ip = result[1].substr(begin, end - begin);
  begin = end + 1;
  std::stringstream ss(result[1].substr(begin));
  ss >> endpoint.port;


  //new socket
  qi::TransportSocket *servSocket = new qi::TransportSocket();
  servSocket->setDelegate(this);
  //call connect
  servSocket->connect(endpoint.ip, endpoint.port, _nthd->getEventBase());

  //TODO: serviceName = endpointIt.name();
  std::string serviceName = result[0];
  _services[serviceName] = servSocket;
  //   go to S.2
}


//From Service
// S.1/ new message from master from us                  => create new service, enter S.2
// S.3/ new message from service                         => forward, (end)
void GatewayPrivate::handleServiceRead(TransportSocket *service, const qi::Message &msg)
{
  // get the map of request => client
  ServiceRequestIdMap::iterator it;
  it = _serviceToClient.find(service);
  // should not fail
  if (it == _serviceToClient.end())
  {
    //log error
    return;
  }

  RequestIdMap &request = it->second;
  RequestIdMap::iterator itReq;

  itReq = request.find(msg.id());
  if (itReq != request.end())
  {
    if (itReq->second.second == 0)
      handleGatewayServiceRead(service, msg);
    else
    {
      // S.3
      // id should be rewritten
      qi::Message ans(msg);
      ans.setId(itReq->second.first);
      itReq->second.second->send(ans);
    }
  }
}

void GatewayPrivate::onReadyRead(TransportSocket *client, const qi::Message &msg)
{
  if (std::find(_clients.begin(), _clients.end(), client) != _clients.end())
  {
    // Client
    handleClientRead(client, msg);
  }
  else
  {
    // Server
    handleServiceRead(client, msg);
  }
}

void GatewayPrivate::onWriteDone(TransportSocket *client)
{
}

// S.2/ new service connection                           => handle pending message, jump to S.3
void GatewayPrivate::onConnected(TransportSocket *service)
{
  if (service == _tso)
    return;

  std::string serviceName;
  ServiceSocketMap::const_iterator it;

  //TODO: optimise?  O(log(n)) instead O(n)
  for (it = _services.begin(); it != _services.end(); ++it)
  {
    if ((TransportSocket *)it->second == service)
    {
      serviceName = it->first;
      break;
    }
  }

  if (serviceName.empty())
  {
    qiLogError("Gateway", "fail baby\n");
    return;
  }

  //handle pending message
  PendingMessageVector &pmv = _pendingMessage[serviceName];
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

void Gateway::start(const std::string &addr, unsigned short port, qi::Session *session)
{
  std::stringstream ss;
  ss << "tcp://" << addr << ":" << port;
  _p->_endpoints.push_back(ss.str());

  _p->_ts.setDelegate(_p);
  _p->_ts.start(addr, port, session->_nthd->getEventBase());
}
} // !qi
