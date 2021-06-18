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

#include <boost/version.hpp>
#include <boost/predef/version_number.h>
#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/optional.hpp>
#include <boost/container/flat_map.hpp>

#include <unordered_map>

static const auto category = "qimessaging.servicedirectoryproxy";
qiLogCategory(category);

using ServiceId = unsigned int;

namespace qi
{

namespace
{

const std::string notListeningMsgPrefix = "the proxy is not listening yet";
const std::string notConnectedMsgPrefix = "the proxy is not connected yet";
const std::string cannotMirrorServiceMsgSuffix = ", cannot mirror service";
const std::string cannotUnmirrorServiceMsgSuffix = ", cannot unmirror service";
const std::string skippedErrorMsg = "the service was skipped";

const std::string notListeningMirrorErrorMsg = notListeningMsgPrefix
                                               + cannotMirrorServiceMsgSuffix;
const std::string notConnectedMirrorErrorMsg = notConnectedMsgPrefix
                                               + cannotMirrorServiceMsgSuffix;
const std::string notListeningUnmirrorErrorMsg = notListeningMsgPrefix
                                                 + cannotUnmirrorServiceMsgSuffix;
const std::string notConnectedUnmirrorErrorMsg = notConnectedMsgPrefix
                                                 + cannotUnmirrorServiceMsgSuffix;

using FutureMaybeMirroredIdMap = boost::container::flat_map<std::string, Future<unsigned int>>;

void logAnyMirroringFailure(const FutureMaybeMirroredIdMap& ids)
{
  using namespace boost::adaptors;
  const auto filterNonSkipError = filtered([](const FutureMaybeMirroredIdMap::value_type& pairMaybeId) {
      auto fut = pairMaybeId.second;
      // Do not log mirroring failures caused by service skipping.
      return fut.hasError() && fut.error() != skippedErrorMsg;
    });
  const auto concatFutureError = transformed([](const FutureMaybeMirroredIdMap::value_type& pairMaybeId) {
      return pairMaybeId.first + " ('" + pairMaybeId.second.error() + "')";
    });
  const auto serviceErrors = ids | filterNonSkipError | concatFutureError;
  if (!boost::empty(serviceErrors))
    qiLogWarning() << "Failed to mirror the following services: "
                   << boost::algorithm::join(serviceErrors, ", ") << ".";
}

AuthProviderFactoryPtr placeholderIfNull(AuthProviderFactoryPtr factory)
{
  if (!factory)
    return boost::make_shared<NullAuthProviderFactory>();
  return factory;
}

/// Procedure<Future<T>()> Proc
template<typename Proc>
auto invokeLogProgress(const std::string& prefix, Proc&& proc)
  -> decltype(ka::fwd<Proc>(proc)())
{
  qiLogVerbose() << prefix << " - ...";
  auto res = ka::fwd<Proc>(proc)();
  res.connect([=](decltype(res) fut) {
    if (fut.hasError())
    {
      qiLogVerbose() << prefix << " - failure, reason: '" << fut.error() << "'.";
      return;
    }
    if (fut.isCanceled())
    {
      qiLogVerbose() << prefix << " - failure, reason: canceled.";
      return;
    }
    QI_ASSERT_TRUE(fut.hasValue());
    qiLogVerbose() << prefix << " - done.";
  }, FutureCallbackType_Sync);
  return res;
}

static const ServiceDirectoryProxy::Status totallyDisconnected{
  ServiceDirectoryProxy::ConnectionStatus::NotConnected,
  ServiceDirectoryProxy::ListenStatus::NotListening
};

/// Repeats an operation returning a Future until it succeeds or is cancelled, with a delay between
/// each try.
///
/// @param op The operation, as a procedure to invoke.
/// @param strand The strand that will execute the operation.
/// @param opDesc The description of the operation, for logging purposes.
/// @returns The future result of the first successful try of the operation.
///
/// Procedure<Fut()> ProcOp
template <typename ProcOp,
          typename Fut = decltype(std::declval<ProcOp>()())>
Fut repeatWhileError(ProcOp op,
                     Strand& strand,
                     const std::string& opDesc,
                     Duration delay)
{
  // We execute everything in the strand not just to synchronize the calls to the operation, but
  // also to ensure that the strand is still alive when we access it.
  return strand.async(op).unwrap()
    .then(strand.unwrappedSchedulerFor([=, &strand](const Fut& fut) {
      if (fut.hasError())
      {
        qiLogVerbose()
          << "Retrying to " << opDesc << " in "
          << boost::chrono::duration_cast<MilliSeconds>(delay).count()
          << "msec.";
        return strand.asyncDelay(
            [=, &strand] {
              return repeatWhileError(op, strand, opDesc, delay);
            },
            delay).unwrap();
      }
      return fut;
    })).unwrap();
}

}

// To implement the proxy, we use a session that listens and provides endpoints for connections and
// another session that connects to the service directory. The first session is referenced as
// the 'server' and the second is referenced as the 'service directory client'.
class ServiceDirectoryProxy::Impl
{
public:
  static const Seconds retryAttachDelay;
  static const MilliSeconds retryServiceDelay;

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
  Future<unsigned int> mirrorServiceFromSDUnsync(unsigned int remoteId,
                                                 const std::string& name);

