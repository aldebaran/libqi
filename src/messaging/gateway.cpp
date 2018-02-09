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

namespace
{

void logAnyMirroringFailure(const MirroringResults& results)
{
  std::ostringstream errorMsg;
  bool atLeastOneFailed = false;
  for (const auto& result : results)
  {
    if (result.status == MirroringResult::Status::Failed_Error)
    {
      if (atLeastOneFailed)
        errorMsg << ", ";
      errorMsg << result.serviceName;
      atLeastOneFailed = true;
    }
  }
  if (atLeastOneFailed)
    qiLogWarning() << "Failed to mirror the following services: " << errorMsg.str();
}

}

Gateway::Gateway(bool enforceAuth)
  : _p(new Gateway::Impl(enforceAuth))
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

Future<Gateway::IdValidationStatus> Gateway::setValidateIdentity(const std::string& key,
                                                                 const std::string& crt)
{
  return _p->setValidateIdentity(key, crt);
}


qi::Future<void> Gateway::attachToServiceDirectory(const Url& serviceDirectoryUrl)
{
  return _p->connect(serviceDirectoryUrl).andThen(&logAnyMirroringFailure);
}

Gateway::Impl::Impl(bool enforceAuth)
  : _isEnforcedAuth(enforceAuth)
{}

Gateway::Impl::~Impl()
{
  try
  {
    _strand.join();
    closeUnsync();
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

void Gateway::Impl::closeUnsync()
{
  connected = false;
  // TODO: consider chain the following operations asynchronously and return a future
  _server.reset();
  _sdClient.reset();
}

Future<void> Gateway::Impl::close()
{
  return _strand.async([&]{
    closeUnsync();
  });
}

Future<UrlVector> Gateway::Impl::endpoints() const
{
  return _strand.async([&]{
    return _server ? _server->endpoints() : UrlVector{};
  });
}

Future<bool> Gateway::Impl::listen(const Url& url)
{
  return _strand.async([=] {
    _listenUrl = url;
    try
    {
      _server = createServerUnsync();
      return mirrorAllServices()
          .andThen(&logAnyMirroringFailure)
          .andThen(_strand.unwrappedSchedulerFor([=](void*) {
            if (!_server)
              throw std::runtime_error{ "No server available to listen" };
            return _server->listenStandalone(url).async().then([url](Future<void> listenFut) {
              if (listenFut.hasError())
              {
                qiLogError() << "Error while trying to listen at '" << url.str()
                             << "': " << listenFut.error();
                return false;
              }
              return true;
            });
          })).unwrap();
    }
    catch (const std::exception& ex)
    {
      qiLogError() << "Exception caught while trying to listen at '" << url.str()
                   << "': " << ex.what();
      throw;
    }
  }).unwrap();
}

Future<bool> Gateway::Impl::setIdentity(const std::string& key, const std::string& crt)
{
  return setValidateIdentity(key, crt).andThen([](IdValidationStatus status) {
    return status == IdValidationStatus::Valid;
  });
}

Future<Gateway::IdValidationStatus> Gateway::Impl::setValidateIdentity(const std::string& key,
                                                                       const std::string& crt)
{
  return _strand.async([=] {
    if (key.empty() || crt.empty())
    {
      _identity.reset();
      return IdValidationStatus::Invalid;
    }
    _identity = Identity{ key, crt };
    if (!_server)
      return IdValidationStatus::PendingCheckOnListen;
    return _server->setIdentity(key, crt) ? IdValidationStatus::Valid : IdValidationStatus::Invalid;
  });
}

Future<MirroringResults> Gateway::Impl::connect(const Url& sdUrl)
{
  if (!sdUrl.isValid()) return makeFutureError<MirroringResults>("Invalid service directory URL");
  return _strand.async([=] {
    _sdClient.reset(new Session);
    return _sdClient->connect(sdUrl)
        .async()
        .andThen(_strand.unwrappedSchedulerFor([=](void*) {
          connected = true;
          return bindToServiceDirectoryUnsync(sdUrl);
        }))
        .unwrap()
        .then(_strand.unwrappedSchedulerFor([=](Future<MirroringResults> connectFut) {
          // If any error occurred during connection, no need to keep the SD client alive
          if (connectFut.hasError())
            _sdClient.reset();
          return connectFut;
        })).unwrap();
  }).unwrap();
}

Future<MirroringResults> Gateway::Impl::mirrorAllServices()
{
  return _strand.async([=] {
    if (!_sdClient)
      return makeFutureError<MirroringResults>("Not connected to service directory");
    qiLogVerbose() << "Mirroring services: requesting list of services from ServiceDirectory";
    return _sdClient->services().async().andThen(_strand.schedulerFor(
        [=](const std::vector<ServiceInfo>& services) {
          qiLogVerbose() << "Mirroring services: received list of services from the ServiceDirectory";
          MirroringResults results;
          for (const auto& serviceInfo : services)
            results.push_back({ serviceInfo.name(), mirrorServiceUnsync(serviceInfo.name()) });
          return results;
        })).unwrap();
  }).unwrap();
}

MirroringResult::Status Gateway::Impl::mirrorServiceUnsync(const std::string& serviceName)
{
  qiLogInfo() << "Mirroring service '" << serviceName << "'";
  if (serviceName == Session::serviceDirectoryServiceName())
  {
    qiLogVerbose() << "Service '" << serviceName << "' should not be mirrored, skipping";
    return MirroringResult::Status::Skipped;
  }

  if (!_server)
    return MirroringResult::Status::Failed_NotListening;
  if (!_sdClient)
    return MirroringResult::Status::Failed_NoSdConnection;

  boost::optional<unsigned int> serviceId;
  try
  {
    qiLogVerbose() << "Registering service '" << serviceName << "' to the gateway local server";
    serviceId =
        _server->registerService(serviceName, _sdClient->service(serviceName).value()).value();
    qiLogVerbose() << "Service '" << serviceName << "' registered to the gateway local server";
  }
  catch (const std::exception& e)
  {
    qiLogError() << "An error occurred while mirroring service '" << serviceName
                 << "': " << e.what();
    if (serviceId)
    {
      qiLogVerbose() << "Unregistering '" << serviceName
                     << "' from the gateway local server (rollback because an error occurred)";
      _server->unregisterService(*serviceId).value();
      qiLogVerbose() << "Service '" << serviceName << "' unregistered from the gateway local server";
    }
    return MirroringResult::Status::Failed_Error;
  }
  return MirroringResult::Status::Done;
}

Future<MirroringResults> Gateway::Impl::bindToServiceDirectoryUnsync(const Url& url)
{
  if (!_sdClient)
    return makeFutureError<MirroringResults>("Not connected to service directory");

  qiLogInfo() << "Binding to service directory at url '" << url.str() << "'";

  bool bindingSucceeded = false;

  // When one of the services is registered to the original service
  // directory, register it to this service directory.
  auto scopedDisconnectServiceRegistered =
      scoped(_sdClient->serviceRegistered.connect(_strand.schedulerFor(
                 [=](unsigned int, const std::string& service) { mirrorServiceUnsync(service); })),
             [&](const SignalSubscriber& sigSub) {
               if (!bindingSucceeded)
                 _sdClient->serviceRegistered.disconnect(sigSub.link());
             });

  // When one of the services we track is unregistered from the original service
  // directory, unregister it from the this service directory.
  auto scopedDisconnectServiceUnregistered =
      scoped(_sdClient->serviceUnregistered.connect(
                 _strand.schedulerFor([=](unsigned int id, const std::string& service) {
                   removeServiceUnsync(id, service);
                 })),
             [&](const SignalSubscriber& sigSub) {
               if (!bindingSucceeded)
                 _sdClient->serviceUnregistered.disconnect(sigSub.link());
             });

  // When disconnected, try to reconnect
  auto scopedDisconnectDisconnected =
      scoped(_sdClient->disconnected
                 .connect(_strand.schedulerFor([=](const std::string& reason) {
                   resetConnectionToServiceDirectoryUnsync(url, reason);
                 })),
             [&](const SignalSubscriber& sigSub) {
               if (!bindingSucceeded)
                 _sdClient->disconnected.disconnect(sigSub.link());
             });

  // Mirror all services already available
  const auto servicesFut = mirrorAllServices();

  bindingSucceeded = true;
  return servicesFut;
}

void Gateway::Impl::resetConnectionToServiceDirectoryUnsync(const Url& url, const std::string& reason)
{
  qiLogWarning() << "Lost connection to the ServiceDirectory: " << reason;

  closeUnsync();

  static const Duration retryTimer = Seconds{ 1 };
  asyncDelay(_strand.schedulerFor([=]{ retryConnect(url, retryTimer); }), retryTimer);
}

void Gateway::Impl::removeServiceUnsync(unsigned int id, const std::string& service)
{
  if (!_server)
    return;

  try
  {
    _server->unregisterService(id);
    qiLogInfo() << "Removed unregistered service " << service;
  }
  catch (const std::exception& e)
  {
    qiLogInfo() << "Error while unregistering " << service << ": " << e.what();
    throw;
  }
}

Future<void> Gateway::Impl::retryConnect(const Url& sdUrl, Duration lastTimer)
{
  return connect(sdUrl)
      .then(_strand.unwrappedSchedulerFor([=](const Future<MirroringResults>& connectFuture) {
        if (connectFuture.hasError())
        {
          auto newTimer = lastTimer * 2;
          qiLogWarning() << "Can't reach ServiceDirectory at address " << sdUrl.str()
                         << ", retrying in "
                         << boost::chrono::duration_cast<Seconds>(lastTimer).count() << "sec.";
          return asyncDelay(_strand.schedulerFor([=] { retryConnect(sdUrl, newTimer); }), newTimer);
        }
        else
        {
          qiLogInfo() << "Successfully reestablished connection to the ServiceDirectory at address "
                      << sdUrl.str();
          logAnyMirroringFailure(connectFuture.value());

          if (!_listenUrl.isValid())
            return Future<void>{ nullptr };

          const auto listenUrl = _listenUrl;
          qiLogInfo() << "Trying to listen to " << listenUrl.str();
          return listen(_listenUrl)
              .then([listenUrl](Future<bool> futSuccess) {
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
              })
              .unwrap();
        }
      }))
      .unwrap();
}

std::unique_ptr<Session> Gateway::Impl::createServerUnsync()
{
  std::unique_ptr<Session> server{ new Session{ _isEnforcedAuth } };

  if (_identity && !server->setIdentity(_identity->key, _identity->crt))
  {
    throw std::runtime_error{ "Invalid identity parameters : key: '" + _identity->key +
                              "', crt: '" + _identity->crt + "'" };
  }
  return server;
}
}
