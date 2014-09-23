#pragma once
/*
** Copyright (C) 2014 Aldebaran Robotics
** See COPYING for the license
*/

#ifndef _SRC_MESSAGING_GWSDCLIENT_HPP_
#define _SRC_MESSAGING_GWSDCLIENT_HPP_

#include <map>

#include <boost/shared_ptr.hpp>

#include <qi/future.hpp>
#include <qi/url.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include <qi/signal.hpp>

#include "clientauthenticator_p.hpp"

namespace qi
{
class Message;
class TransportSocket;
typedef boost::shared_ptr<TransportSocket> TransportSocketPtr;

class GwSDClient
{
public:
  GwSDClient();
  ~GwSDClient();

  FutureSync<void> connect(const qi::Url& sdUrl);
  void close();
  Url url() const;
  void setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr);
  bool isConnected() const;

  TransportSocketPtr socket();
  const MetaObject& metaObject();

public:
  Future<ServiceInfoVector> services();
  Future<ServiceInfo> service(const std::string& name);
  Future<unsigned int> registerService(const ServiceInfo& svInfo);
  Future<void> unregisterService(unsigned int idx);
  Future<std::string> machineId();

  Signal<> connected;
  Signal<std::string> disconnected;
  Signal<unsigned int, std::string> serviceAdded;
  Signal<unsigned int, std::string> serviceRemoved;

private:
  TransportSocketPtr _sdSocket;
  ClientAuthenticatorFactoryPtr _authFactory;
  MetaObject _metaObject;
  SignalLink _messageReadyLink;

  Future<MetaObject> fetchMetaObject();
  Future<SignalLink> connectEvent(const std::string& eventName, SignalLink link);
  Future<void> unregisterEvent();

  void onSocketConnected(qi::FutureSync<void> future, qi::Promise<void> promise);
  void onAuthentication(const Message& msg,
                        Promise<void> prom,
                        ClientAuthenticatorPtr authenticator,
                        SignalSubscriberPtr old);
  void onMetaObjectFetched(Future<MetaObject> fut, Promise<void> prom);
  void onEventConnected(Future<SignalLink> fut,
                        Promise<void> prom,
                        boost::shared_ptr<boost::mutex> mutex,
                        boost::shared_ptr<int> initCount);
  void onMessageReady(const Message& msg);

  typedef void (*SetterFunc)(void*, const Message&, TransportSocketPtr);
  typedef std::map<unsigned int, std::pair<void*, SetterFunc> > PromiseMap;
  PromiseMap _promises;
  boost::mutex _promutex;
};
}

#endif
