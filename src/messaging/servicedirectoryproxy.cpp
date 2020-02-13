/*
**  Copyright (C) 2018 Aldebaran Robotics
**  See COPYING for the license
*/

#include "clientauthenticator_p.hpp"
#include "server.hpp"
#include <ka/errorhandling.hpp>
#include <ka/functional.hpp>
#include <ka/scoped.hpp>
#include <ka/typetraits.hpp>
#include <qi/log.hpp>
#include <qi/messaging/clientauthenticatorfactory.hpp>
#include <qi/messaging/servicedirectoryproxy.hpp>
#include <qi/url.hpp>
#include <qi/macro.hpp>

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/optional.hpp>

#include <unordered_map>

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
  boost::optional<unsigned int> mirroredId;
};
using MirroringResults = std::vector<MirroringResult>;

const auto delayIncreaseExponent = 1;
const auto delayIncreaseFactor = 1 << delayIncreaseExponent;
const auto maxTryCount = 7;

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

MirroringResult mirrorService(const std::string& name,
                              Session& srcSess,
                              Session& destSess,
                              const std::string& srcDesc,
                              const std::string& destDesc)
{
  AnyObject service;
  try
  {
    qiLogVerbose() << "Getting service '" << name << "' from " << srcDesc << ".";
    service = srcSess.service(name).value();
    qiLogVerbose() << "Got service '" << name << "' from " << srcDesc << ".";
  }
  catch (const std::exception& e)
  {
    qiLogError() << "Failed to get service '" << name << "' from " << srcDesc << ": " << e.what();
    return { name, MirroringResult::Status::Failed_Error, {} };
  }

  boost::optional<unsigned int> destId;
  try
  {
    qiLogVerbose() << "Registering service '" << name << "' on " << destDesc << ".";
    destId = destSess.registerService(name, service).value();
    qiLogVerbose() << "Registered service '" << name << "' (#" << *destId << ") on " << destDesc
                   << ".";
  }
  catch (const std::exception& e)
  {
    qiLogError() << "Failed to register service '" << name << " on " << destDesc
                 << " ': " << e.what();
    if (destId)
    {
      qiLogVerbose() << "Unregistering '" << name << "' (#" << *destId
                     << ") on " << destDesc << " (rollback because an error occurred)";
      ka::invoke_catch(ka::constant_function(), // TODO: Log something as verbose when there is
                                                // something like LogExceptionError for all levels.
                                                // For now do nothing, it should be rare enough to
                                                // be acceptable.
                       [&] { destSess.unregisterService(*destId).value(); });
      qiLogVerbose() << "Unregistered service '" << name << "' (#" << *destId
                     << ") on " << destDesc << " (rollback because an error occurred)";
    }
    return { name, MirroringResult::Status::Failed_Error, {} };
  }
  return { name, MirroringResult::Status::Done, destId };
}

static const ServiceDirectoryProxy::Status totallyDisconnected{
  ServiceDirectoryProxy::ConnectionStatus::NotConnected,
  ServiceDirectoryProxy::ListenStatus::NotListening
};

}

// To implement the proxy, we use a session that listens and provides endpoints for connections and
// another session that connects to the service directory. The first session is referenced as
// the 'server' and the second is referenced as the 'service directory client'.
class ServiceDirectoryProxy::Impl
{
public:
  static const Seconds initTryDelay;
  static Seconds maxTryDelay();

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
  // Precondition synchronized():
  // Concurrent calls of functions with the same precondition on the same object must be sequenced.

  // Precondition: synchronized()
  void closeUnsync();

  Future<void> mirrorAllServices();

  // Precondition: synchronized()
  MirroringResult mirrorServiceFromSDUnsync(unsigned int remoteId, const std::string& serviceName);

  // Precondition: synchronized() && _sdClient
  //
  // Binds to a service directory by connecting to its signals.
  void bindToServiceDirectoryUnsync();

  // Precondition: synchronized()
  void unmirrorServiceFromSDUnsync(const std::string& service);

  // Precondition: synchronized() && _sdUrl.isValid()
  //
  // Attaches the proxy to a service directory at _sdUrl. Returns a future set when the attachment
  // is successful.
  Future<void> doAttachUnsync();

