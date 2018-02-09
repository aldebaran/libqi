#pragma once
/*
** Copyright (C) 2015 Aldebaran Robotics
** See COPYING for the license
*/

#ifndef _SRC_MESSAGING_GATEWAY_P_HPP_
#define _SRC_MESSAGING_GATEWAY_P_HPP_

#include "server.hpp"
#include <qi/session.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/messaging/clientauthenticatorfactory.hpp>
#include <qi/trackable.hpp>
#include <qi/type/metaobject.hpp>
#include <qi/url.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <boost/optional.hpp>
#include <unordered_map>

using ServiceId = unsigned int;

namespace qi
{

struct MirroringResult
{
  enum class Status
  {
    Done,
    Failed_Error,
    Failed_NotListening,
    Failed_NoSdConnection,
    Skipped,
  };
  std::string serviceName;
  Status status;
};
using MirroringResults = std::vector<MirroringResult>;

class Gateway::Impl
{
public:
  explicit Impl(bool enforceAuth);
  ~Impl();

  Property<bool> connected;

  Future<void> close();
  Future<UrlVector> endpoints() const;
  Future<bool> listen(const Url& url);
  Future<bool> setIdentity(const std::string& key, const std::string& crt);
  Future<IdValidationStatus> setValidateIdentity(const std::string& key, const std::string& crt);
  Future<MirroringResults> connect(const Url& sdUrl);

private:

  void closeUnsync();
  Future<MirroringResults> mirrorAllServices();
  MirroringResult::Status mirrorServiceUnsync(const std::string& serviceName);
  Future<MirroringResults> bindToServiceDirectoryUnsync(const Url& url);
  void resetConnectionToServiceDirectoryUnsync(const Url& url, const std::string& reason);
  void removeServiceUnsync(unsigned int id, const std::string& service);
  Future<void> retryConnect(const qi::Url& sdUrl, qi::Duration lastTimer);
  std::unique_ptr<Session> createServerUnsync();

  struct Identity
  {
    std::string key;
    std::string crt;
  };

  std::unique_ptr<Session> _server; // ptr because we have to recreate it every time we listen
  std::unique_ptr<Session> _sdClient;
  Url _listenUrl;
  boost::optional<Identity> _identity;
  bool _isEnforcedAuth;

  mutable Strand _strand;
};
}

#endif
