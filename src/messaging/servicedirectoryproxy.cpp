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

static const ServiceDirectoryProxy::Status totallyDisconnected{
  ServiceDirectoryProxy::ConnectionStatus::NotConnected,
  ServiceDirectoryProxy::ListenStatus::NotListening
};

}

class ServiceDirectoryProxy::Impl
{
public:
  static const Seconds initRetryDelay;
  static Seconds maxRetryDelay();

  explicit Impl(bool enforceAuth);
  ~Impl();

  Property<bool> connected;

  Property<Status> status;

  Future<void> close();
  Future<UrlVector> endpoints() const;
  Future<ListenStatus> listenAsync(const Url& url);
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

  // Owns the current status value and dispatch the value through the associated property when changed.
  // This is to help maintenance (makes impossible to change the value without setting the property).
  // TODO: Will be replaced by direct usage of the property once we can make
  // the property use the same strand as the one used here
  // (to avoid some issues with concurrency and thread abuse).
  class StatusKeeper
  {
    Status _currentStatus;
    Property<Status>& _publisher;
  public:
    StatusKeeper(Status initialStatus, Property<Status>& publisher)
      : _currentStatus(initialStatus)
      , _publisher(publisher)
    {}

    void set(const Status& newStatus)
    {
      _currentStatus = newStatus;
      _publisher.set(_currentStatus).async();
    }

    void set(ConnectionStatus newValue)
    {
      set({newValue, _currentStatus.listen});
    }

    void set(ListenStatus newValue)
    {
      set({ _currentStatus.connection, newValue });
    }

    const Status& current() const { return _currentStatus; }

  } _status{totallyDisconnected, status};

  std::unique_ptr<Session> _server; // ptr because we have to recreate it every time we listen
  std::map<std::string, unsigned int> _serviceIndexes;
  std::unique_ptr<Session> _sdClient;
  Url _listenUrl;
  Url _sdUrl;
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
  , status(_p->status)
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

Future<ServiceDirectoryProxy::ListenStatus> ServiceDirectoryProxy::listenAsync(const Url& url)
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
  : connected{ false, Property<bool>::Getter{}, util::SetAndNotifyIfChanged{}}
  , status{ totallyDisconnected, Property<Status>::Getter{}, util::SetAndNotifyIfChanged{}}
  ,_isEnforcedAuth(enforceAuth)
  , _serviceFilter{ PolymorphicConstantFunction<bool>{ false } }
{
  status.connect(_strand.schedulerFor([this](const Status& newStatus) {
    connected.set(newStatus.isConnected()).async();

    if (newStatus.isReady())
      mirrorAllServices();

    // Because we close the server if we get disconnected from the SD, automatically restart
    // the server if _listenUrl is valid when we reconnect
    if (newStatus.isConnected()
      && newStatus.listen == ListenStatus::NotListening
      && _listenUrl.isValid())
    {
      listenAsync(_listenUrl);
    }

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
    _status.set(totallyDisconnected);
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

Future<ServiceDirectoryProxy::ListenStatus> ServiceDirectoryProxy::Impl::listenAsync(
    const Url& url)
{
  return _strand.async([=] {

        if (_status.current().listen != ListenStatus::NotListening
        && _listenUrl == url )
        {
          return futurize(_status.current().listen);
        }

        _listenUrl = url;

        if (!_status.current().isConnected())
        {
          qiLogVerbose() << "ServiceDirectoryProxy is not connected to the service directory, it "
                            "will start listening once the connection is established";
          _status.set(ListenStatus::PendingConnection);
          return futurize(_status.current().listen);
        }

        static const auto errorMsgPrefix =
            "Exception caught while trying to instanciate the server, reason: ";
        try
        {
          qiLogVerbose() << "Instanciating server";
          _status.set(ListenStatus::NotListening);
          _serviceIndexes.clear();
          _server = createServerUnsync();
          _status.set(ListenStatus::Starting);
        }
        catch (const std::exception& ex)
        {
          return logThenReturnFutureError<ListenStatus>(LogLevel_Error, [&](std::ostream& os) {
            os << errorMsgPrefix << "standard exception '" << ex.what() << "'";
          });
        }
        catch (const boost::exception& ex)
        {
          return logThenReturnFutureError<ListenStatus>(LogLevel_Error, [&](std::ostream& os) {
            os << errorMsgPrefix << "boost exception '" << boost::diagnostic_information(ex) << "'";
          });
        }

        qiLogInfo() << "Starting listening on URL '" << url.str() << "'";
        return _server->listenStandalone(url).async()
            .then(_strand.unwrappedSchedulerFor([=](Future<void> listenFut) {
              if (listenFut.hasError())
              {
                _status.set(ListenStatus::NotListening);
                return logThenReturnFutureError<ListenStatus>(
                    LogLevel_Error, [&](std::ostream& os) {
                      os << "Error while trying to listen at '" << url.str()
                         << "': " << listenFut.error();
                    });
              }
              _status.set(ListenStatus::Listening);
              return futurize(_status.current().listen);
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
    if (_status.current().connection != ConnectionStatus::NotConnected
    && _sdClient && sdUrl == _sdUrl)
    {
      return futurize();
    }

    _sdUrl = sdUrl;
    _status.set(ConnectionStatus::NotConnected);

    qiLogDebug() << "Instanciating new service directory client session";
    _sdClient.reset(new Session);
    _status.set(ConnectionStatus::Starting);

    qiLogInfo() << "Attaching to service directory at URL '" << sdUrl.str() << "'";
    return _sdClient->connect(sdUrl).async()
        .then(_strand.unwrappedSchedulerFor([=](Future<void> connectFut) {
          if (connectFut.hasError())
          {
            return delayRetryAttach(sdUrl) // try again
              .then(_strand.unwrappedSchedulerFor([=](Future<void> connectFut) {
                if (connectFut.hasError())
                  _status.set(ConnectionStatus::NotConnected);
                return connectFut;
              })).unwrap();
          }
          else
          {
            _status.set(ConnectionStatus::Connected);
            bindToServiceDirectoryUnsync(sdUrl);
            return futurize();
          }
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

  if (!_status.current().isListening())
    return MirroringResult::Status::Failed_NotListening;
  QI_ASSERT_TRUE(_server);

  if (!_status.current().isConnected())
    return MirroringResult::Status::Failed_NoSdConnection;
  QI_ASSERT_TRUE(_sdClient);

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
  if (!_status.current().isConnected())
    throw std::runtime_error("Not connected to service directory");
  QI_ASSERT_TRUE(_sdClient);

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