  // Precondition: synchronized()
  //
  // Tries to reattach to a service directory and in case of failure, schedules another try with
  // a delay calculated from the last try delay. Returns a future set when the attachment is
  // successful.
  Future<void> tryAttachUnsync(Seconds lastDelay);

  // Precondition: synchronized()
  //
  // Instanciates a server (as a session) and initializes it with the current state of the proxy.
  // Returns the pointer the new server.
  SessionPtr createServerUnsync();

  // Precondition: synchronized()
  //
  // Resets this object by closing it then trying to reattach it to the service directory.
  void resetUnsync();

  // Tries to attach to a service directory after a delay.
  Future<void> delayTryAttach(Seconds delay = initTryDelay);

  // Precondition: synchronized()
  //
  // Returns true if the service should be mirrored, regardless of the current state of the proxy.
  bool shouldMirrorServiceFromSDUnsync(const std::string& name) const;

  // Precondition: synchronized()
  MirroringResult mirrorServiceToSDUnsync(unsigned int localId, const std::string& serviceName);

  // Precondition: synchronized()
  void unmirrorServiceToSDUnsync(const std::string& serviceName);

  // Precondition: synchronized()
  //
  // Returns an optional set with a failed MirroringResult if the service can not be mirrored,
  // otherwise returns a empty optional.
  boost::optional<MirroringResult> immediateMirroringFailureUnsync(const std::string& name) const;

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

  SessionPtr _server; // ptr because we have to recreate it every time we listen
  SessionPtr _sdClient;

  struct MirroredServiceInfo
  {
    unsigned int localId;
    unsigned int remoteId;
    enum class Source { Proxy, ServiceDirectory };
    Source source;
  };
  using MirroredServiceInfoMap = std::unordered_map<std::string, MirroredServiceInfo>;
  MirroredServiceInfoMap _servicesInfo;
  Url _listenUrl;
  Url _sdUrl;
  boost::optional<Identity> _identity;
  AuthProviderFactoryPtr _authProviderFactory;
  bool _isEnforcedAuth;
  ServiceFilter _serviceFilter;

  mutable Strand _strand;
};

const Seconds ServiceDirectoryProxy::Impl::initTryDelay { 1 };

QI_WARNING_PUSH()
QI_WARNING_DISABLE(4996, deprecated-declarations) // ignore connected deprecation warnings
ServiceDirectoryProxy::ServiceDirectoryProxy(bool enforceAuth)
  : _p(new Impl(enforceAuth))
  , connected(_p->connected)
  , status(_p->status)
{
}
QI_WARNING_POP()

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
  , _serviceFilter{ ka::constant_function(false) }
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


Seconds ServiceDirectoryProxy::Impl::maxTryDelay()
{
  static const Seconds val = boost::chrono::duration_cast<Seconds>(
      initTryDelay * std::ldexp(initTryDelay.count(), delayIncreaseExponent * maxTryCount));
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
    _servicesInfo.clear();
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
          _servicesInfo.clear();
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

        qiLogVerbose() << "Starting listening on URL '" << url.str() << "'";
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
        return doAttachUnsync();
      }).unwrap()
      .then([=](Future<void> connectFut) {
        if (connectFut.hasError())
          connectFut = delayTryAttach(); // try again
        return connectFut;
      }).unwrap();
}

Future<void> ServiceDirectoryProxy::Impl::doAttachUnsync()
{
  qiLogVerbose() << "Attaching to service directory at URL '" << _sdUrl.str() << "'";

  _status.set(ConnectionStatus::NotConnected);

  qiLogDebug() << "Instanciating new service directory client session";
  _sdClient = makeSession();
  _status.set(ConnectionStatus::Starting);

  return _sdClient->connect(_sdUrl).async()
      .then(_strand.unwrappedSchedulerFor([=](Future<void> connectFut) {
        if (connectFut.hasError())
        {
          _sdClient.reset();
          _status.set(ConnectionStatus::NotConnected);
        }
        else
        {
          bindToServiceDirectoryUnsync();
          _status.set(ConnectionStatus::Connected);
        }
        return connectFut;
      })).unwrap();

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
                results.push_back(mirrorServiceFromSDUnsync(serviceInfo.serviceId(), serviceInfo.name()));
              return results;
            })).unwrap();
      }).unwrap()
      .andThen(&logAnyMirroringFailure);
}

