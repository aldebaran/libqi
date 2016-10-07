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
#include "messagesocket.hpp"
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
  MessageSocketPtr socket;
};

struct MessageInfo
{
  ClientMessageId clientId;
  Message message;
  MessageSocketPtr target;
};

class GwTransaction
{
public:
  GwTransaction(const Message& msg)
    : content(msg)
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

  void forceDestination(MessageSocketPtr dest);
  void setDestinationIfNull(MessageSocketPtr dest);
  MessageSocketPtr destination()
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
  MessageSocketPtr _destination;
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
  MessageSocketPtr safeGetService(ServiceId id);

  void onServerAcceptError(int err);

  void onClientConnection(const std::pair<MessageSocketPtr, Url>& socketUrl);
  void onLocalClientConnection(const std::pair<MessageSocketPtr, Url>& socketUrl);
  void onSdServiceAdded(ServiceId id, const std::string& name);
  void onSdServiceRemoved(ServiceId id);
  void onSdConnected(Future<void> fut, Promise<void> prom);
  void onClientDisconnected(MessageSocketPtr socket, std::string url, const std::string& reason);
  void serviceDisconnected(ServiceId sid);
  void onServiceDirectoryDisconnected(MessageSocketPtr socket, const std::string& reason);

  void firstServiceMessage(const Message& capMessage,
                           MessageSocketPtr service,
                           ServiceId sid,
                           SignalSubscriberPtr sub);
  void firstClientMessage(const Message& capMessage, MessageSocketPtr socket, SignalSubscriberPtr sub);
  void clientAuthenticationMessages(const Message& capMessage,
                                    MessageSocketPtr socket,
                                    AuthProviderPtr auth,
                                    boost::shared_ptr<bool> firstMessage,
                                    SignalSubscriberPtr sub);
  void localClientSkipAuthMessage(const Message& capMsg, MessageSocketPtr socket, SignalSubscriberPtr sub);
  void startServiceAuthentication(MessageSocketPtr serviceSocket, ServiceId sid);
  void serviceAuthenticationMessages(const Message& msg,
                                     MessageSocketPtr service,
                                     ServiceId sid,
                                     ClientAuthenticatorPtr authenticator,
                                     SignalSubscriberPtr sub);

  void serviceUnavailable(ServiceId service, const Message& subject, MessageSocketPtr client);
  void forwardMessage(ClientMessageId origId,
                      const Message& forward,
                      MessageSocketPtr client,
                      MessageSocketPtr destination);
  void forwardPostMessage(GwTransaction& t, MessageSocketPtr origin);
  GWMessageId handleCallMessage(GwTransaction& msg, MessageSocketPtr origin, MessageSocketPtr destination = {});
  void handleReplyMessage(GwTransaction& msg);
  void handleEventMessage(GwTransaction& msg, MessageSocketPtr socket);

  void onAnyMessageReady(const Message& msg, MessageSocketPtr socket);
  void onServiceDirectoryMessageReady(const Message& msg, MessageSocketPtr socket);

  void registerEventListenerCall(GwTransaction& msg, MessageSocketPtr origin);
  void registerEventListenerReply(GwTransaction& msg, MessageSocketPtr origin);
  void unregisterEventListenerCall(GwTransaction& msg, MessageSocketPtr origin);
  Future<void> unregisterServiceFromSD(ServiceId sid);
  void invalidateClientsMessages(ServiceId sid);
  void forgeServiceInfo(ServiceInfo&);

  void sdConnectionRetry(const Url& sdUrl, qi::Duration lastTimer);
  void localServiceRegistration(Future<ServiceInfo> serviceInfo, ServiceId targetService);
  void localServiceRegistrationCont(Future<MessageSocketPtr> fut, ServiceId sid);
  void localServiceRegistrationEnd(MessageSocketPtr socket, ServiceId sid);

  void updateEndpoints(const Url& url);

  void reportProcessingFailure(const Message& processedMessage, MessageSocketPtr source, std::string messageText);

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

  std::vector<MessageSocketPtr> _clients;
  boost::recursive_mutex _clientsMutex;
  std::map<ServiceId, MessageSocketPtr> _services;
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

  using EventSubscriberEndpoint = MessageSocketPtr;
  using EventHostEndpoint = MessageSocketPtr;
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

  std::vector<std::function<void()>> _signalDisconnections;
  boost::mutex _signalDisconnectionsMutex;


  void disconnectSDSignalsCallback();
};
}

#endif