  // Precondition: synchronized() && _sdClient
  //
  // Binds to a service directory by connecting to its signals.
  void bindToServiceDirectoryUnsync();

  // Precondition: synchronized()
  Future<void> unmirrorServiceFromSDUnsync(const std::string& service);

  // Precondition: synchronized() && _sdUrl.isValid()
  //
  // Attaches the proxy to a service directory at _sdUrl. Returns a future set
  // when the attachment is successful.
  Future<void> doAttachUnsync();

  // Precondition: synchronized()
  //
  // Instantiates a server (as a session) and initializes it with the current state of the proxy.
  // Returns the pointer the new server.
  SessionPtr createServerUnsync();

  // Precondition: synchronized()
  //
  // Resets this object by closing it then trying to reattach it to the service directory.
  void resetUnsync();

  // Precondition: synchronized()
  //
  // Returns true if the service should be mirrored, regardless of the current state of the proxy.
  bool shouldMirrorServiceFromSDUnsync(const std::string& name) const;

  // Precondition: synchronized()
  Future<unsigned int> mirrorServiceToSDUnsync(unsigned int localId, const std::string& name);

  // Precondition: synchronized()
  Future<void> unmirrorServiceToSDUnsync(const std::string& name);

  // Precondition: synchronized() && srcSess != nullptr && destSess != nullptr
  Future<unsigned int> mirrorServiceUnsync(const std::string& name,
                                           const SessionPtr& srcSess,
                                           const SessionPtr& destSess,
                                           const std::string& srcDesc,
                                           const std::string& destDesc);

  // Precondition: synchronized()
  //
  // Returns an optional set with an error message if the service cannot be mirrored, otherwise
  // returns a empty optional.
  boost::optional<std::string> immediateMirroringFailureUnsync(const std::string& name) const;

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

  struct MirroredFromServiceDirectoryServiceId
  {
    Future<unsigned int> local;
    unsigned int remote;
  };
  struct MirroredFromProxyServiceId
  {
    unsigned int local;
    Future<unsigned int> remote;
  };
  using MirroredServiceId = boost::variant<MirroredFromServiceDirectoryServiceId,
                                           MirroredFromProxyServiceId>;
  using MirroredServiceIdMap = std::unordered_map<std::string, MirroredServiceId>;
  MirroredServiceIdMap _servicesIdMap;
  Url _listenUrl;
  Url _sdUrl;
  boost::optional<Identity> _identity;
  AuthProviderFactoryPtr _authProviderFactory;
  bool _isEnforcedAuth;
  ServiceFilter _serviceFilter;

  mutable Strand _strand;
};

const Seconds ServiceDirectoryProxy::Impl::retryAttachDelay { 1 };
const MilliSeconds ServiceDirectoryProxy::Impl::retryServiceDelay { 500 };

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
  ka::invoke_catch(
    exceptionLogWarning(category,
                        "Exception caught during destruction of implementation class: "),
    [&] {
      _strand.join();
      closeUnsync();
    });
}


