/*
**  Copyright (C) 2018 SoftBank Robotics Europe
**  See COPYING for the license
*/

#include <qi/messaging/gateway.hpp>
#include <qi/macro.hpp>

namespace qi
{

QI_WARNING_PUSH()
QI_WARNING_DISABLE(4996, deprecated-declarations) // ignore connected deprecation warnings
Gateway::Gateway(bool enforceAuth)
  : _proxy{ enforceAuth }
  , connected(_proxy.connected)
  , status(_proxy.status)
{
}
QI_WARNING_POP()

Gateway::~Gateway() = default;

UrlVector Gateway::endpoints() const
{
  return _proxy.endpoints();
}

bool Gateway::listen(const Url& url)
{
  auto fut = listenAsync(url);
  return fut.wait() == FutureState_FinishedWithValue && fut.value() == ListenStatus::Listening;
}

Future<Gateway::ListenStatus> Gateway::listenAsync(const Url& url)
{
  return _proxy.listenAsync(url);
}

bool Gateway::setIdentity(const std::string& key, const std::string& crt)
{
  auto fut = setValidateIdentity(key, crt);
  return fut.wait() == FutureState_FinishedWithValue && fut.value() == IdValidationStatus::Done;
}

Future<Gateway::IdValidationStatus> Gateway::setValidateIdentity(const std::string& key,
                                                                 const std::string& crt)
{
  return _proxy.setValidateIdentity(key, crt);
}

void Gateway::setAuthProviderFactory(AuthProviderFactoryPtr provider)
{
  _proxy.setAuthProviderFactory(provider);
}

qi::Future<void> Gateway::attachToServiceDirectory(const Url& serviceDirectoryUrl)
{
  return _proxy.attachToServiceDirectory(serviceDirectoryUrl);
}

void Gateway::close()
{
  _proxy.close();
}

}
