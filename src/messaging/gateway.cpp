/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <string>

#include <qi/trackable.hpp>
#include <qi/atomic.hpp>
#include <qi/os.hpp>
#include <qi/api.hpp>
#include <qi/log.hpp>
#include <qi/session.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/future.hpp>
#include <qi/signal.hpp>

#include "tcptransportsocket.hpp"
#include "servicedirectoryclient.hpp"
#include "transportserver.hpp"
#include "clientauthenticator_p.hpp"
#include "gateway_p.hpp"

qiLogCategory("qimessaging.gateway");

namespace
{
static const unsigned int DefaultServiceDirectoryPort = 9559;
static const ServiceId ServiceSD = qi::Message::Service_ServiceDirectory;
static const auto UpdateEndpointsPeriod = qi::Seconds(5);

class NullTransportSocket : public qi::TransportSocket
{
public:
  virtual qi::FutureSync<void> connect(const qi::Url&)
  {
    qi::Promise<void> p;
    qi::Future<void> f = p.future();

    p.setValue(0);
    return f;
  }
  virtual qi::FutureSync<void> disconnect()
  {
    return connect(qi::Url());
  }
  virtual bool send(const qi::Message&)
  {
    return true;
  }
  virtual void startReading()
  {
  }
  virtual qi::Url remoteEndpoint() const
  {
    return qi::Url();
  }
};

struct _pending_msg_eraser
{
  _pending_msg_eraser(qi::TransportSocketPtr socket)
    : target(socket)
  {
  }
  bool operator()(const boost::tuple<ClientMessageId, qi::Message, qi::TransportSocketPtr>& p)
  {
    return boost::tuples::get<2>(p) == target;
  }
  qi::TransportSocketPtr target;
};

typedef boost::shared_ptr<bool> boolptr;
}

namespace qi
{
void GwTransaction::forceDestination(TransportSocketPtr dest)
{
  _destination = dest;
}
void GwTransaction::setDestinationIfNull(TransportSocketPtr dest)
{
  if (!_destination)
    _destination = dest;
}

Gateway::Gateway(bool enforceAuth)
  : _p(boost::make_shared<GatewayPrivate>(enforceAuth))
  , connected(_p->connected)
{
  _p->setAuthProviderFactory(boost::make_shared<NullAuthProviderFactory>());
  _p->setClientAuthenticatorFactory(boost::make_shared<NullClientAuthenticatorFactory>());
}

Gateway::~Gateway()
{
  close();
}

void Gateway::close()
{
  _p->close(true);
}

void Gateway::setAuthProviderFactory(AuthProviderFactoryPtr provider)
{
  _p->setAuthProviderFactory(provider);
}

void Gateway::setLocalClientAuthProviderFactory(AuthProviderFactoryPtr provider)
{
  _p->setLocalClientAuthProviderFactory(provider);
}

void Gateway::setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr authenticator)
{
  _p->setClientAuthenticatorFactory(authenticator);
}

UrlVector Gateway::endpoints() const
{
  return _p->endpoints();
}

bool Gateway::listen(const Url& url)
{
  return _p->listen(url);
}

qi::Future<void> Gateway::attachToServiceDirectory(const Url& serviceDirectoryUrl)
{
  return _p->connect(serviceDirectoryUrl);
}

GatewayPrivate::GatewayPrivate(bool ea)
  : _enforceAuth(ea)
  , _dying(false)
{
  _socketCache.init();
  _server.newConnection.connect(&GatewayPrivate::onClientConnection, this, _1);
  _localServer.listen("tcp://127.0.0.1:0");
  _localServer.newConnection.connect(&GatewayPrivate::onLocalClientConnection, this, _1);
  _localClientAuthProviderFactory = boost::make_shared<NullAuthProviderFactory>();
}

GatewayPrivate::~GatewayPrivate()
{
  _dying = true;
  close();
  Trackable::destroy();
}

void GatewayPrivate::close(bool clearEndpoints)
{
  qiLogInfo() << "Bringing the gateway down";
  _updateEndpointsTask.stop();
  if (_retryFut.isRunning())
    _retryFut.cancel();
  if (_sdClient.isConnected())
    _sdClient.socket()->disconnected.disconnectAll();
  _server.close();
  _sdClient.close();
  _sdClient.serviceAdded.disconnectAll();
  _sdClient.serviceRemoved.disconnectAll();
  _socketCache.close();
  {
    std::vector<qi::Future<void>> disconnections;
    {
      boost::recursive_mutex::scoped_lock lock(_serviceMutex);
      disconnections.reserve(_services.size());
      for (const auto& serviceSlot : _services)
      {
        if (serviceSlot.second && serviceSlot.first != ServiceSD && serviceSlot.second->isConnected())
          disconnections.emplace_back(serviceSlot.second->disconnect());
      }
      _services.clear();
      _sdAvailableServices.clear();
    }
    qi::waitForAll(disconnections);
  }
  {
    std::vector<qi::Future<void>> disconnections;
    {
      boost::mutex::scoped_lock lock(_clientsMutex);
      for (auto& client : _clients)
      {
        disconnections.emplace_back(client->disconnect());
      }
    }
    qi::waitForAll(disconnections);
    _clients.clear();
  }
  {
    boost::mutex::scoped_lock lock(_ongoingMsgMutex);
    _ongoingMessages.clear();
  }
  {
    boost::mutex::scoped_lock lock(_pendingMsgMutex);
    _pendingMessages.clear();
  }
  {
    boost::recursive_mutex::scoped_lock lock(_eventSubMutex);
    _pendingEventSubscriptions.clear();
    _eventSubscribers.clear();
  }
  if (clearEndpoints)
    _endpoints.clear();
}

void GatewayPrivate::setAuthProviderFactory(AuthProviderFactoryPtr provider)
{
  _authProviderFactory = provider;
}

void GatewayPrivate::setLocalClientAuthProviderFactory(AuthProviderFactoryPtr provider)
{
  _localClientAuthProviderFactory = provider;
}

void GatewayPrivate::setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr authenticator)
{
  _clientAuthenticatorFactory = authenticator;
  _sdClient.setClientAuthenticatorFactory(authenticator);
}

UrlVector GatewayPrivate::endpoints() const
{
  return _endpoints;
}

