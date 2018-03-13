/*
**  Copyright (C) 2018 Aldebaran Robotics
**  See COPYING for the license
*/

#include "clientauthenticator_p.hpp"
#include "server.hpp"
#include <qi/errorhandling.hpp>
#include <qi/log.hpp>
#include <qi/messaging/clientauthenticatorfactory.hpp>
#include <qi/messaging/servicedirectoryproxy.hpp>
#include <qi/scoped.hpp>
#include <qi/url.hpp>

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/algorithm/string/join.hpp>

qiLogCategory("qimessaging.servicedirectoryproxy");

using ServiceId = unsigned int;

namespace qi
{

namespace
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

const auto delayIncreaseExponent = 1;
const auto delayIncreaseFactor = 1 << delayIncreaseExponent;
const auto maxRetryCount = 7;

void logAnyMirroringFailure(const MirroringResults& results)
{
  const auto failedServiceNames =
      results | boost::adaptors::filtered([](const MirroringResult& result) {
        return result.status == MirroringResult::Status::Failed_Error;
      }) | boost::adaptors::transformed([](const MirroringResult& result) -> const std::string& {
        return result.serviceName;
      });

  if (!boost::empty(failedServiceNames))
    qiLogWarning() << "Failed to mirror the following services: "
                   << boost::algorithm::join(failedServiceNames, ", ");
}

/// Procedure<void(std::ostream&)> Proc
template<typename T, typename Proc>
Future<T> logThenReturnFutureError(LogLevel level, Proc&& outputMsg)
{
  std::ostringstream oss;
  outputMsg(oss);
  const auto message = oss.str();
  switch(level)
  {
    case LogLevel_Silent:                             break;
    case LogLevel_Fatal:   qiLogFatal()   << message; break;
    case LogLevel_Error:   qiLogError()   << message; break;
    case LogLevel_Warning: qiLogWarning() << message; break;
    case LogLevel_Info:    qiLogInfo()    << message; break;
    case LogLevel_Verbose: qiLogVerbose() << message; break;
    case LogLevel_Debug:   qiLogDebug()   << message; break;
  }
  return makeFutureError<T>(message);
}

AuthProviderFactoryPtr placeholderIfNull(AuthProviderFactoryPtr factory)
{
  if (!factory)
    return boost::make_shared<NullAuthProviderFactory>();
  return factory;
}

}

class ServiceDirectoryProxy::Impl
{
public:
  static const Seconds initRetryDelay;
  static Seconds maxRetryDelay();

  explicit Impl(bool enforceAuth);
  ~Impl();

  Property<bool> connected;
  Property<bool> listening;

  Future<void> close();
  Future<UrlVector> endpoints() const;
  Future<ListeningStatus> listenAsync(const Url& url);
  Future<IdValidationStatus> setValidateIdentity(const std::string& key, const std::string& crt);
  Future<void> setAuthProviderFactory(AuthProviderFactoryPtr provider);
  Future<void> attachToServiceDirectory(const Url& sdUrl);
  Future<ServiceFilter> setServiceFilter(ServiceFilter filter);

private:
  void closeUnsync();
  Future<void> mirrorAllServices();
  MirroringResult::Status mirrorServiceUnsync(const std::string& serviceName);
  void bindToServiceDirectoryUnsync(const Url& url);
  void removeServiceUnsync(const std::string& service);
  Future<void> retryAttach(const qi::Url& sdUrl, Seconds lastDelay);
  std::unique_ptr<Session> createServerUnsync();
  void handleSdDisconnectionUnsync(const Url& url, const std::string& reason);
  Future<void> delayRetryAttach(const Url& url, Seconds delay = initRetryDelay);
  bool shouldMirrorServiceUnsync(boost::string_ref serviceName) const;

  struct Identity
  {
    std::string key;
    std::string crt;
  };

  std::unique_ptr<Session> _server; // ptr because we have to recreate it every time we listen
  std::map<std::string, unsigned int> _serviceIndexes;
  std::unique_ptr<Session> _sdClient;
  Url _listenUrl;
  boost::optional<Identity> _identity;
  AuthProviderFactoryPtr _authProviderFactory;
  bool _isEnforcedAuth;
  ServiceFilter _serviceFilter;