void ServiceDirectoryProxy::Impl::closeUnsync()
{
  qiLogVerbose() << "Closing proxy.";
  auto sdClient = ka::exchange(_sdClient, nullptr); // will be destroyed at end of scope
  auto server = ka::exchange(_server, nullptr);     // idem
  qiLogVerbose() << "Setting the status of the proxy to disconnected.";
  _status.set(totallyDisconnected);
  qiLogVerbose() << "Clearing the list of known services.";
  _servicesIdMap.clear();
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
      if (_status.current().listen != ListenStatus::NotListening && _listenUrl == url)
      {
        // listen is already establishing / ongoing.
        return futurize(_status.current().listen);
      }

      _listenUrl = url;

      if (!_status.current().isConnected())
      {
        qiLogVerbose() << "ServiceDirectoryProxy is not connected to the service directory, it "
                          "will start listening once the connection is established.";
        _status.set(ListenStatus::PendingConnection);
        return futurize(_status.current().listen);
      }

      constexpr auto errorMsgPrefix =
        "Exception caught while trying to instantiate the server, reason: ";

      using namespace ka::functional_ops;
      const auto instantiateServerSession = [&]() -> boost::optional<std::string> {
        qiLogVerbose() << "Instantiating server session.";
        _status.set(ListenStatus::NotListening);
        _servicesIdMap.clear();
        _server.reset();
        _server = createServerUnsync();
        _status.set(ListenStatus::Starting);
        return {};
      };
      const auto logError = ka::exception_message_t{}
          | [&](const std::string& msg) -> boost::optional<std::string> {
            const auto error = errorMsgPrefix + msg;
            qiLogVerbose() << error;
            return error;
          };
      const auto maybeError = ka::invoke_catch(logError, instantiateServerSession);
      if (maybeError)
      {
        _status.set(ListenStatus::NotListening);
        return makeFutureError<ListenStatus>(*maybeError);
      }

      qiLogVerbose() << "Starting server session listening on URL '" << url.str() << "'.";
      return _server->listenStandalone(url).async()
        .then(_strand.unwrappedSchedulerFor([=](Future<void> listenFut) {
          if (listenFut.hasError())
          {
            _status.set(ListenStatus::NotListening);
            std::ostringstream oss;
            oss << "Error while trying to listen at '" << url.str() << "': " << listenFut.error();
            const auto error = oss.str();
            qiLogVerbose() << error;
            return makeFutureError<ListenStatus>(error);
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
      if (_status.current().connection != ConnectionStatus::NotConnected &&
          _sdClient && sdUrl == _sdUrl)
      {
        return futurize();
      }

      _sdUrl = sdUrl;
      return doAttachUnsync();
    }).unwrap();
}

Future<void> ServiceDirectoryProxy::Impl::doAttachUnsync()
{
  if (!_sdUrl.isValid())
    return makeFutureError<void>(
      "Cannot attach to the service directory, the URL is invalid");

  const auto disconnect = [=] {
    _sdClient.reset();
    _status.set(ConnectionStatus::NotConnected);
  };

  const auto connect = [=] {
    return invokeLogProgress(
      "Attaching to service directory at URL '" + _sdUrl.str() + "'", [&] {
        disconnect();

        qiLogVerbose() << "Instantiating new service directory client session.";
        _sdClient = makeSession();
        _status.set(ConnectionStatus::Starting);

        return _sdClient->connect(_sdUrl)
          .async()
          .then(_strand.unwrappedSchedulerFor([=](Future<void> connectFut) {
            if (connectFut.hasError())
            {
              disconnect();
            }
            else
            {
              bindToServiceDirectoryUnsync();
              _status.set(ConnectionStatus::Connected);
            }
            return connectFut;
          })).unwrap();
      });
  };

  return repeatWhileError(connect, _strand,
                          "attach to the service directory",
                          retryAttachDelay);
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
          return makeFutureError<FutureMaybeMirroredIdMap>(notConnectedMirrorErrorMsg);
        return invokeLogProgress(
          "Mirroring services: requesting list of services from ServiceDirectory",
          [&]{
            return _sdClient->services().async();
          }).andThen(_strand.unwrappedSchedulerFor([=](const std::vector<ServiceInfo>& services) {
              using namespace boost::adaptors;
              const auto mirroredIds =
                services | transformed(
#if (BOOST_VERSION >= 106600) && (BOOST_VERSION < 106800)
              // Change [1] in Boost.Optional 1.66 broke Boost.Range's support for lambdas,
              // which was then fixed in Boost.Range 1.68 with [2].
              // Wrap the lambda in a boost::function if Boost in [1.66, 1.68[ is used.
              //
              // [1] https://github.com/boostorg/optional/commit/7541076cf1c95201158e47de10f0c8222fb02a70
              // [2] https://github.com/boostorg/range/commit/1b4f8100efab5f3b7d9886c3bdfe309b2c500f4e
                        boost::function<std::pair<std::string, Future<unsigned int>>(const ServiceInfo& serviceInfo)>
#endif
                            ([&](const ServiceInfo& serviceInfo) {
                  const auto name = serviceInfo.name();
                  return std::make_pair(name,
                                        mirrorServiceFromSDUnsync(serviceInfo.serviceId(), name));
                }));
              return FutureMaybeMirroredIdMap{ begin(mirroredIds), end(mirroredIds) };
            })).unwrap();
      }).unwrap().andThen(&logAnyMirroringFailure);
}