static UrlVector allAvailableInterfaces(bool includeLocalhost, unsigned int port, const std::string& protocol)
{
  typedef std::map<std::string, std::vector<std::string> > IfMap;
  std::map<std::string, std::vector<std::string> > interfaces = os::hostIPAddrs(false);
  std::vector<Url> result;

  for (IfMap::iterator it = interfaces.begin(), end = interfaces.end(); it != end; ++it)
  {
    for (std::vector<std::string>::iterator uit = it->second.begin(), uend = it->second.end(); uit != uend; ++uit)
    {
      if (*uit == "127.0.0.1" || *uit == "localhost")
      {
        if (includeLocalhost)
          result.push_back(Url(*uit, protocol, port));
      }
      else
        result.push_back(Url(*uit, protocol, port));
    }
  }
  return result;
}

void GatewayPrivate::onServerAcceptError(int err)
{
  qiLogWarning() << "Accept error, interfaces changed (error " << err << ")";
  listen(_listenUrl);
  _server.endpointsChanged();
}

bool GatewayPrivate::listen(const Url& url)
{
  _listenUrl = url;

  const std::string& host = url.host();
  const unsigned int port = url.port();

  if ((host == "127.0.0.1" || host == "localhost") && port == DefaultServiceDirectoryPort)
    throw std::runtime_error("Address 127.0.0.1:9559 is reserved.");

  _server.acceptError.connect(&GatewayPrivate::onServerAcceptError, this, _1);

  _updateEndpointsTask.stop();
  if (host == "0.0.0.0" && port == DefaultServiceDirectoryPort)
  {
    _updateEndpointsTask.setPeriod(UpdateEndpointsPeriod);
    _updateEndpointsTask.setCallback(qi::bind(&GatewayPrivate::updateEndpoints, this, url));
    _updateEndpointsTask.start();
    return true;
  }
  else
  {
    qi::Future<void> fut = _server.listen(url);
    if (fut.hasError())
    {
      qiLogError() << "Can't listen on " << url.str();
      return false;
    }
    else
    {
      qiLogInfo() << "Listening on " << url.str();
      _endpoints = _server.endpoints();
      return true;
    }
  }
}

void GatewayPrivate::updateEndpoints(const qi::Url& url)
{
  auto filteredUrls = allAvailableInterfaces(false, url.port(), url.protocol());
  std::sort(filteredUrls.begin(), filteredUrls.end());

  UrlVector toDo;
  std::set_difference(filteredUrls.begin(),
                      filteredUrls.end(),
                      _pendingListens.begin(),
                      _pendingListens.end(),
                      std::back_inserter(toDo));

  // TODO we listen on new interfaces, but we don't stop listening on interfaces that have disappeared. This is because
  // we don't have a method to stop listening, it must be implemented first

  for (const auto& url : toDo)
  {
    try
    {
      qiLogInfo() << "New address " << url.str() << ", trying to listen";
      _server.listen(url)
          .thenR<void>(qi::bind(boost::function<void(GatewayPrivate*, qi::Future<void>)>(
                                                                [url](GatewayPrivate* p, qi::Future<void> fut)
                                                                {
                                                                  if (fut.hasError())
                                                                    qiLogWarning() << "Failed to listen on "
                                                                                   << url.str() << ": " << fut.error();
                                                                  else
                                                                    qiLogWarning() << "Now listening on " << url.str();
                                                                  std::lock_guard<std::mutex> _(p->_endpointsMutex);
                                                                  p->_endpoints = p->_server.endpoints();
                                                                }),
                                                            this,
                                                            _1));
      _pendingListens.insert(url);
    }
    catch (std::exception& e)
    {
      qiLogWarning() << "Failed to listen on " << url.str() << ": " << e.what();
    }
  }
}

TransportSocketPtr GatewayPrivate::safeGetService(ServiceId id)
{
  boost::recursive_mutex::scoped_lock lock(_serviceMutex);
  std::map<ServiceId, TransportSocketPtr>::iterator it;
  if ((it = _services.find(id)) == _services.end())
    return TransportSocketPtr();
  return it->second;
}

void GatewayPrivate::onClientDisconnected(TransportSocketPtr socket, std::string url, const std::string& reason)
{
  qiLogVerbose() << "Client " << url << " has left us: " << reason;
  {
    boost::recursive_mutex::scoped_lock lock(_eventSubMutex);
    _eventSubscribers.erase(socket);
    EventsEndpointMap::iterator it = _eventSubscribers.begin();
    while (it != _eventSubscribers.end())
    {
      EventServiceMap::iterator sit = it->second.begin();
      while (sit != it->second.end())
      {
        ServiceId serviceId = sit->first;
        EventsPerObjectMap::iterator oit = sit->second.begin();
        while (oit != sit->second.end())
        {
          unsigned int objectId = oit->first;
          ClientsPerEventMap::iterator eit = oit->second.begin();
          while (eit != oit->second.end())
          {
            qi::uint32_t eventId = eit->first;
            removeEventSubscriber(serviceId, objectId, eventId, socket, it->first);
            if (eit->second.remoteSubscribers.size() == 0)
              eit = oit->second.erase(eit);
            else
              ++eit;
          }
          if (oit->second.size() == 0)
            oit = sit->second.erase(oit);
          else
            ++oit;
        }
        if (sit->second.size() == 0)
          sit = it->second.erase(sit);
        else
          ++sit;
      }
      if (it->second.size() == 0)
        it = _eventSubscribers.erase(it);
      else
        ++it;
    }
  }
  {
    boost::mutex::scoped_lock lock(_ongoingMsgMutex);
    OngoingMessagesMap::iterator it = _ongoingMessages.begin();
    OngoingMessagesMap::iterator end = _ongoingMessages.end();
    for (; it != end; ++it)
    {
      IdLookupMap::iterator msgIt = it->second.begin();
      IdLookupMap::iterator msgEnd = it->second.end();
      while (msgIt != msgEnd)
      {
        if (msgIt->second.second == socket)
          it->second.erase(msgIt++);
        else
          msgIt++;
      }
    }
  }
  {
    boost::mutex::scoped_lock lock(_pendingMsgMutex);
    PendingMessagesMap::iterator it = _pendingMessages.begin();
    PendingMessagesMap::iterator end = _pendingMessages.end();
    while (it != end)
    {
      it->second.erase(std::remove_if(it->second.begin(), it->second.end(), _pending_msg_eraser(socket)),
                       it->second.end());
      if (it->second.size() == 0)
        _pendingMessages.erase(it++);
      else
        ++it;
    }
  }
  {
    // Check if this client was hosting any service,
    // and if so clean them up.
    boost::recursive_mutex::scoped_lock lock(_serviceMutex);
    std::map<ServiceId, TransportSocketPtr>::iterator it = _services.begin();
    std::map<ServiceId, TransportSocketPtr>::iterator end = _services.end();
    for (; it != end;)
      if (it->second == socket)
      {
        ServiceId sid = it->first;
        ++it;
        serviceDisconnected(sid);
        unregisterServiceFromSD(sid);
      }
      else
        ++it;
  }
  {
    boost::mutex::scoped_lock lock(_clientsMutex);
    _clients.erase(std::remove(_clients.begin(), _clients.end(), socket), _clients.end());
  }
  _objectHost.clientDisconnected(socket);
  socket->messageReady.disconnectAll();
  socket->disconnected.disconnectAll();
}