MirroringResult ServiceDirectoryProxy::Impl::mirrorServiceFromSDUnsync(unsigned int remoteId,
                                                                       const std::string& name)
{
  qiLogVerbose() << "Trying to mirror service '" << name << "' from the service directory.";

  if (const auto mirroringFailure = immediateMirroringFailureUnsync(name))
    return *mirroringFailure;

  if (!shouldMirrorServiceFromSDUnsync(name))
  {
    qiLogVerbose() << "Service '" << name << "' should not be mirrored, skipping";
    return { name, MirroringResult::Status::Skipped, {} };
  }

  qiLogVerbose() << "Mirroring service '" << name << "' from the service directory to the proxy.";
  const auto result =
      mirrorService(name, *_sdClient, *_server, "service directory", "proxy");
  if (result.mirroredId)
    _servicesInfo[name] = { *result.mirroredId, remoteId,
                            MirroredServiceInfo::Source::ServiceDirectory };
  return result;
}

void ServiceDirectoryProxy::Impl::bindToServiceDirectoryUnsync()
{
  using ka::scoped;
  QI_ASSERT_TRUE(_sdClient);
  qiLogVerbose() << "Binding to service directory at url '" << _sdUrl.str() << "'";

  bool bindingSucceeded = false;

  // When one of the services is registered to the original service
  // directory, register it to this service directory.
  auto scopedDisconnectServiceRegistered =
      scoped(_sdClient->serviceRegistered.connect(
                 _strand.schedulerFor([=](unsigned int remoteId, const std::string& service) {
                   qiLogVerbose() << "Service '" << service << "' (#" << remoteId
                                  << ") was just registered on service directory";
                   mirrorServiceFromSDUnsync(remoteId, service);
                 })),
             [&](const SignalSubscriber& sigSub) {
               if (!bindingSucceeded)
                 _sdClient->serviceRegistered.disconnect(sigSub.link());
             });

  // When one of the services we track is unregistered from the original service
  // directory, unregister it from the this service directory.
  auto scopedDisconnectServiceUnregistered =
      scoped(_sdClient->serviceUnregistered.connect(
                 _strand.schedulerFor([=](unsigned int remoteId, const std::string& service) {
                   qiLogVerbose() << "Service '" << service << "' (#" << remoteId
                                  << ") was just unregistered from service directory";
                   unmirrorServiceFromSDUnsync(service);
                 })),
             [&](const SignalSubscriber& sigSub) {
               if (!bindingSucceeded)
                 _sdClient->serviceUnregistered.disconnect(sigSub.link());
             });

  // When disconnected, try to reconnect
  auto scopedDisconnectDisconnected =
      scoped(_sdClient->disconnected
                 .connect(_strand.schedulerFor([=](const std::string& reason) {
                   qiLogWarning() << "Lost connection to the service directory, reason: " << reason;
                   resetUnsync();
                 })),
             [&](const SignalSubscriber& sigSub) {
               if (!bindingSucceeded)
                 _sdClient->disconnected.disconnect(sigSub.link());
             });

  bindingSucceeded = true;
}

void ServiceDirectoryProxy::Impl::resetUnsync()
{
  qiLogVerbose() << "Resetting";
  closeUnsync();
  delayTryAttach();
}

Future<void> ServiceDirectoryProxy::Impl::delayTryAttach(Seconds delay)
{
  qiLogVerbose() << "Trying to attach to the service directory in "
                 << boost::chrono::duration_cast<Seconds>(delay).count() << "sec.";
  return asyncDelay(_strand.schedulerFor([=] { return tryAttachUnsync(delay); }), delay).unwrap();
}

bool ServiceDirectoryProxy::Impl::shouldMirrorServiceFromSDUnsync(const std::string& name) const
{
  return name != Session::serviceDirectoryServiceName() && !_serviceFilter(name);
}