  mutable Strand _strand;
};

const Seconds ServiceDirectoryProxy::Impl::initRetryDelay { 1 };

ServiceDirectoryProxy::ServiceDirectoryProxy(bool enforceAuth)
  : _p(new Impl(enforceAuth))
  , connected(_p->connected)
{
}

ServiceDirectoryProxy::~ServiceDirectoryProxy() = default;

void ServiceDirectoryProxy::close()
{
  _p->close().value();
}

Future<ServiceDirectoryProxy::ServiceFilter> ServiceDirectoryProxy::setServiceFilter(
    ServiceFilter filter)
{
  return _p->setServiceFilter(std::move(filter));
}

UrlVector ServiceDirectoryProxy::endpoints() const
{
  return _p->endpoints().value();
}

Future<ServiceDirectoryProxy::ListeningStatus> ServiceDirectoryProxy::listenAsync(const Url& url)
{
  return _p->listenAsync(url);
}

Future<ServiceDirectoryProxy::IdValidationStatus> ServiceDirectoryProxy::setValidateIdentity(
    const std::string& key,
    const std::string& crt)
{
  return _p->setValidateIdentity(key, crt);
}

void ServiceDirectoryProxy::setAuthProviderFactory(AuthProviderFactoryPtr provider)
{
  _p->setAuthProviderFactory(std::move(provider)).value();
}

qi::Future<void> ServiceDirectoryProxy::attachToServiceDirectory(const Url& serviceDirectoryUrl)
{
  return _p->attachToServiceDirectory(serviceDirectoryUrl);
}

ServiceDirectoryProxy::Impl::Impl(bool enforceAuth)
  : connected{ false, Property<bool>::Getter{}, Property<bool>::Setter{} }
  , listening{ false, Property<bool>::Getter{}, Property<bool>::Setter{} }
  ,_isEnforcedAuth(enforceAuth)
  , _serviceFilter{ PolymorphicConstantFunction<bool>{ false } }
{
  const auto mirrorAllIfBoth = [=](bool connected, bool listening){
    if (connected && listening)
      mirrorAllServices();
  };

  connected.connect(_strand.schedulerFor([=](bool connected) {
    mirrorAllIfBoth(connected, listening.get().value());

    // Because we close the server if we get disconnected from the SD, automatically restart
    // the server if _listenUrl is valid when we reconnect
    if (connected && _listenUrl.isValid())
      listenAsync(_listenUrl);
  }));
  listening.connect(_strand.schedulerFor([=](bool listening){
    mirrorAllIfBoth(connected.get().value(), listening);
  }));
}

ServiceDirectoryProxy::Impl::~Impl()
{
  static const auto errorMessagePrefix =
      "Exception caught during destruction of implementation class: ";
  try
  {
    _strand.join();
    closeUnsync();
  }
  catch (const std::exception& ex)
  {
    qiLogError() << errorMessagePrefix << "standard exception '" << ex.what() << "'.";
  }
  catch (const boost::exception& ex)
  {
    qiLogError() << errorMessagePrefix << "boost exception '" << boost::diagnostic_information(ex)
                 << "'.";
  }
  catch (...)
  {
    qiLogError() << errorMessagePrefix << "unknown exception.";
  }
}

Seconds ServiceDirectoryProxy::Impl::maxRetryDelay()
{
  static const Seconds val = boost::chrono::duration_cast<Seconds>(
      initRetryDelay * std::ldexp(initRetryDelay.count(), delayIncreaseExponent * maxRetryCount));
  return val;
}

void ServiceDirectoryProxy::Impl::closeUnsync()
{
  static const auto errorMessagePrefix = "Error while closing the service directory proxy: ";
  try
  {
    auto sdClient = std::move_if_noexcept(_sdClient); // will be destroyed at end of scope
    auto server = std::move_if_noexcept(_server);     // idem
    connected = false;
    listening = false;
    _serviceIndexes.clear();
  }
  catch (const std::exception& ex)
  {
    qiLogError() << errorMessagePrefix << "standard exception '" << ex.what()
                 << "'. Rethrowing the exception.";
    throw;
  }
  catch (const boost::exception& ex)
  {
    qiLogError() << errorMessagePrefix << "boost exception '" << boost::diagnostic_information(ex)
                 << "'. Rethrowing the exception.";
    throw;
  }
  catch (...)
  {
    qiLogError() << errorMessagePrefix << "unknown exception. Rethrowing the exception.";
    throw;
  }
}