Future<void> GatewayPrivate::unregisterServiceFromSD(ServiceId sid)
{
  if (_sdClient.isConnected())
    return _sdClient.unregisterService(sid);
  return makeFutureError<void>("SD disconnected.");
}

void GatewayPrivate::sdConnectionRetry(const qi::Url& sdUrl, qi::Duration lastTimer)
{
  if (*_dying)
    return;

  qi::Future<void> fut = connect(sdUrl);
  if (fut.hasError())
  {
    lastTimer *= 2;
    qiLogWarning() << "Can't reach ServiceDirectory at address " << sdUrl.str() << ", retrying in "
                   << qi::to_string(boost::chrono::duration_cast<qi::Seconds>(lastTimer)) << ".";
    _retryFut = qi::asyncDelay(qi::bind(&GatewayPrivate::sdConnectionRetry, this, sdUrl, lastTimer), lastTimer);
  }
  else
  {
    qiLogInfo() << "Successfully reestablished connection to the ServiceDirectory at address " << sdUrl.str();
    const auto endpointsToReconnect = _endpoints;
    for (const auto& endpointUrl : endpointsToReconnect)
    {
      qiLogInfo() << "Trying reconnection to " << endpointUrl.str();
      if (listen(endpointUrl))
      {
        qiLogInfo() << "Reconnected to " << endpointUrl.str();
      }
      else
      {
        qiLogInfo() << "Reconnection failed: " << endpointUrl.str();
      }
    }

  }
}

void GatewayPrivate::onServiceDirectoryDisconnected(TransportSocketPtr socket, const std::string& reason)
{
  if (*_dying)
    return;
  connected.set(false);
  qiLogWarning() << "Lost connection to the ServiceDirectory: " << reason;
  qiLogWarning() << "Kicking out all clients until the connection is re-established.";
  close();
  qi::Duration retryTimer = qi::Seconds(1);

  _retryFut =
      qi::asyncDelay(qi::bind(&GatewayPrivate::sdConnectionRetry, this, socket->url(), retryTimer), retryTimer);
}

void GatewayPrivate::serviceDisconnected(ServiceId sid)
{
  qiLogVerbose() << "Disconnecting service #" << sid;
  invalidateClientsMessages(sid);
  {
    boost::recursive_mutex::scoped_lock lock(_eventSubMutex);

    for (EventsEndpointMap::iterator it = _eventSubscribers.begin(), end = _eventSubscribers.end(); it != end; ++it)
    {
      EventServiceMap::iterator sit = it->second.find(sid);
      if (sit != it->second.end())
      {
        sit->second.clear();
        it->second.erase(sit);
      }
    }
  }
  {
    boost::recursive_mutex::scoped_lock lock(_serviceMutex);
    if (_services.find(sid) != _services.end())
      _services.erase(sid);
  }
  _objectHost.serviceDisconnected(sid);
}

void GatewayPrivate::onSdServiceAdded(ServiceId id, const std::string& name)
{
  qiLogVerbose() << "Service #" << id << " [" << name << "] was registered.";
  {
    boost::recursive_mutex::scoped_lock lock(_serviceMutex);
    _sdAvailableServices[id] = name;
  }
}

void GatewayPrivate::onSdServiceRemoved(ServiceId id)
{
  qiLogVerbose() << "Service #" << id << " unregistered.";
  bool isOnGateway = false;
  {
    boost::recursive_mutex::scoped_lock lock(_serviceMutex);
    _sdAvailableServices.erase(id);
    isOnGateway = _services.find(id) != _services.end();
  }
  if (isOnGateway)
    serviceDisconnected(id);
}

void GatewayPrivate::onSdConnected(Future<void> fut, Promise<void> prom)
{
  if (fut.hasError())
    return prom.setError(fut.error());
  TransportSocketPtr sdSocket = _sdClient.socket();

  _services[ServiceSD] = sdSocket;
  // Additional checks are required for some of the SD's messages, so it gets
  // it's own messageReady callback.
  sdSocket->messageReady.connect(&GatewayPrivate::onServiceDirectoryMessageReady, this, _1, sdSocket);
  sdSocket->disconnected.connect(&GatewayPrivate::onServiceDirectoryDisconnected, this, sdSocket, _1);

  std::string mid = _sdClient.machineId();
  _socketCache.insert(mid, sdSocket->url(), sdSocket);

  // We need to keep track of the client present on the SD, so we can know whether
  // or not we can deliver a client's messages.
  _sdClient.serviceAdded.connect(&GatewayPrivate::onSdServiceAdded, this, _1, _2);
  _sdClient.serviceRemoved.connect(&GatewayPrivate::onSdServiceRemoved, this, _1);
  EventSubInfo manualInfo;
  manualInfo.gwLink = 0;
  // We use a null transport socket to avoid additional checks when events are triggered:
  // instead we'll run a nop function.
  manualInfo.remoteSubscribers[boost::make_shared<NullTransportSocket>()] = 0;
  ClientsPerEventMap& sdEvents = _eventSubscribers[sdSocket][ServiceSD][Message::GenericObject_Main];
  sdEvents[_sdClient.metaObject().signalId("serviceAdded")] = manualInfo;
  manualInfo.gwLink = 1;
  sdEvents[_sdClient.metaObject().signalId("serviceRemoved")] = manualInfo;

  ServiceInfoVector services = _sdClient.services();
  ServiceInfoVector::const_iterator it = services.begin();
  ServiceInfoVector::const_iterator end = services.end();

  qiLogVerbose() << "Keeping track of any services already in the SD...";
  for (; it != end; ++it)
  {
    qiLogVerbose() << "Keeping track of service " << it->name() << " (#" << it->serviceId() << ")";
    _sdAvailableServices[it->serviceId()] = it->name();
  }
  qiLogInfo() << "Gateway is ready.";
  prom.setValue(0);
  connected.set(true);
}