Future<unsigned int> ServiceDirectoryProxy::Impl::mirrorServiceFromSDUnsync(
  unsigned int remoteId,
  const std::string& name)
{
  if (const auto mirroringFailure = immediateMirroringFailureUnsync(name))
    return makeFutureError<unsigned int>(*mirroringFailure);

  if (!shouldMirrorServiceFromSDUnsync(name))
  {
    qiLogVerbose() << "Service '" << name << "' should not be mirrored, skipping.";
    return makeFutureError<unsigned int>(skippedErrorMsg);
  }

  QI_ASSERT_FALSE(_server->endpoints().empty());

  return invokeLogProgress("Mirroring service '" + name + "' from the service directory", [&]{
    const auto idFut = mirrorServiceUnsync(name, _sdClient, _server, "service directory", "proxy");
    _servicesIdMap[name] = MirroredFromServiceDirectoryServiceId{ idFut, remoteId };
    return idFut;
  });
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
                              << ") was just registered on service directory.";
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
                              << ") was just unregistered from service directory.";
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
                   qiLogVerbose()
                    << "The connection to the service directory has been lost (reason: '"
                    << reason << "').";
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
  qiLogVerbose() << "Resetting.";
  closeUnsync();
  doAttachUnsync();
}

bool ServiceDirectoryProxy::Impl::shouldMirrorServiceFromSDUnsync(const std::string& name) const
{
  return name != Session::serviceDirectoryServiceName() && !_serviceFilter(name);
}

Future<unsigned int> ServiceDirectoryProxy::Impl::mirrorServiceToSDUnsync(
  unsigned int localId,
  const std::string& name)
{
  if (const auto mirroringFailure = immediateMirroringFailureUnsync(name))
    return makeFutureError<unsigned int>(*mirroringFailure);

  auto listenFut = futurize();
  if (_sdClient->endpoints().empty())
  {
    listenFut = invokeLogProgress(
      "Making service directory client session listen on localhost before registering service to "
      "it",
      [&] { return _sdClient->listen("tcp://localhost:0").async(); });
  }

  return listenFut.andThen(_strand.unwrappedSchedulerFor([=](void*) {
    return invokeLogProgress("Mirroring service '" + name + "' to the service directory", [&]{
      const auto idFut = mirrorServiceUnsync(name, _server, _sdClient, "proxy", "service directory");
      _servicesIdMap[name] = MirroredFromProxyServiceId{ localId, idFut };
      return idFut;
    });
  })).unwrap();
}

Future<void> ServiceDirectoryProxy::Impl::unmirrorServiceToSDUnsync(const std::string& name)
{
  if (!_sdClient)
    return makeFutureError<void>(notConnectedUnmirrorErrorMsg);

  const auto serviceIdIt = _servicesIdMap.find(name);
  if (serviceIdIt == end(_servicesIdMap))
    return makeFutureError<void>("could not find the mirrored service id");

  const auto& serviceIdVariant = serviceIdIt->second;
  const auto* serviceId = boost::get<MirroredFromProxyServiceId>(&serviceIdVariant);
  if (!serviceId)
    return makeFutureError<void>("the service was not registered locally");

  return ka::invoke_catch(
    futureErrorFromException<void>(),
    [&] {
      auto scopeErase = ka::scoped([&]{ _servicesIdMap.erase(serviceIdIt); });
      const auto localId = serviceId->local;
      auto remoteId = serviceId->remote;
      // Cancel any potentially running mirroring procedure.
      remoteId.cancel();
      return remoteId.andThen(_strand.unwrappedSchedulerFor([=](unsigned int remoteId) {
        qiLogVerbose() << "Service being unmirrored has local id " << localId << " and remote id "
                       << remoteId << ".";
        return invokeLogProgress(
          "Unregistering service '" + name + "' to the service directory",
          [&]{
            return _sdClient->unregisterService(remoteId).async();
          });
      })).unwrap();
    });
}

boost::optional<std::string> ServiceDirectoryProxy::Impl::immediateMirroringFailureUnsync(
  const std::string& name) const
{
  // Verify that the service is not already mirrored.
  if (_servicesIdMap.find(name) != _servicesIdMap.end())
  {
    qiLogVerbose() << "Service '" << name << "' is already mirrored, skipping.";
    return skippedErrorMsg;
  }

  if (!_status.current().isListening())
  {
    qiLogVerbose() << notListeningMirrorErrorMsg;
    return notListeningMirrorErrorMsg;
  }
  QI_ASSERT_TRUE(_server);

  if (!_status.current().isConnected())
  {
    qiLogVerbose() << notConnectedMirrorErrorMsg;
    return notConnectedMsgPrefix;
  }
  QI_ASSERT_TRUE(_sdClient);

  return {};
}