Future<void> ServiceDirectoryProxy::Impl::close()
{
  return _strand.async([&] { closeUnsync(); });
}

Future<UrlVector> ServiceDirectoryProxy::Impl::endpoints() const
{
  return _strand.async([&] { return _server ? _server->endpoints() : UrlVector{}; });
}

Future<ServiceDirectoryProxy::ListeningStatus> ServiceDirectoryProxy::Impl::listenAsync(
    const Url& url)
{
  return _strand.async([=] {
        _listenUrl = url;

        if (!connected)
        {
          qiLogVerbose() << "ServiceDirectoryProxy is not connected to the service directory, it "
                            "will start listening once the connection is established";
          return futurize(ListeningStatus::PendingOnConnection);
        }

        static const auto errorMsgPrefix =
            "Exception caught while trying to instanciate the server, reason: ";
        try
        {
          qiLogVerbose() << "Instanciating server";
          listening = false;
          _serviceIndexes.clear();
          _server = createServerUnsync();
        }
        catch (const std::exception& ex)
        {
          return logThenReturnFutureError<ListeningStatus>(LogLevel_Error, [&](std::ostream& os) {
            os << errorMsgPrefix << "standard exception '" << ex.what() << "'";
          });
        }
        catch (const boost::exception& ex)
        {
          return logThenReturnFutureError<ListeningStatus>(LogLevel_Error, [&](std::ostream& os) {
            os << errorMsgPrefix << "boost exception '" << boost::diagnostic_information(ex) << "'";
          });
        }

        qiLogInfo() << "Starting listening on URL '" << url.str() << "'";
        return _server->listenStandalone(url).async()
            .then(_strand.unwrappedSchedulerFor([=](Future<void> listenFut) {
              if (listenFut.hasError())
              {
                return logThenReturnFutureError<ListeningStatus>(
                    LogLevel_Error, [&](std::ostream& os) {
                      os << "Error while trying to listen at '" << url.str()
                         << "': " << listenFut.error();
                    });
              }
              listening = true;
              return futurize(ListeningStatus::Done);
            })).unwrap();
      }).unwrap();
}

Future<ServiceDirectoryProxy::IdValidationStatus>
ServiceDirectoryProxy::Impl::setValidateIdentity(const std::string& key, const std::string& crt)
{
  return _strand.async([=] {
        if (key.empty() || crt.empty())
        {
          _identity.reset();
          return makeFutureError<IdValidationStatus>(
              "Either the key or the certificate path is empty.");
        }
        _identity = Identity{ key, crt };
        if (!_server)
          return futurize(IdValidationStatus::PendingCheckOnListen);
        return _server->setIdentity(key, crt) ?
                   futurize(IdValidationStatus::Done) :
                   makeFutureError<IdValidationStatus>(
                       "ServiceDirectoryProxy identity was not accepted by the server.");
      }).unwrap();
}

Future<void> ServiceDirectoryProxy::Impl::setAuthProviderFactory(AuthProviderFactoryPtr provider)
{
  return _strand.async([=] {
    _authProviderFactory = provider;
    if (_server)
      _server->setAuthProviderFactory(placeholderIfNull(provider));
  });
}

Future<void> ServiceDirectoryProxy::Impl::attachToServiceDirectory(const Url& sdUrl)
{
  if (!sdUrl.isValid()) return makeFutureError<void>("Invalid service directory URL");
  return _strand.async([=] {
        qiLogDebug() << "Instanciating new service directory client session";
        connected = false;
        _sdClient.reset(new Session);

        qiLogInfo() << "Attaching to service directory at URL '" << sdUrl.str() << "'";
        return _sdClient->connect(sdUrl).async()
            .then(_strand.unwrappedSchedulerFor([=](Future<void> connectFut) {
              if (connectFut.hasError())
                return delayRetryAttach(sdUrl); // try again
              connected = true;
              bindToServiceDirectoryUnsync(sdUrl);
              return futurize();
            })).unwrap();
      }).unwrap();
}