Future<void> GatewayPrivate::connect(const Url& sdUrl)
{
  qi::Promise<void> prom;
  qi::Future<void> fut = _sdClient.connect(sdUrl);

  fut.connect(&GatewayPrivate::onSdConnected, this, _1, prom);
  return prom.future();
}

void GatewayPrivate::forwardMessage(ClientMessageId origId,
                                    const Message& forward,
                                    TransportSocketPtr origin,
                                    TransportSocketPtr destination)
{
  ServiceId service = forward.service();
  GWMessageId gwId = forward.id();

  if (destination)
  {
    // If our destination is valid, we can send the `forward` message as-is.
    // Its ID is gateway-generated, hence is unique on our side. We use
    // it as our key in our map message.
    qiLogDebug() << "Forward message: " << forward.address() << " Original id:" << origId
                 << " Origin: " << origin.get();
    {
      boost::mutex::scoped_lock lock(_ongoingMsgMutex);
      _ongoingMessages[service][gwId] = std::make_pair(origId, origin);
    }
    destination->send(forward);
  }
  else
  {
    // If the destination is not valid, we send an error back to the client using
    // the original message's ID.
    Message forged(Message::Type_Error, forward.address());
    forged.setId(origId);
    serviceUnavailable(service, forged, origin);
  }
}

void GatewayPrivate::clientAuthenticationMessages(const Message& msg,
                                                  TransportSocketPtr socket,
                                                  AuthProviderPtr auth,
                                                  boolptr firstMessage,
                                                  SignalSubscriberPtr sub)
{
  int id = msg.id();
  int service = msg.service();
  int function = msg.function();
  int type = msg.type();
  const std::string client_endpoint = socket->remoteEndpoint().str();
  std::string cmsig = typeOf<CapabilityMap>()->signature().toString();
  Message reply;

  reply.setId(id);
  reply.setService(service);
  reply.setFunction(function);
  if (service != Message::Service_Server || type != Message::Type_Call ||
      function != Message::ServerFunction_Authenticate)
  {
    socket->messageReady.disconnect(*sub);
    if (_enforceAuth)
    {
      qiLogVerbose() << "Client tried to bypass authentication";
      reply.setType(Message::Type_Error);
      reply.setError("Not authenticated.");
      socket->send(reply);
      socket->disconnect();
    }
    else
    {
      qiLogVerbose() << "Client not authenticating, but not enforced. Skipping auth, but sending capabilities.";
      Message caps;
      caps.setType(Message::Type_Capability);
      caps.setService(Message::Service_Server);
      caps.setValue(socket->localCapabilities(), typeOf<CapabilityMap>()->signature());
      socket->send(caps);

      socket->messageReady.connect(&GatewayPrivate::onAnyMessageReady, this, _1, socket);
      onAnyMessageReady(msg, socket);
    }
    return;
  }
  qiLogVerbose() << "Starting authentication step for client " << client_endpoint << "...";

  AnyReference cmref = msg.value(typeOf<CapabilityMap>()->signature(), socket);
  CapabilityMap authData = cmref.to<CapabilityMap>();
  cmref.destroy();
  authData = auth->processAuth(authData);
  AuthProvider::State state = authData[AuthProvider::State_Key].to<AuthProvider::State>();
  switch (state)
  {
  case AuthProvider::State_Done:
    qiLogVerbose() << "Client " << client_endpoint << " has successfully authenticated.";
    socket->messageReady.disconnect(*sub);
    socket->messageReady.connect(&GatewayPrivate::onAnyMessageReady, this, _1, socket);
  // Absence of `break` intentional: we want to send a reply in both cases.
  case AuthProvider::State_Cont:
    if (*firstMessage)
    {
      const CapabilityMap& sockCaps = socket->localCapabilities();
      authData.insert(sockCaps.begin(), sockCaps.end());
      *firstMessage = false;
    }
    reply.setValue(AnyReference::from(authData), cmsig);
    reply.setType(Message::Type_Reply);
    socket->send(reply);
    break;
  default:
    qiLogError() << "Unknown state: " << state;
    assert(false);
  case AuthProvider::State_Error:
  {
    std::stringstream builder;

    builder << "Authentication failed";
    if (authData.find(AuthProvider::Error_Reason_Key) != authData.end())
    {
      builder << ": " << authData[AuthProvider::Error_Reason_Key].to<std::string>() << " [auth v"
              << _authProviderFactory->authVersionMajor() << "." << _authProviderFactory->authVersionMinor() << "]";
    }
    reply.setType(Message::Type_Error);
    reply.setError(builder.str());
    socket->send(reply);
    socket->disconnect();
    qiLogVerbose() << builder.str();
  }
  }
  qiLogVerbose() << "Authentication step for client " << client_endpoint << " has ended.";
}

void GatewayPrivate::onClientConnection(TransportSocketPtr socket)
{
  qiLogVerbose() << "Client " << socket->remoteEndpoint().str() << " has knocked knocked knocked on the gateway";
  SignalSubscriberPtr sub = boost::make_shared<SignalSubscriber>();
  boolptr firstMessage = boost::make_shared<bool>(true);

  *sub = socket->messageReady.connect(&GatewayPrivate::clientAuthenticationMessages,
                                      this,
                                      _1,
                                      socket,
                                      _authProviderFactory->newProvider(),
                                      firstMessage,
                                      sub);
  socket->disconnected.connect(
      &GatewayPrivate::onClientDisconnected, this, socket, socket->remoteEndpoint().str(), _1);
  socket->startReading();
  {
    boost::mutex::scoped_lock lock(_clientsMutex);
    _clients.push_back(socket);
  }
}

