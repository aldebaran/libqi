/*
**  Copyright (C) 2018 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QIMESSAGING_GATEWAY_HPP
#define QIMESSAGING_GATEWAY_HPP

#pragma once

#include <qi/messaging/servicedirectoryproxy.hpp>

namespace qi
{

class QI_API Gateway
{
private:
  ServiceDirectoryProxy _proxy; // must be declared before any other member

public:
  using IdValidationStatus = ServiceDirectoryProxy::IdValidationStatus;
  using ListeningStatus = ServiceDirectoryProxy::ListeningStatus;

  /**
   * @param enforceAuth If set to true, reject clients that try to skip the authentication step. If
   * false, accept all incoming connections whether or not they authentify.
   */
  Gateway(bool enforceAuth = true);

  ~Gateway();

  Property<bool>& connected;

  UrlVector endpoints() const;

  QI_API_DEPRECATED_MSG("Use listenAsync() instead.")
  bool listen(const Url& url);

  Future<ListeningStatus> listenAsync(const Url& url);

  QI_API_DEPRECATED_MSG("Use setValidateIdentity() instead.")
  bool setIdentity(const std::string& key, const std::string& crt);

  Future<IdValidationStatus> setValidateIdentity(const std::string& key, const std::string& crt);

  void setAuthProviderFactory(AuthProviderFactoryPtr provider);

  qi::Future<void> attachToServiceDirectory(const Url& serviceDirectoryUrl);

  void close();
};
}

#endif // QIMESSAGING_GATEWAY_HPP
