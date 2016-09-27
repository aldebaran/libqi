#pragma once
/*
** Copyright (C) 2015 Aldebaran Robotics
** See COPYING for the license
*/

#ifndef _SRC_MESSAGING_GATEWAY_P_HPP_
#define _SRC_MESSAGING_GATEWAY_P_HPP_

#include <boost/thread/mutex.hpp>

#include <qi/messaging/gateway.hpp>
#include <qi/property.hpp>
#include <qi/periodictask.hpp>

#include <mutex>

#include "message.hpp"
#include "transportsocket.hpp"
#include "authprovider_p.hpp"
#include "transportsocketcache.hpp"
#include "gwsdclient.hpp"
#include "gwobjecthost.hpp"
#include "transportserver.hpp"

using ServiceId = unsigned int;
using ClientMessageId = unsigned int;
using GWMessageId = unsigned int;
using ObjectId = unsigned int;
using EventId = unsigned int;

namespace qi
{

struct ClientInfo
{
  ClientMessageId id;
  TransportSocketPtr socket;
};

struct MessageInfo
{
  ClientMessageId clientId;
  Message message;
  TransportSocketPtr target;
};

class GwTransaction
{
public:
  GwTransaction(const Message& msg)
    : content(msg)
    , _destination(TransportSocketPtr())
    , _originalObjectId(msg.object())
    , _originalServiceId(msg.service())
  {
  }
  GwTransaction(const GwTransaction& t)
    : content(t.content)
    , _destination(t._destination)
    , _originalObjectId(t._originalObjectId)
    , _originalServiceId(t._originalServiceId)
  {
  }
  Message content;

  void forceDestination(TransportSocketPtr dest);
  void setDestinationIfNull(TransportSocketPtr dest);
  TransportSocketPtr destination()
  {
    return _destination;
  }
  ServiceId originalService() const
  {
    return _originalServiceId;
  }
  ObjectId originalObject() const
  {
    return _originalObjectId;
  }

private:
  TransportSocketPtr _destination;
  ObjectId _originalObjectId;
  ServiceId _originalServiceId;
};

class GatewayPrivate : public qi::Trackable<GatewayPrivate>
{
public:
  GatewayPrivate(bool enforceAuth);
  ~GatewayPrivate();

  Property<bool> connected;

  void close(bool clearEndpoints = false);
  void setAuthProviderFactory(AuthProviderFactoryPtr provider);
  void setLocalClientAuthProviderFactory(AuthProviderFactoryPtr provider);
  void setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr authenticator);
  UrlVector endpoints() const;
  bool listen(const Url& url);
  bool setIdentity(const std::string& key, const std::string& crt);
  Future<void> connect(const Url& sdUrl);

private:
  TransportSocketPtr safeGetService(ServiceId id);

  void onServerAcceptError(int err);

  void onClientConnection(TransportSocketPtr socket);
  void onLocalClientConnection(TransportSocketPtr socket);
  void onSdServiceAdded(ServiceId id, const std::string& name);
  void onSdServiceRemoved(ServiceId id);
  void onSdConnected(Future<void> fut, Promise<void> prom);
  void onClientDisconnected(TransportSocketPtr socket, std::string url, const std::string& reason);
  void serviceDisconnected(ServiceId sid);
  void onServiceDirectoryDisconnected(TransportSocketPtr socket, const std::string& reason);

  void firstServiceMessage(const Message& capMessage,
                           TransportSocketPtr service,
                           ServiceId sid,
                           SignalSubscriberPtr sub);
  void firstClientMessage(const Message& capMessage, TransportSocketPtr socket, SignalSubscriberPtr sub);
  void clientAuthenticationMessages(const Message& capMessage,
                                    TransportSocketPtr socket,
                                    AuthProviderPtr auth,
                                    boost::shared_ptr<bool> firstMessage,
                                    SignalSubscriberPtr sub);
  void localClientSkipAuthMessage(const Message& capMsg, TransportSocketPtr socket, SignalSubscriberPtr sub);
  void startServiceAuthentication(TransportSocketPtr serviceSocket, ServiceId sid);
  void serviceAuthenticationMessages(const Message& msg,
                                     TransportSocketPtr service,
                                     ServiceId sid,
                                     ClientAuthenticatorPtr authenticator,
                                     SignalSubscriberPtr sub);