void GatewayPrivate::onLocalClientConnection(TransportSocketPtr socket)
{
  qiLogVerbose() << "Client " << socket->remoteEndpoint().str() << " has connected on the local endpoint.";
  SignalSubscriberPtr sub = boost::make_shared<SignalSubscriber>();
  boolptr firstMessage = boost::make_shared<bool>(true);

  *sub = socket->messageReady.connect(&GatewayPrivate::clientAuthenticationMessages,
                                      this,
                                      _1,
                                      socket,
                                      _localClientAuthProviderFactory->newProvider(),
                                      firstMessage,
                                      sub);
  socket->disconnected.connect(
      &GatewayPrivate::onClientDisconnected, this, socket, socket->remoteEndpoint().str(), _1);
  socket->startReading();
  {
    boost::mutex::scoped_lock lock(_clientsMutex);
    _clients.push_back(socket);
  }
}

void GatewayPrivate::forgeServiceInfo(ServiceInfo& serviceInfo)
{
  // When a ServiceInfo is heading outside the robot, we want to
  // actually hand out the Gateway's endpoint, not the service's ones.
  qi::UrlVector gwEndpoints = _server.endpoints();
  const qi::UrlVector& localEndpoints = _localServer.endpoints();

  gwEndpoints.insert(gwEndpoints.end(), localEndpoints.begin(), localEndpoints.end());
  serviceInfo.setEndpoints(gwEndpoints);
  serviceInfo.setMachineId(qi::os::getMachineId());
  serviceInfo.setProcessId(qi::os::getpid());
}

void GatewayPrivate::startServiceAuthentication(TransportSocketPtr serviceSocket, ServiceId sid)
{
  ClientAuthenticatorPtr authenticator = _clientAuthenticatorFactory->newAuthenticator();
  CapabilityMap socketCaps = serviceSocket->localCapabilities();
  {
    CapabilityMap tmp = authenticator->initialAuthData();
    for (CapabilityMap::iterator it = tmp.begin(), end = tmp.end(); it != end; ++it)
      socketCaps[AuthProvider::UserAuthPrefix + it->first] = it->second;
  }
  SignalSubscriberPtr sub = boost::make_shared<SignalSubscriber>();
  *sub = serviceSocket->messageReady.connect(
      &GatewayPrivate::serviceAuthenticationMessages, this, _1, serviceSocket, sid, authenticator, sub);

  Message msgAuth;
  msgAuth.setFunction(Message::ServerFunction_Authenticate);
  msgAuth.setService(Message::Service_Server);
  msgAuth.setType(Message::Type_Call);
  msgAuth.setValue(socketCaps, typeOf<CapabilityMap>()->signature());
  serviceSocket->send(msgAuth);
}

void GatewayPrivate::serviceAuthenticationMessages(const Message& msg,
                                                   TransportSocketPtr service,
                                                   ServiceId sid,
                                                   ClientAuthenticatorPtr authenticator,
                                                   SignalSubscriberPtr sub)
{
  qiLogVerbose() << "Service Authentication Messages";
  std::string cmsig = typeOf<CapabilityMap>()->signature().toString();
  // Error on authentication results in termination of the connection.
  // We'll let the callback take care of cleaning up.
  if (msg.type() == Message::Type_Error)
  {
    AnyReference ref = msg.value("s", service);
    std::string error = ref.to<std::string>();
    ref.destroy();
    qiLogError() << error;
    return;
  }
  AnyReference cmref = msg.value(cmsig, service);
  CapabilityMap authData = cmref.to<CapabilityMap>();
  cmref.destroy();
  if (authData[AuthProvider::State_Key].to<AuthProvider::State>() == AuthProvider::State_Done)
  {
    service->messageReady.disconnect(*sub);
    service->messageReady.connect(&GatewayPrivate::onAnyMessageReady, this, _1, service);
    localServiceRegistrationEnd(service, sid);
    return;
  }

  Message next;
  authData = authenticator->processAuth(authData);
  next.setService(Message::Service_Server);
  next.setType(Message::Type_Call);
  next.setValue(AnyReference::from(authData), cmsig);
  next.setFunction(Message::ServerFunction_Authenticate);
  service->send(next);
}

void GatewayPrivate::localServiceRegistrationCont(Future<TransportSocketPtr> fut, ServiceId sid)
{
  if (fut.hasError())
    return invalidateClientsMessages(sid);
  TransportSocketPtr socket = fut.value();

  // This method is called by the transportsocketcache, which
  // means the socket may have alread been used: check that.
  if (!socket->hasReceivedRemoteCapabilities())
    startServiceAuthentication(socket, sid);
  else
    localServiceRegistrationEnd(socket, sid);
}

void GatewayPrivate::localServiceRegistrationEnd(TransportSocketPtr socket, ServiceId sid)
{
  {
    boost::recursive_mutex::scoped_lock lock(_serviceMutex);
    _services[sid] = socket;
  }
  {
    boost::mutex::scoped_lock lock(_pendingMsgMutex);
    std::vector<boost::tuple<ClientMessageId, Message, TransportSocketPtr> >::iterator it =
        _pendingMessages[sid].begin();
    std::vector<boost::tuple<ClientMessageId, Message, TransportSocketPtr> >::iterator end =
        _pendingMessages[sid].end();

    // Once we have established a connection to the new service,
    // we can send it all the messages we had pending for him.
    for (; it != end; ++it)
      forwardMessage(boost::tuples::get<0>(*it),
                     boost::tuples::get<1>(*it),
                     boost::tuples::get<2>(*it),
                     safeGetService(boost::tuples::get<1>(*it).service()));
    _pendingMessages[sid].clear();
    _pendingMessages.erase(sid);
  }
  qiLogVerbose() << "Done registering local service";
}