Future<ServiceDirectoryProxy::ServiceFilter> ServiceDirectoryProxy::Impl::setServiceFilter(
    ServiceFilter filter)
{
  return _strand.async([=]() mutable {
    swap(_serviceFilter, filter);
    return filter;
  });
}

Future<void> ServiceDirectoryProxy::Impl::mirrorAllServices()
{
  return _strand.async([=] {
        if (!_sdClient)
          return makeFutureError<MirroringResults>("Not connected to service directory");
        qiLogVerbose() << "Mirroring services: requesting list of services from ServiceDirectory";
        return _sdClient->services().async()
            .andThen(_strand.schedulerFor([=](const std::vector<ServiceInfo>& services) {
              qiLogVerbose()
                  << "Mirroring services: received list of services from the ServiceDirectory";
              MirroringResults results;
              for (const auto& serviceInfo : services)
                results.push_back({ serviceInfo.name(), mirrorServiceUnsync(serviceInfo.name()) });
              return results;
            })).unwrap();
      }).unwrap()
      .andThen(&logAnyMirroringFailure);
}

MirroringResult::Status ServiceDirectoryProxy::Impl::mirrorServiceUnsync(
    const std::string& serviceName)
{
  qiLogInfo() << "Mirroring service '" << serviceName << "'";
  if (!shouldMirrorServiceUnsync(serviceName))
  {
    qiLogVerbose() << "Service '" << serviceName << "' should not be mirrored, skipping";
    return MirroringResult::Status::Skipped;
  }

  if (!listening)
    return MirroringResult::Status::Failed_NotListening;
  if (!_sdClient)
    return MirroringResult::Status::Failed_NoSdConnection;

  AnyObject service;
  try
  {
    qiLogVerbose() << "Getting service '" << serviceName << "' from service directory";
    service = _sdClient->service(serviceName).value();
    qiLogVerbose() << "Got service '" << serviceName << "' from service directory";
  }
  catch (const std::exception& e)
  {
    qiLogError() << "Failed to get service '" << serviceName << "': " << e.what();
    return MirroringResult::Status::Failed_Error;
  }

  boost::optional<unsigned int> serviceId;
  try
  {
    qiLogVerbose() << "Registering service '" << serviceName << "'";
    serviceId = _server->registerService(serviceName, service).value();
    qiLogVerbose() << "Registered service '" << serviceName << "' (#" << *serviceId << ")";

    _serviceIndexes[serviceName] = *serviceId;
  }
  catch (const std::exception& e)
  {
    qiLogError() << "Failed to register service '" << serviceName << "': " << e.what();
    if (serviceId)
    {
      qiLogInfo() << "Unregistering '" << serviceName
                  << "' (#" << *serviceId << ") (rollback because an error occurred)";
      _serviceIndexes.erase(serviceName);
      _server->unregisterService(*serviceId).value();
      qiLogVerbose() << "Unregistered service '" << serviceName << "' (#" << *serviceId
                     << ") (rollback because an error occurred)";
    }
    return MirroringResult::Status::Failed_Error;
  }
  return MirroringResult::Status::Done;
}