Future<void> ServiceDirectoryProxy::Impl::unmirrorServiceFromSDUnsync(const std::string& name)
{
  if (!_server)
    return makeFutureError<void>(notListeningUnmirrorErrorMsg);

  const auto serviceIdIt = _servicesIdMap.find(name);
  if (serviceIdIt == end(_servicesIdMap))
    return makeFutureError<void>("could not find the mirrored service id");

  const auto& serviceIdVariant = serviceIdIt->second;
  const auto* serviceId = boost::get<MirroredFromServiceDirectoryServiceId>(&serviceIdVariant);
  if (!serviceId)
    return makeFutureError<void>(
      "the service was not registered on the service directory first");

  return ka::invoke_catch(
    futureErrorFromException<void>(),
    [&] {
      auto scopeErase = ka::scoped([&]{ _servicesIdMap.erase(serviceIdIt); });
      const auto remoteId = serviceId->remote;
      auto localId = serviceId->local;
      // Cancel any potentially running mirroring procedure.
      localId.cancel();
      return localId.andThen(_strand.unwrappedSchedulerFor([=](unsigned int localId) {
          qiLogVerbose() << "Service being unmirrored has local id " << localId << " and remote id "
                         << remoteId << ".";
          return invokeLogProgress(
            "Unregistering service '" + name + "' from the service directory",
            [&] {
              return _server->unregisterService(localId).async();
            });
        })).unwrap();
    });
}

Future<unsigned int> ServiceDirectoryProxy::Impl::mirrorServiceUnsync(
  const std::string& name,
  const SessionPtr& srcSess,
  const SessionPtr& destSess,
  const std::string& srcDesc,
  const std::string& destDesc)
{
  QI_ASSERT_NOT_NULL(srcSess);
  QI_ASSERT_NOT_NULL(destSess);

  const std::string sessionResetMessage = "the session was reset";

  auto weakSrcSess = ka::weak_ptr(srcSess);
  const auto getServ = [=] {
    auto session = weakSrcSess.lock();
    if (!session)
      return makeFutureError<AnyObject>(sessionResetMessage);
    return invokeLogProgress("Getting service '" + name + "' from " + srcDesc,
                             [&] { return session->service(name).async(); });
  };

  auto weakDestSess = ka::weak_ptr(destSess);
  const auto registerServ = [=](AnyObject service) {
    auto session = weakDestSess.lock();
    if (!session)
      return makeFutureError<unsigned int>(sessionResetMessage);
    return invokeLogProgress(
      "Registering service '" + name + "' on " + destDesc,
      [&] { return session->registerService(name, service).async(); });
  };

  const auto mirror = [=] {
    return getServ()
      .andThen(_strand.unwrappedSchedulerFor(registerServ))
      .unwrap();
  };

  return ka::invoke_catch(futureErrorFromException<unsigned int>(), [&] {
    return repeatWhileError(mirror, _strand,
                            "get and register service '" + name + "'",
                            retryServiceDelay);
  });
}

SessionPtr ServiceDirectoryProxy::Impl::createServerUnsync()
{
  using ka::scoped;
  auto server = makeSession(_isEnforcedAuth);

  if (_identity && !server->setIdentity(_identity->key, _identity->crt))
  {
    throw std::runtime_error{ "Invalid identity parameters : key: '" + _identity->key +
                              "', crt: '" + _identity->crt + "'." };
  }
  server->setAuthProviderFactory(placeholderIfNull(_authProviderFactory));

  bool connectSucceeded = false;

  auto scopedDisconnectServiceRegistered =
    scoped(server->serviceRegistered.connect(
             _strand.schedulerFor([=](unsigned int localId, const std::string& name) {
               return mirrorServiceToSDUnsync(localId, name);
             })),
           [&](const SignalSubscriber& sigSub) {
             if (!connectSucceeded)
               _server->serviceRegistered.disconnect(sigSub.link());
           });

  auto scopedDisconnectServiceUnregistered =
    scoped(server->serviceUnregistered.connect(
             _strand.schedulerFor([=](unsigned int, const std::string& name) {
               return unmirrorServiceToSDUnsync(name);
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
  }
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
  }
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
  }
  return out;
}

}