void GatewayPrivate::removeEventSubscriber(ServiceId sid,
                                           uint32_t object,
                                           uint32_t event,
                                           EventSubscriberEndpoint client,
                                           EventHostEndpoint host,
                                           ClientMessageId fakeId)
{
  int remainingSubs = 0;
  SignalLink gwLink = 0;
  {
    boost::recursive_mutex::scoped_lock lock(_eventSubMutex);
    EventSubInfo& info = _eventSubscribers[host][sid][object][event];

    gwLink = info.gwLink;
    info.remoteSubscribers.erase(client);
    remainingSubs = info.remoteSubscribers.size();
  }
  // no more subscribers: unregister the gateway from the event
  // send the message to the service host
  if (remainingSubs == 0)
  {
    AnyReferenceVector v;
    // We forge a new message just to insert the GW's link
    // instead of the client's, since it's the GW that's registered
    // remotely.
    Message forged;

    if (sid == Message::Service_Server && object > static_cast<unsigned int>(Message::GenericObject_Main))
    {
      // Check if we have a client object.
      // Retrieve the original serviceID and objectID, otherwise the message won't
      // reach the intended destination.
      ObjectAddress addr = _objectHost.getOriginalObjectAddress(ObjectAddress(sid, object));
      sid = addr.service;
      object = addr.object;
    }
    v.push_back(AnyReference::from(object));
    v.push_back(AnyReference::from(event));
    v.push_back(AnyReference::from(gwLink));
    forged.setType(Message::Type_Call);
    forged.setService(sid);
    forged.setFunction(Message::BoundObjectFunction_UnregisterEvent);
    forged.setObject(object);
    forged.setValues(v, "(IIL)");

    // We shouldn't care about the service's response so we can just go in raw
    host->send(forged);
  }
}

// This callback is called as a result of sd->service : use the endpoint to connect to the
// service
void GatewayPrivate::localServiceRegistration(Future<ServiceInfo> serviceInfoFut, ServiceId targetService)
{
  if (serviceInfoFut.hasError())
    return invalidateClientsMessages(targetService);

  qiLogVerbose() << "Starting local service registration";
  qi::Future<TransportSocketPtr> fut = _socketCache.socket(serviceInfoFut.value(), "");
  fut.connect(&GatewayPrivate::localServiceRegistrationCont, this, _1, targetService);
}

void GatewayPrivate::serviceUnavailable(ServiceId service, const Message& subject, TransportSocketPtr client)
{
  Message unavailable;
  std::stringstream err;

  err << "Service " << service << " is unavailable." << std::endl;
  unavailable.setType(Message::Type_Error);
  unavailable.setId(subject.id());
  unavailable.setService(service);
  unavailable.setError(err.str());
  unavailable.setFunction(subject.function());
  unavailable.setObject(subject.object());
  client->send(unavailable);
}

GWMessageId GatewayPrivate::handleCallMessage(GwTransaction& t, TransportSocketPtr origin)
{
  qiLogDebug() << "Handle call " << t.content.address();
  Message& msg = t.content;
  ServiceId targetService = msg.service();
  Message forward;
  TransportSocketPtr serviceSocket = safeGetService(targetService);

  t.setDestinationIfNull(serviceSocket);
  forward.setType(msg.type());
  forward.setService(msg.service());
  forward.setObject(msg.object());
  forward.setFunction(msg.function());
  forward.setBuffer(msg.buffer());
  forward.setFlags(msg.flags());
  // Check if we already have a connection to this service
  if (!serviceSocket || !serviceSocket->isConnected())
  {
    qiLogVerbose() << "No connection to service " << targetService << "...";
    std::map<ServiceId, std::string>::iterator it;
    std::map<ServiceId, std::string>::iterator end;
    {
      boost::recursive_mutex::scoped_lock lock(_serviceMutex);
      it = _sdAvailableServices.find(targetService);
      end = _sdAvailableServices.end();
    }
    // If the service doesn't exist even on the SD, then it's an error:
    // let the client know his message won't be delivered.
    if (it == end)
    {
      qiLogWarning() << "Service " << targetService << " is unavailable.";
      serviceUnavailable(targetService, msg, origin);
    }
    // The service exists on the SD: request its informations and connect to it
    else
    {
      bool requestService = false;
      qiLogVerbose() << " > Querying the SD for more info on service \"" << it->second << "\"";
      {
        boost::mutex::scoped_lock lock(_pendingMsgMutex);
        // if there are pendingMessages already, it means we're already actively trying to
        // connect to the service: don't request it a second time.
        requestService = _pendingMessages[targetService].size() == 0;
        _pendingMessages[targetService].push_back(boost::make_tuple(msg.id(), forward, origin));
      }
      if (requestService)
      {
        qi::Future<ServiceInfo> fut = _sdClient.service(it->second);
        fut.connect(&GatewayPrivate::localServiceRegistration, this, _1, targetService);
      }
    }
    return forward.id();
  }
  forwardMessage(msg.id(), forward, origin, t.destination());
  return forward.id();
}

void GatewayPrivate::handleReplyMessage(GwTransaction& t)
{
  Message& msg = t.content;
  ServiceId service = msg.service();
  GWMessageId gwId = msg.id();
  std::pair<ClientMessageId, TransportSocketPtr> client;

  if (service == 0 && msg.object() > 1)
  {
    // We have a client object: we need to retrieve the original serviceID.
    service = _objectHost.getOriginalObjectAddress(ObjectAddress(service, msg.object())).service;
  }

  {
    boost::mutex::scoped_lock lock(_ongoingMsgMutex);
    // This is likely an internal message and so can be ignored here
    if (_ongoingMessages[service].find(gwId) == _ongoingMessages[service].end())
    {
      qiLogDebug() << "Reply with no original message [" << gwId << "]: " << t.content.address();
      return;
    }
    client = _ongoingMessages[service][gwId];
    _ongoingMessages[service].erase(gwId);
  }

  qiLogDebug() << "Reply to socket " << client.second << " with original ID " << client.first;
  Message answer(msg);
  answer.setId(client.first);
  t.setDestinationIfNull(client.second);
  if (t.destination()->isConnected())
  {
    qiLogVerbose() << "Reply: " << msg.address();
    t.destination()->send(answer);
  }
}