MirroringResult ServiceDirectoryProxy::Impl::mirrorServiceToSDUnsync(unsigned int localId,
                                                                     const std::string& name)
{
  qiLogVerbose() << "Trying to mirror service '" << name << "' to the service directory.";

  if (const auto mirroringFailure = immediateMirroringFailureUnsync(name))
    return *mirroringFailure;

  qiLogVerbose() << "Mirroring service '" << name << "' from the proxy to the service directory.";
  const auto result =
      mirrorService(name, *_server, *_sdClient, "proxy", "service directory");
  if (result.mirroredId)
    _servicesInfo[name] = { localId, *result.mirroredId, MirroredServiceInfo::Source::Proxy };
  return result;
}

void ServiceDirectoryProxy::Impl::unmirrorServiceToSDUnsync(const std::string& serviceName)
{
  if (!_sdClient)
    return;

  try
  {
    const auto serviceIndexIt = _servicesInfo.find(serviceName);
    if (serviceIndexIt == end(_servicesInfo))
    {
      qiLogVerbose() << "Cannot unmirror service '" << serviceName
                     << "' to service directory : could not find mirrored service info.";
      return;
    }

    if (serviceIndexIt->second.source != MirroredServiceInfo::Source::Proxy)
    {
      qiLogVerbose() << "Cannot unmirror service '" << serviceName
                     << "' to service directory : service was not registered locally.";
      return;
    }

    auto scopeErase = ka::scoped([&]{ _servicesInfo.erase(serviceIndexIt); });

    const auto localId = serviceIndexIt->second.localId;
    qiLogVerbose() << "Unmirroring service '" << serviceName << "' to service directory, (#"
                   << localId << ").";
    _sdClient->unregisterService(serviceIndexIt->second.remoteId).value();
    qiLogVerbose() << "Unmirrored service '" << serviceName << "' to service directory, (#"
                   << localId << ").";
  }
  catch (const std::exception& e)
  {
    qiLogVerbose() << "Error while unmirroring '" << serviceName
                   << "' to service directory: " << e.what();
    throw;
  }
}

boost::optional<MirroringResult> ServiceDirectoryProxy::Impl::immediateMirroringFailureUnsync(
  const std::string& name) const
{
  // Verify that the service is not already mirrored.
  if (_servicesInfo.find(name) != _servicesInfo.end())
  {
    qiLogVerbose() << "Service '" << name << "' is already mirrored, skipping.";
    return MirroringResult{ name, MirroringResult::Status::Skipped, {} };
  }

  if (!_status.current().isListening())
  {
    qiLogVerbose() << "Proxy is not yet listening, cannot mirror service.";
    return MirroringResult{ name, MirroringResult::Status::Failed_NotListening, {} };
  }
  QI_ASSERT_TRUE(_server);

  if (!_status.current().isConnected())
  {
    qiLogVerbose() << "Proxy is not yet connected, cannot mirror service.";
    return MirroringResult{ name, MirroringResult::Status::Failed_NoSdConnection, {} };
  }
  QI_ASSERT_TRUE(_sdClient);

  return {};
}

void ServiceDirectoryProxy::Impl::unmirrorServiceFromSDUnsync(const std::string& serviceName)
{
  if (!_server)
    return;

  try
  {
    const auto serviceIndexIt = _servicesInfo.find(serviceName);
    if (serviceIndexIt == end(_servicesInfo))
    {
      qiLogVerbose() << "Cannot unmirror service '" << serviceName
                     << "' from service directory : could not find mirrored service info.";
      return;
    }

    if (serviceIndexIt->second.source != MirroredServiceInfo::Source::ServiceDirectory)
    {
      qiLogVerbose() << "Cannot unmirror service '" << serviceName
                     << "' from service directory : service was not registered on the service "
                        "directory first.";
      return;
    }

    auto scopeErase = ka::scoped([&]{ _servicesInfo.erase(serviceIndexIt); });
    const auto localId = serviceIndexIt->second.localId;

    qiLogVerbose() << "Unmirroring service '" << serviceName << "' from the service directory, (#"
                   << localId << ").";
    _server->unregisterService(localId).value();
    qiLogVerbose() << "Unmirrored service '" << serviceName << "' from the service directory, (#"
                   << localId << ").";
  }
  catch (const std::exception& e)
  {
    qiLogVerbose() << "Error while unmirroring '" << serviceName
                   << "' from the service directory: " << e.what();
    throw;
  }
}

