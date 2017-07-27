#pragma once
/*
** Copyright (C) 2015 Aldebaran Robotics
** See COPYING for the license
*/

#ifndef _SRC_MESSAGING_GATEWAY_P_HPP_
#define _SRC_MESSAGING_GATEWAY_P_HPP_

#include "server.hpp"
#include <qi/session.hpp>
#include <qi/messaging/clientauthenticatorfactory.hpp>
#include <qi/trackable.hpp>
#include <qi/type/metaobject.hpp>
#include <qi/url.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <unordered_map>

using ServiceId = unsigned int;

namespace qi
{

class GatewayPrivate
{
public:
  explicit GatewayPrivate(bool enforceAuth);
  ~GatewayPrivate();

  Property<bool> connected;

  Future<void> close();
  Future<UrlVector> endpoints() const;
  Future<bool> listen(const Url& url);
  Future<bool> setIdentity(const std::string& key, const std::string& crt);
  Future<void> connect(const Url& sdUrl);

private:
  void closeNow();
  bool mirrorService(const std::string& serviceName);
  Future<void> bindServicesToServiceDirectory(const Url& url);
  void resetConnectionToServiceDirectory(const Url& url, const std::string& reason);
  bool removeService(const std::string& service);
  Future<void> retryConnect(const qi::Url& sdUrl, qi::Duration lastTimer);
  std::unique_ptr<Session> recreateServer();

  std::unique_ptr<Session> _server; // ptr because we have to recreate it every time we listen
  std::unique_ptr<Session> _sdClient;
  Url _listenUrl;
  std::pair<std::string, std::string> _identity;
  std::unordered_map<std::string, unsigned int> _registeredServices;
  bool _isEnforcedAuth;

  mutable Strand _strand;
};
}

#endif