void GatewayPrivate::handleEventMessage(GwTransaction& t, TransportSocketPtr socket)
{
  Message& msg = t.content;
  ServiceId service = msg.service();
  unsigned int object = msg.object();
  unsigned int event = msg.event();

  {
    boost::recursive_mutex::scoped_lock lock(_eventSubMutex);
    qiLogDebug() << "Handling event " << service << "." << object << "." << event << "...";
    EventsEndpointMap::iterator endpointIt;
    EventServiceMap::iterator serviceIt;
    EventsPerObjectMap::iterator objectIt;
    ClientsPerEventMap::iterator eventIt;
    if ((endpointIt = _eventSubscribers.find(socket)) == _eventSubscribers.end() ||
        (serviceIt = endpointIt->second.find(service)) == endpointIt->second.end() ||
        (objectIt = serviceIt->second.find(object)) == serviceIt->second.end() ||
        (eventIt = objectIt->second.find(event)) == objectIt->second.end())
    {
      qiLogDebug() << "No subscribers.";
      return;
    }

    std::map<TransportSocketPtr, SignalLink>& subs = eventIt->second.remoteSubscribers;

    qiLogDebug() << "Forwarding event to " << subs.size() << " subscribers.";
    for (std::map<TransportSocketPtr, SignalLink>::iterator it = subs.begin(), end = subs.end(); it != end; ++it)
      it->first->send(msg);
  }
}

void GatewayPrivate::onAnyMessageReady(const Message& msg, TransportSocketPtr socket)
{
  GwTransaction transaction(msg);

  _objectHost.treatMessage(transaction, socket);
  qiLogDebug() << socket.get() << " Transaction ready: " << Message::typeToString(transaction.content.type()) << " "
               << transaction.content.address();
  unsigned int function = msg.function();
  switch (msg.type())
  {
  // Could be either message
  case Message::Type_Post:
    forwardPostMessage(transaction, socket);
    break;
  // Client Message
  case Message::Type_Call:
    switch (function)
    {
    case Message::BoundObjectFunction_UnregisterEvent:
      unregisterEventListenerCall(transaction, socket);
      break;
    case Message::BoundObjectFunction_RegisterEvent:
    case Message::BoundObjectFunction_RegisterEventWithSignature:
      registerEventListenerCall(transaction, socket);
      break;
    default:
      handleCallMessage(transaction, socket);
    }
    break;
  // Service Message
  case Message::Type_Reply:
  case Message::Type_Error:
    if (function == Message::BoundObjectFunction_RegisterEvent)
      registerEventListenerReply(transaction, socket);
    else
      handleReplyMessage(transaction);
    break;
  case Message::Type_Event:
    handleEventMessage(transaction, socket);
    break;
  default:
    qiLogError() << "Unexpected message type: " << msg.type();
    break;
  }
}

void GatewayPrivate::onServiceDirectoryMessageReady(const Message& msg, TransportSocketPtr socket)
{
  // We have to do this check in case multiple services exist on the same socket as the SD.
  if (msg.service() != Message::Service_ServiceDirectory)
    return onAnyMessageReady(msg, socket);

  switch (msg.function())
  {
  // Insert the GW's information instead of the service's ones.
  case Message::ServiceDirectoryAction_Service:
  {
    Message forgedMessage(msg.type(), msg.address());
    std::string sig = typeOf<ServiceInfo>()->signature().toString();

    forgedMessage.setFlags(msg.flags());
    if (msg.type() == Message::Type_Error)
      forgedMessage.setBuffer(msg.buffer());
    else
    {
      ServiceInfo info = msg.value(sig, socket).to<ServiceInfo>();
      forgeServiceInfo(info);
      forgedMessage.setValue(AnyReference::from(info), sig);
    }
    return onAnyMessageReady(forgedMessage, socket);
  }
  // Insert the GW's information instead of the services's ones.
  case Message::ServiceDirectoryAction_Services:
  {
    std::string sig = typeOf<ServiceInfoVector>()->signature().toString();
    Message forgedMessage(msg.type(), msg.address());

    forgedMessage.setFlags(msg.flags());
    if (msg.type() == Message::Type_Error)
      forgedMessage.setBuffer(msg.buffer());
    else
    {
      ServiceInfoVector info = msg.value(sig, socket).to<ServiceInfoVector>();
      for (ServiceInfoVector::iterator it = info.begin(), end = info.end(); it != end; ++it)
        forgeServiceInfo(*it);
      forgedMessage.setValue(AnyReference::from(info), sig);
    }
    return onAnyMessageReady(forgedMessage, socket);
  }
  case Message::ServiceDirectoryAction_RegisterService:
    if (msg.type() != Message::Type_Error)
    {
      TransportSocketPtr origin;
      {
        boost::mutex::scoped_lock lock(_ongoingMsgMutex);
        origin = _ongoingMessages[ServiceSD][msg.id()].second;
      }
      int serviceId = msg.value("I", socket).to<unsigned int>();
      {
        boost::recursive_mutex::scoped_lock lock(_serviceMutex);
        _services[serviceId] = origin;
      }
    }
    break;
  }
  onAnyMessageReady(msg, socket);
}

void GatewayPrivate::forwardPostMessage(GwTransaction& t, TransportSocketPtr)
{
  ServiceId sid = t.content.service();
  TransportSocketPtr target = safeGetService(sid);
  t.setDestinationIfNull(target);
  // the service may have already disconnected
  if (t.destination())
    t.destination()->send(t.content);
}