void ServiceDirectoryProxy::Impl::bindToServiceDirectoryUnsync(const Url& url)
{
  if (!_sdClient || !connected)
    throw std::runtime_error("Not connected to service directory");

  qiLogInfo() << "Binding to service directory at url '" << url.str() << "'";

  bool bindingSucceeded = false;

  // When one of the services is registered to the original service
  // directory, register it to this service directory.
  auto scopedDisconnectServiceRegistered =
      scoped(_sdClient->serviceRegistered.connect(
                 _strand.schedulerFor([=](unsigned int id, const std::string& service) {
                   qiLogInfo() << "Service '" << service << "' (#" << id
                               << ") was just registered on service directory";
                   mirrorServiceUnsync(service);
                 })),
             [&](const SignalSubscriber& sigSub) {
               if (!bindingSucceeded)
                 _sdClient->serviceRegistered.disconnect(sigSub.link());
             });

  // When one of the services we track is unregistered from the original service
  // directory, unregister it from the this service directory.
  auto scopedDisconnectServiceUnregistered =
      scoped(_sdClient->serviceUnregistered.connect(
                 _strand.schedulerFor([=](unsigned int id, const std::string& service) {
                   qiLogInfo() << "Service '" << service << "' (#" << id
                               << ") was just unregistered from service directory";
                   removeServiceUnsync(service);
                 })),
             [&](const SignalSubscriber& sigSub) {
               if (!bindingSucceeded)
                 _sdClient->serviceUnregistered.disconnect(sigSub.link());
             });

  // When disconnected, try to reconnect
  auto scopedDisconnectDisconnected =
      scoped(_sdClient->disconnected
                 .connect(_strand.schedulerFor([=](const std::string& reason) {
                   handleSdDisconnectionUnsync(url, reason);
                 })),
             [&](const SignalSubscriber& sigSub) {
               if (!bindingSucceeded)
                 _sdClient->disconnected.disconnect(sigSub.link());
             });

  bindingSucceeded = true;
}

void ServiceDirectoryProxy::Impl::handleSdDisconnectionUnsync(const Url& url,
                                                              const std::string& reason)
{
  qiLogWarning() << "Lost connection to the service directory, reason: " << reason;
  closeUnsync();
  delayRetryAttach(url);
}

Future<void> ServiceDirectoryProxy::Impl::delayRetryAttach(const Url& url, Seconds delay)
{
  qiLogInfo() << "Retrying to attach to the service directory in "
              << boost::chrono::duration_cast<Seconds>(delay).count() << "sec.";
  return asyncDelay(_strand.schedulerFor([=] {
      return retryAttach(url, delay);
  }), delay).unwrap();
}

bool ServiceDirectoryProxy::Impl::shouldMirrorServiceUnsync(boost::string_ref serviceName) const
{
  return serviceName != Session::serviceDirectoryServiceName() && !_serviceFilter(serviceName);
}

void ServiceDirectoryProxy::Impl::removeServiceUnsync(const std::string& serviceName)
{
  if (!_server)
    return;

  try
  {
    const auto serviceIndexIt = _serviceIndexes.find(serviceName);
    if (serviceIndexIt == end(_serviceIndexes))
    {
      qiLogWarning() << "Cannot unregister service '" << serviceName
                     << "' : could not find local index of the service";
      return;
    }
    const auto id = serviceIndexIt->second;
    _serviceIndexes.erase(serviceIndexIt);

    qiLogInfo() << "Unregistering service '" << serviceName << "', (#" << id << ")";
    _server->unregisterService(id).value();
    qiLogVerbose() << "Unregistered service '" << serviceName << "', (#" << id << ")";
  }
  catch (const std::exception& e)
  {
    qiLogInfo() << "Error while unregistering " << serviceName << ": " << e.what();
    throw;
  }
}

Future<void> ServiceDirectoryProxy::Impl::retryAttach(const Url& sdUrl, Seconds lastDelay)
{
  return attachToServiceDirectory(sdUrl)
      .then(_strand.unwrappedSchedulerFor([=](const Future<void>& attachFuture) {
        if (attachFuture.hasError())
        {
          auto newDelay = std::max(lastDelay * delayIncreaseFactor, maxRetryDelay());
          qiLogWarning() << "Could not reach ServiceDirectory at URL '" << sdUrl.str() << "'";
          return delayRetryAttach(sdUrl, newDelay);
        }

        qiLogInfo() << "Successfully established connection to the ServiceDirectory at URL '"
                    << sdUrl.str() << "'";
        return futurize();
      })).unwrap();
}

std::unique_ptr<Session> ServiceDirectoryProxy::Impl::createServerUnsync()
{
  std::unique_ptr<Session> server{ new Session{ _isEnforcedAuth } };

  if (_identity && !server->setIdentity(_identity->key, _identity->crt))
  {
    throw std::runtime_error{ "Invalid identity parameters : key: '" + _identity->key +
                              "', crt: '" + _identity->crt + "'" };
  }
  server->setAuthProviderFactory(placeholderIfNull(_authProviderFactory));
  return server;
}

}