  void serviceUnavailable(ServiceId service, const Message& subject, TransportSocketPtr client);
  void forwardMessage(ClientMessageId origId,
                      const Message& forward,
                      TransportSocketPtr client,
                      TransportSocketPtr destination);
  void forwardPostMessage(GwTransaction& t, TransportSocketPtr origin);
  GWMessageId handleCallMessage(GwTransaction& msg, TransportSocketPtr origin, TransportSocketPtr destination = {});
  void handleReplyMessage(GwTransaction& msg);
  void handleEventMessage(GwTransaction& msg, TransportSocketPtr socket);

  void onAnyMessageReady(const Message& msg, TransportSocketPtr socket);
  void onServiceDirectoryMessageReady(const Message& msg, TransportSocketPtr socket);

  void registerEventListenerCall(GwTransaction& msg, TransportSocketPtr origin);
  void registerEventListenerReply(GwTransaction& msg, TransportSocketPtr origin);
  void unregisterEventListenerCall(GwTransaction& msg, TransportSocketPtr origin);
  Future<void> unregisterServiceFromSD(ServiceId sid);
  void invalidateClientsMessages(ServiceId sid);
  void forgeServiceInfo(ServiceInfo&);

  void sdConnectionRetry(const Url& sdUrl, qi::Duration lastTimer);
  void localServiceRegistration(Future<ServiceInfo> serviceInfo, ServiceId targetService);
  void localServiceRegistrationCont(Future<TransportSocketPtr> fut, ServiceId sid);
  void localServiceRegistrationEnd(TransportSocketPtr socket, ServiceId sid);

  void updateEndpoints(const Url& url);

  void reportProcessingFailure(const Message& processedMessage, TransportSocketPtr source, std::string messageText);

  bool _enforceAuth;

  TransportServer _server;

  // The local server will be used by processes on the robot
  // trying to access the outside world through the gateway.
  // Auth scheme may be different, so we use a second one.
  TransportServer _localServer;

  Url _listenUrl;
  std::mutex _endpointsMutex;
  UrlVector _endpoints;
  std::set<Url> _pendingListens;
  qi::PeriodicTask _updateEndpointsTask;
  TransportSocketCache _socketCache;

  std::vector<TransportSocketPtr> _clients;
  boost::recursive_mutex _clientsMutex;
  std::map<ServiceId, TransportSocketPtr> _services;
  std::map<ServiceId, std::string> _sdAvailableServices;
  boost::recursive_mutex _serviceMutex;
  GwSDClient _sdClient;
  GwObjectHost _objectHost;


  using IdLookupMap = std::map<GWMessageId, ClientInfo >;
  using OngoingMessagesMap = std::map<ServiceId, IdLookupMap>;
  // This represents the messages that are currently awaiting a response, with both endpoints being known
  // and connected to the gateway.
  OngoingMessagesMap _ongoingMessages;
  boost::mutex _ongoingMsgMutex;

  // Messages for services that are not registered to the GW yet.
  // Once they are, we'll forward them.
  using PendingMessagesMap =
      std::map<ServiceId, std::vector<MessageInfo>>;
  PendingMessagesMap _pendingMessages;
  boost::mutex _pendingMsgMutex;

  using EventSubscriberEndpoint = TransportSocketPtr;
  using EventHostEndpoint = TransportSocketPtr;
  struct EventAddress
  {
    ServiceId serviceId;
    ObjectId objectId;
    EventId eventId;
    SignalLink signalLink;
    EventSubscriberEndpoint subscriberSocket;
    EventHostEndpoint hostSocket;
  };
  std::map<GWMessageId, EventAddress> _pendingEventSubscriptions;
  struct EventSubInfo
  {
    SignalLink gwLink;
    std::map<EventSubscriberEndpoint, SignalLink> remoteSubscribers;
  };
  //                event n
  using ClientsPerEventMap = std::map<EventId, EventSubInfo>;
  //                object ID
  using EventsPerObjectMap = std::map<ObjectId, ClientsPerEventMap>;
  using EventServiceMap = std::map<ServiceId, EventsPerObjectMap>;
  using EventsEndpointMap = std::map<EventHostEndpoint, EventServiceMap>;
  EventsEndpointMap _eventSubscribers;
  boost::recursive_mutex _eventSubMutex;

  void removeEventSubscriber(ServiceId service,
                             uint32_t object,
                             uint32_t event,
                             EventSubscriberEndpoint client,
                             EventHostEndpoint host,
                             ClientMessageId = 0);

  AuthProviderFactoryPtr _authProviderFactory;
  AuthProviderFactoryPtr _localClientAuthProviderFactory;
  ClientAuthenticatorFactoryPtr _clientAuthenticatorFactory;
  qi::Future<void> _retryFut;
  Atomic<bool> _dying;
};
}

#endif