Future<void> ServiceDirectoryProxy::Impl::tryAttachUnsync(Seconds lastDelay)
{
  if (!(_sdUrl.isValid())) // precondition
    return makeFutureError<void>(
        "Cannot try to attach to the service directory, the URL is invalid");
  return doAttachUnsync()
      .then(_strand.unwrappedSchedulerFor([=](const Future<void>& attachFuture) {
        if (attachFuture.hasError())
        {
          auto newDelay = std::min(lastDelay * delayIncreaseFactor, maxTryDelay());
          qiLogVerbose() << "Could not attach to the ServiceDirectory at URL '" << _sdUrl.str()
                         << "', reason: '" << attachFuture.error() << "'";
          return delayTryAttach(newDelay);
        }

        qiLogVerbose() << "Successfully established connection to the ServiceDirectory at URL '"
                       << _sdUrl.str() << "'";
        return futurize();
      })).unwrap();
}

SessionPtr ServiceDirectoryProxy::Impl::createServerUnsync()
{
  auto server = makeSession(_isEnforcedAuth);

  if (_identity && !server->setIdentity(_identity->key, _identity->crt))
  {
    throw std::runtime_error{ "Invalid identity parameters : key: '" + _identity->key +
                              "', crt: '" + _identity->crt + "'" };
  }
  server->setAuthProviderFactory(placeholderIfNull(_authProviderFactory));

  bool connectSucceeded = false;

  auto scopedDisconnectServiceRegistered =
      ka::scoped(server->serviceRegistered.connect(
                     _strand.schedulerFor([=](unsigned int localId, const std::string& name) {
                       mirrorServiceToSDUnsync(localId, name);
                     })),
                 [&](const SignalSubscriber& sigSub) {
                   if (!connectSucceeded)
                     _server->serviceRegistered.disconnect(sigSub.link());
                 });

  auto scopedDisconnectServiceUnregistered =
      ka::scoped(server->serviceUnregistered.connect(
                     _strand.schedulerFor([=](unsigned int, const std::string& name) {
                       unmirrorServiceToSDUnsync(name);
                     })),
                 [&](const SignalSubscriber& sigSub) {
                   if (!connectSucceeded)
                     _server->serviceRegistered.disconnect(sigSub.link());
                 });

  connectSucceeded = true;

  return server;
}

namespace
{
  template <typename T>
  void printUnexpectedEnumValue(std::ostream& out, T value)
  {
    out << "<UNEXPECTED VALUE '" << static_cast<ka::UnderlyingType<T>>(value) << "'>";
  }
}

std::ostream& operator<<(std::ostream& out, ServiceDirectoryProxy::IdValidationStatus status)
{
  using Status = ServiceDirectoryProxy::IdValidationStatus;
  switch (status)
  {
    case Status::Done:                 out << "Done";                         break;
    case Status::PendingCheckOnListen: out << "PendingCheckOnListen";         break;
    default:                           printUnexpectedEnumValue(out, status); break;
  };
  return out;
}

std::ostream& operator<<(std::ostream& out, ServiceDirectoryProxy::ListenStatus status)
{
  using Status = ServiceDirectoryProxy::ListenStatus;
  switch (status)
  {
    case Status::NotListening:      out << "NotListening";                 break;
    case Status::Listening:         out << "Listening";                    break;
    case Status::Starting:          out << "Starting";                     break;
    case Status::PendingConnection: out << "PendingConnection";            break;
    default:                        printUnexpectedEnumValue(out, status); break;
  };
  return out;
}

std::ostream& operator<<(std::ostream& out, ServiceDirectoryProxy::ConnectionStatus status)
{
  using Status = ServiceDirectoryProxy::ConnectionStatus;
  switch (status)
  {
    case Status::NotConnected: out << "NotConnected";                 break;
    case Status::Connected:    out << "Connected";                    break;
    case Status::Starting:     out << "Starting";                     break;
    default:                   printUnexpectedEnumValue(out, status); break;
  };
  return out;
}

}