void GatewayPrivate::registerEventListenerCall(GwTransaction& t, TransportSocketPtr origin)
{
  static const char* sig = "(IIL)";
  Message& msg = t.content;
  AnyReference values = msg.value(sig, origin);
  /*
   * Need to use the original object/service ID:
   * when the event is on a client object, we need to see
   * the translated endpoint (which are the original values of the
   * call, originating from a service).
   */
  EventId event = values[1].to<unsigned int>();
  ServiceId serviceId = t.originalService();
  ObjectId objectId = t.originalObject();
  t.setDestinationIfNull(safeGetService(serviceId));
  EventHostEndpoint eventHost = t.destination();
  SignalLink signalLink = values[2].to<SignalLink>();
  {
    boost::recursive_mutex::scoped_lock lock(_eventSubMutex);
    EventsEndpointMap::iterator endpointIt;
    EventServiceMap::iterator serviceIt;
    EventsPerObjectMap::iterator objectIt;
    ClientsPerEventMap::iterator eventIt;
    // Check if this client is the first one to subscribe.
    if ((endpointIt = _eventSubscribers.find(eventHost)) == _eventSubscribers.end() ||
        (serviceIt = endpointIt->second.find(serviceId)) == endpointIt->second.end() ||
        (objectIt = serviceIt->second.find(objectId)) == serviceIt->second.end() ||
        (eventIt = objectIt->second.find(event)) == objectIt->second.end())
    {
      // Deflect the messAge to handlecall:
      // if this cLient is the first to subscribe to this event,
      // we have to send a subscription message to the service
      // to make the connection.
      GWMessageId id = handleCallMessage(t, origin);
      _pendingEventSubscriptions[id] = boost::make_tuple(serviceId, objectId, event, signalLink, origin, eventHost);
      return;
    }
    else
      // If some clients were already subscribed, add this new client to the list.
      eventIt->second.remoteSubscribers[origin] = signalLink;
  }
  values.destroy();

  // Otherwise, handle the response ourself and
  // return their own signalLink
  Message rep;
  rep.setId(msg.id());
  rep.setType(Message::Type_Reply);
  rep.setService(serviceId);
  rep.setFunction(msg.function());
  rep.setObject(objectId);
  rep.setValue(AnyReference::from(signalLink), "L");
  origin->send(rep);
}

void GatewayPrivate::registerEventListenerReply(GwTransaction& t, TransportSocketPtr origin)
{
  namespace bt = boost::tuples;
  Message& msg = t.content;
  GWMessageId msgId = msg.id();
  SignalLink link = 0;
  {
    boost::recursive_mutex::scoped_lock lock(_eventSubMutex);
    std::map<GWMessageId, EventAddress>::iterator evIt = _pendingEventSubscriptions.find(msgId);

    if (evIt == _pendingEventSubscriptions.end() || msg.type() == Message::Type_Error)
    {
      if (evIt != _pendingEventSubscriptions.end())
        _pendingEventSubscriptions.erase(evIt);
      lock.unlock();
      return handleReplyMessage(t);
    }

    EventAddress& evt = evIt->second;
    ServiceId service = bt::get<0>(evt);
    uint32_t object = bt::get<1>(evt);
    uint32_t event = bt::get<2>(evt);
    link = bt::get<3>(evt);
    EventSubscriberEndpoint client = bt::get<4>(evt);
    EventHostEndpoint eventHost = bt::get<5>(evt);
    EventSubInfo& eventInfo = _eventSubscribers[eventHost][service][object][event];
    eventInfo.remoteSubscribers[client] = link;
    eventInfo.gwLink = link;
    _pendingEventSubscriptions.erase(evIt);
  }
  Message rep;
  rep.setId(msgId);
  rep.setType(msg.type());
  rep.setService(msg.service());
  rep.setFunction(msg.function());
  rep.setObject(msg.object());
  rep.setValue(AnyReference::from(link), "L");
  t.content = rep;
  handleReplyMessage(t);
}

void GatewayPrivate::unregisterEventListenerCall(GwTransaction& t, TransportSocketPtr origin)
{
  static const char* sig = "(IIL)";
  AnyReference values = t.content.value(sig, origin);
  ServiceId service = t.originalService();
  ObjectId object = t.originalObject();
  unsigned int event = values[1].to<unsigned int>();
  t.setDestinationIfNull(safeGetService(service));
  EventHostEndpoint eventHost = t.destination();

  values.destroy();
  {
    boost::recursive_mutex::scoped_lock lock(_eventSubMutex);
    EventsEndpointMap::iterator endpointIt;
    EventServiceMap::iterator serviceIt;
    EventsPerObjectMap::iterator objectIt;
    ClientsPerEventMap::iterator eventIt;

    if ((endpointIt = _eventSubscribers.find(eventHost)) == _eventSubscribers.end() ||
        (serviceIt = endpointIt->second.find(service)) == endpointIt->second.end() ||
        (objectIt = serviceIt->second.find(object)) == serviceIt->second.end() ||
        (eventIt = objectIt->second.find(event)) == objectIt->second.end())
      return;

    EventSubInfo& info = eventIt->second;

    // If this was the last subscriber, a message will be sent to
    // the service to sever the GW<->service event connection.
    removeEventSubscriber(service, object, event, origin, eventHost);
    if (info.remoteSubscribers.size() == 0)
    {
      objectIt->second.erase(eventIt);
      if (objectIt->second.size() == 0)
      {
        serviceIt->second.erase(objectIt);
        if (serviceIt->second.size() == 0)
        {
          endpointIt->second.erase(serviceIt);
          if (endpointIt->second.size() == 0)
            _eventSubscribers.erase(endpointIt);
        }
      }
    }
  }
  // Send a response to the client to confirm he won't receive those
  // events anymore.
  Message rep;
  rep.setId(t.content.id());
  rep.setType(Message::Type_Reply);
  rep.setService(service);
  rep.setFunction(t.content.function());
  rep.setObject(object);
  origin->send(rep);
}

void GatewayPrivate::invalidateClientsMessages(ServiceId sid)
{
  {
    namespace bt = boost::tuples;
    boost::mutex::scoped_lock lock(_pendingMsgMutex);
    std::vector<boost::tuple<ClientMessageId, Message, TransportSocketPtr> >::iterator it =
        _pendingMessages[sid].begin();
    std::vector<boost::tuple<ClientMessageId, Message, TransportSocketPtr> >::iterator end =
        _pendingMessages[sid].end();

    for (; it != end; ++it)
      serviceUnavailable(sid, bt::get<1>(*it), bt::get<2>(*it));
    _pendingMessages[sid].clear();
    _pendingMessages.erase(sid);
  }
  {
    boost::mutex::scoped_lock lock(_ongoingMsgMutex);
    Message forged;
    IdLookupMap::iterator it = _ongoingMessages[sid].begin();
    IdLookupMap::iterator end = _ongoingMessages[sid].end();

    for (; it != end; ++it)
    {
      forged.setId(it->second.first);
      serviceUnavailable(sid, forged, it->second.second);
    }
    _ongoingMessages[sid].clear();
    _ongoingMessages.erase(sid);
  }
}
}
