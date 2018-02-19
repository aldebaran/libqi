/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/messaging/gateway.hpp>
#include "gateway_p.hpp"
#include "clientauthenticator_p.hpp"
#include <qi/log.hpp>
#include <qi/scoped.hpp>

qiLogCategory("qimessaging.gateway");

namespace qi
{

Gateway::Gateway(bool enforceAuth)
  : _p(new GatewayPrivate(enforceAuth))
  , connected(_p->connected)
{
}

Gateway::~Gateway() = default;

void Gateway::close()
{
  _p->close().value();
}

UrlVector Gateway::endpoints() const
{
  return _p->endpoints().value();
}

bool Gateway::listen(const Url& url)
{
  return _p->listen(url).value();
}

bool Gateway::setIdentity(const std::string& key, const std::string& crt)
{
  return _p->setIdentity(key, crt).value();
}

qi::Future<void> Gateway::attachToServiceDirectory(const Url& serviceDirectoryUrl)
{
  return _p->connect(serviceDirectoryUrl);
}

GatewayPrivate::GatewayPrivate(bool enforceAuth)
  : _isEnforcedAuth(enforceAuth)
{}

GatewayPrivate::~GatewayPrivate()
{
  try
  {
    closeNow();
    _strand.join();
  }
  catch (const std::exception& ex)
  {
    qiLogError() << "Exception caught during destruction of private class: " << ex.what();
  }
  catch (...)
  {
    qiLogError() << "Unknown exception caught during destruction of private class";
  }
}

void GatewayPrivate::closeNow()
{
  connected = false;
  // TODO: consider chain the following operations asynchronously and return a future
  _server.reset();
  _sdClient.reset();
  _registeredServices.clear();
}

Future<void> GatewayPrivate::close()
{
  return _strand.async([&]{ closeNow(); });
}

Future<UrlVector> GatewayPrivate::endpoints() const
{
  return _strand.async([&]{ return _server->endpoints(); });
}

Future<bool> GatewayPrivate::listen(const Url& url)
{
  return _strand.async([=] {
    _listenUrl = url;
    try
    {
      _server = recreateServer();
      return _server->listenStandalone(url).async().then([url](Future<void> listenFut) {
        if (listenFut.hasError())
        {
          qiLogError() << "Error while trying to listen at '" << url.str()
                       << "': " << listenFut.error();
          return false;
        }
        return true;
      });
    }
    catch (const std::exception& ex)
    {
      qiLogError() << "Exception caught while trying to listen at '" << url.str()
                   << "': " << ex.what();
      return Future<bool>{false};
    }
  }).unwrap();
}

Future<bool> GatewayPrivate::setIdentity(const std::string& key, const std::string& crt)
{
  return _strand.async([=] {
    auto res = _server->setIdentity(key, crt);
    _identity = { key, crt };
    return res;
  });
}

Future<void> GatewayPrivate::connect(const Url& sdUrl)
{
  if (!sdUrl.isValid()) return makeFutureError<void>("Invalid service directory URL");
  return _strand.async([=] {
    _sdClient.reset(new Session);
    return _sdClient->connect(sdUrl)
        .async()
        .andThen(_strand.unwrappedSchedulerFor([=](void*) {
          connected = true;
          return bindServicesToServiceDirectory(sdUrl);
        }))
        .unwrap()
        .then(_strand.schedulerFor([=](Future<void> connectFut) {
          // If any error occurred during connection, no need to keep the SD client alive
          if (connectFut.hasError())
            _sdClient.reset();
        })).unwrap();
  }).unwrap();
}

bool GatewayPrivate::mirrorService(const std::string& serviceName)
{
  qiLogInfo() << "Mirrorring service '" << serviceName << "'";
  if (serviceName == Session::serviceDirectoryServiceName())
  {
    qiLogVerbose() << "Service '" << serviceName << "' should not be mirrorred, skipping";
    return false;
  }

  boost::optional<unsigned int> serviceId;
  try
  {
    qiLogVerbose() << "Registering service '" << serviceName << "' to the gateway local server";
    serviceId =
        _server->registerService(serviceName, _sdClient->service(serviceName).value()).value();
    qiLogVerbose() << "Service '" << serviceName << "' registered to the gateway local server";
    _registeredServices[serviceName] = *serviceId;
    return true;
  }
  catch (const std::exception& e)
  {
    qiLogError() << "An error occurred while mirrorring service '" << serviceName
                 << "': " << e.what();
    if (serviceId)
    {
      qiLogVerbose() << "Unregistering '" << serviceName
                     << "' from the gateway local server (rollback because an error occurred)";
      _server->unregisterService(*serviceId).value();
      qiLogVerbose() << "Service '" << serviceName << "' unregistered from the gateway local server";
    }
    return false;
  }
}

Future<void> GatewayPrivate::bindServicesToServiceDirectory(const Url& url)
{
  _registeredServices.clear();

  qiLogInfo() << "Will now mirror " << url.str() << "'s services to "
              << (_listenUrl.isValid() ? _listenUrl.str() : "no url");

    // When one of the services is registered to the original service
    // directory, register it to this service directory.
  bool success = false;

  auto scopedDisconnectServiceRegistered =
      scoped(_sdClient->serviceRegistered
                 .connect(_strand.schedulerFor(
                     [=](unsigned int, const std::string& service) { mirrorService(service); })),
             [&](const SignalSubscriber& sigSub) {
               if (!success)
                 _sdClient->serviceRegistered.disconnect(sigSub.link());
             });

  // When one of the services we track is unregistered from the original service
  // directory, unregister it from the this service directory.
  auto scopedDisconnectServiceUnregistered =
      scoped(_sdClient->serviceUnregistered
                 .connect(_strand.schedulerFor(
                     [=](unsigned int, const std::string& service) { removeService(service); })),
             [&](const SignalSubscriber& sigSub) {
               if (!success)
                 _sdClient->serviceUnregistered.disconnect(sigSub.link());
             });

  // When disconnected, try to reconnect
  auto scopedDisconnectDisconnected =
      scoped(_sdClient->disconnected
                 .connect(_strand.schedulerFor([=](const std::string& reason) {
                   resetConnectionToServiceDirectory(url, reason);
                 })),
             [&](const SignalSubscriber& sigSub) {
               if (!success)
                 _sdClient->disconnected.disconnect(sigSub.link());
             });

  // Mirror all services already available
  const auto servicesFut = _sdClient->services().async().andThen(_strand.schedulerFor(
        [=](const std::vector<ServiceInfo>& services) {
          for (const auto& serviceInfo : services)
            mirrorService(serviceInfo.name());
        })).unwrap();

  success = true;
  return servicesFut;
}

void GatewayPrivate::resetConnectionToServiceDirectory(const Url& url, const std::string& reason)
{
  qiLogWarning() << "Lost connection to the ServiceDirectory: " << reason;

  closeNow();

  static const Duration retryTimer = Seconds{ 1 };
  asyncDelay(_strand.schedulerFor([=]{ retryConnect(url, retryTimer); }), retryTimer);
}

bool GatewayPrivate::removeService(const std::string& service)
{
  auto it = _registeredServices.find(service);
  if (it == _registeredServices.end())
    return false;
  const auto id = it->second;
  _registeredServices.erase(it);

  try
  {
    _server->unregisterService(id);
    qiLogInfo() << "Removed unregistered service " << service;
    return true;
  }
  catch (const std::exception& e)
  {
    qiLogInfo() << "Error while unregistering " << service << ": " << e.what();
    return false;
  }
}

Future<void> GatewayPrivate::retryConnect(const Url& sdUrl, Duration lastTimer)
{
  return connect(sdUrl).then(_strand.unwrappedSchedulerFor([=](const Future<void>& connectFuture)
    {
      if (connectFuture.hasError())
      {
        auto newTimer = lastTimer * 2;
        qiLogWarning() << "Can't reach ServiceDirectory at address " << sdUrl.str()
                       << ", retrying in "
                       << boost::chrono::duration_cast<Seconds>(lastTimer).count() << "sec.";
        return asyncDelay(_strand.schedulerFor([=]{ retryConnect(sdUrl, newTimer); }), newTimer);
      }
      else
      {
        qiLogInfo() << "Successfully reestablished connection to the ServiceDirectory at address "
                    << sdUrl.str();
        if (!_listenUrl.isValid())
          return Future<void>{ nullptr };

        qiLogInfo() << "Trying to listen to " << _listenUrl.str();
        const auto listenUrl = _listenUrl;
        return listen(_listenUrl).then([listenUrl](Future<bool> futSuccess){
          if (!futSuccess.hasError() && futSuccess.value())
          {
            qiLogInfo() << "Listening to " << listenUrl.str();
            return Future<void>{ nullptr };
          }
          else
          {
            std::ostringstream oss;
            oss << "Listening to " << listenUrl.str() << " failed";
            const auto msg = oss.str();
            qiLogInfo() << msg;
            return makeFutureError<void>(msg);
          }
        }).unwrap();
      }
  })).unwrap();
}

std::unique_ptr<Session> GatewayPrivate::recreateServer()
{
  std::unique_ptr<Session> server{ new Session{ _isEnforcedAuth } };

  if (!_identity.first.empty() && !_identity.second.empty())
    server->setIdentity(_identity.first, _identity.second);

  for (const auto& serviceNameId : _registeredServices)
  {
    const auto serviceName = serviceNameId.first;
    server->registerService(serviceName, _sdClient->service(serviceName).value());
  }

  return server;
}

}
