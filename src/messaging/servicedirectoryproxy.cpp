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
#include <qi/messaging/tcpscheme.hpp>
#include <qi/url.hpp>
#include <qi/macro.hpp>
#include <qi/future.hpp>

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

const std::string notListeningUnmirrorErrorMsg = notListeningMsgPrefix
                                                 + cannotUnmirrorServiceMsgSuffix;
const std::string notConnectedUnmirrorErrorMsg = notConnectedMsgPrefix
                                                 + cannotUnmirrorServiceMsgSuffix;

using FutureMaybeMirroredIdMap = boost::container::flat_map<std::string, Future<unsigned int>>;

void logAnyMirroringFailure(const FutureMaybeMirroredIdMap& ids)
{
  FutureBarrier<unsigned int> barrier;
  for (const auto& id : ids)
    barrier.addFuture(id.second);
  // Wait for all futures to end then log if failures occurred.
  barrier.future().then([ids](const Future<std::vector<Future<unsigned int>>>&) {
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
  });
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
          << "msec (last operation failed: " << fut.error() << ").";
        return strand.asyncDelay(
            [=, &strand] {
              return repeatWhileError(op, strand, opDesc, delay);
            },
            delay).unwrap();
      }
      return fut;
    })).unwrap();
}

/// Enables printing containers elements in an output stream, with the following format:
///   `[e1, e2, ..., en]`
template<typename C>
struct Prettified
{
  const C* c;
  friend std::ostream& operator<<(std::ostream& os, const Prettified& p)
  {
    if (p.c == nullptr)
      return os << "<null>";
    os << "[";
    bool delim = false;
    for (const auto& v : *p.c)
    {
      if (delim)
        os << ", ";
      else
        delim = true;
      os << v;
    }
    os << "]";
    return os;
  }
};

template<typename C>
Prettified<C> prettified(const C* c)
{
  return { c };
}

/// Procedure<_(std::ostream&)> P
template<typename P>
boost::optional<std::string> unexpectedState(Property<ServiceDirectoryProxy::Status>& property,
                                             ServiceDirectoryProxy::Status expected,
                                             P logAction)
{
  const auto currentStatus = property.get().value();
  if (currentStatus != expected)
  {
    std::ostringstream oss;
    oss << "unexpected proxy state before ";
    logAction(oss);
    oss << ": expected state " << expected << " but current state is "
        << currentStatus;
    return oss.str();
  }
  return {};
}

/// Constructs the configuration of the service directory client according to
/// the configuration of the proxy.
Session::Config sdClientConfig(const ServiceDirectoryProxy::Config& cfg)
{
  Session::Config sessCfg;
  sessCfg.connectUrl = cfg.serviceDirectoryUrl;
  sessCfg.clientAuthenticatorFactory = cfg.clientAuthenticatorFactory;
  sessCfg.clientSslConfig = cfg.clientSslConfig;
  return sessCfg;
}

/// Constructs the configuration of the server according to the configuration
/// of the proxy.
Session::Config serverConfig(const ServiceDirectoryProxy::Config& cfg)
{
  Session::Config sessCfg;
  sessCfg.listenUrls = cfg.listenUrls;
  sessCfg.authProviderFactory = cfg.authProviderFactory;
  sessCfg.serverSslConfig = cfg.serverSslConfig;
  return sessCfg;
}

} // anonymous namespace

// To implement the proxy, we use a session that listens and provides endpoints for connections and
// another session that connects to the service directory. The first session is referenced as
// the 'server' and the second is referenced as the 'service directory client'.
class ServiceDirectoryProxy::Impl
{
public:
  using Config = ServiceDirectoryProxy::Config;
  static const Seconds retryAttachDelay;
  static const MilliSeconds retryServiceDelay;

  explicit Impl(Config config, Promise<void> ready);
  ~Impl();

  Future<UrlVector> endpoints() const;

private:
  // Precondition synchronized():
  // Concurrent calls of functions with the same precondition on the same object must be sequenced.

  // Precondition: synchronized()
  //
  // Resets the state of the proxy and launches a new starting sequence.
  // Returns a future set once the proxy is connected to the service directory and
  // it started listening on its endpoints.
  Future<void> restartUnsync();

  // Precondition: synchronized()
  //
  // Launches the server of the proxy, and returns a future set once the server is
  // listening on its endpoints.
  Future<void> listenUnsync();

  // Precondition: synchronized()
  Future<void> mirrorAllServicesUnsync();

  // Precondition: synchronized()
  Future<unsigned int> mirrorServiceFromSDUnsync(unsigned int remoteId,
                                                 const std::string& name);

  // Precondition: synchronized() && _sdClient
  //
  // Binds to a service directory by connecting to its signals.
  void bindToServiceDirectoryUnsync();

  // Precondition: synchronized()
  Future<void> unmirrorServiceFromSDUnsync(const std::string& service);

  // Precondition: synchronized() && _config.serviceDirectoryUrl.isValid()
  //
  // Attaches the proxy to a service directory at `_config.serviceDirectoryUrl`. Returns a future set
  // when the connection is successful.
  Future<void> connectUnsync();

  // Precondition: synchronized()
  //
  // Instantiates a server (as a session) and initializes it with the current state of the proxy.
  // Returns the pointer the new server.
  SessionPtr createServerUnsync();

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

private:
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

  mutable Strand _strand;

public:
  Property<Status> status;

private:
  const Config _config;
  SessionPtr _server; // ptr because we have to recreate it every time we listen
  SessionPtr _sdClient;
  MirroredServiceIdMap _servicesIdMap;
};

const Seconds ServiceDirectoryProxy::Impl::retryAttachDelay { 1 };
const MilliSeconds ServiceDirectoryProxy::Impl::retryServiceDelay { 500 };

ServiceDirectoryProxy::ServiceDirectoryProxy(Config config, Promise<void> ready)
  : _p(new Impl(std::move(config), std::move(ready)))
{
}

namespace
{
  void throwRuntimeError(const ssl::Error& e, const char* prefix, const boost::filesystem::path& p)
  {
    std::ostringstream oss;
    oss << prefix << "'" << p << "'. Details: " << e.what();
    throw std::runtime_error(oss.str());
  }

  void throwRuntimeError(const char* prefix, const boost::filesystem::path& p)
  {
    std::ostringstream oss;
    oss << prefix << "'" << p << "'.";
    throw std::runtime_error(oss.str());
  }

  // Throws a `std::runtime_error` iff reading the file failed or no
  // certificate was found in it.
  std::vector<ssl::Certificate> readCertChain(const boost::filesystem::path& p)
  {
    auto certs = std::vector<ssl::Certificate>{};
    try
    {
      certs = ssl::Certificate::chainFromPemFile(p);
    }
    catch (const ssl::Error& e)
    {
      throwRuntimeError(e, "Could not read certificate chain from file ", p);
    }
    if (certs.empty())
    {
      throwRuntimeError("No certificate found in file ", p);
    }
    return certs;
  }

  // Throws a `std::runtime_error` iff reading the file failed or no
  // private key was found in it.
  ssl::PKey readPrivateKey(const boost::filesystem::path& p)
  {
    auto optPrivateKey = boost::optional<ssl::PKey>{};
    try
    {
      optPrivateKey = ssl::PKey::privateFromPemFile(p);
    }
    catch (const ssl::Error& e)
    {
      throwRuntimeError(e, "Could not read private key from file ", p);
    }
    if (! optPrivateKey.has_value())
    {
      throwRuntimeError("No private key found in file ", p);
    }
    return optPrivateKey.value();
  };

  // Returns the certificate read in the given file path, or throws a
  // `std::runtime_error` iff reading failed or no certificate was found in it.
  // Factorizing with `readPrivateKey` does not worth the trouble.
  ssl::Certificate readCert(const boost::filesystem::path& p)
  {
    auto optCert = boost::optional<ssl::Certificate>{};
    try
    {
      optCert = ssl::Certificate::fromPemFile(p);
    }
    catch (const ssl::Error& e)
    {
      throwRuntimeError(e, "Could not read certificate from file ", p);
    }
    if (! optCert.has_value())
    {
      throwRuntimeError("No certificate found in file ", p);
    }
    return optCert.value();
  }
} // anonymous namespace

ServiceDirectoryProxy::Config ServiceDirectoryProxy::Config::createFromListeningProtocol(
    Url sdUrl,
    Url listenUrl,
    ServiceDirectoryProxy::FilterService filterService,
    ServiceDirectoryProxy::Filepaths filepaths,
    boost::optional<boost::shared_ptr<AuthProviderFactory>> authProviderFactory
  )
{
  auto throwError = [](std::string message) {
    throw std::invalid_argument(ka::mv(message));
  };
  auto optScheme = tcpScheme(listenUrl);
  if (! optScheme.has_value())
  {
    throwError("Unsupported protocol. Scheme = " + listenUrl.protocol());
  }
  auto config = ServiceDirectoryProxy::Config{
    ka::mv(sdUrl),
    std::vector<Url>{ listenUrl },
    ka::mv(authProviderFactory),
    boost::optional<ClientAuthenticatorFactoryPtr>{},
    ka::mv(filterService),
    ssl::ClientConfig{},
    ssl::ServerConfig{}
  };
  switch (optScheme.value())
  {
  case TcpScheme::Raw: {
    const auto hasFilepaths = boost::get<ka::unit_t>(&filepaths) == nullptr;
    if (hasFilepaths)
    {
      throwError("Extra data: TLS or mTLS file paths provided while listening with TCP.");
    }
    break;
  }
  case TcpScheme::Tls: {
    const auto* paths = boost::get<ServiceDirectoryProxy::TlsFilepaths>(&filepaths);
    if (paths == nullptr)
    {
      throwError("Missing data: TLS file paths.");
    }
    // Call functions that might throw out of aggregate initialization of struct to
    // prevent a bug from GCC (see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66139).
    auto certChain = readCertChain(paths->certChain);
    auto privateKey = readPrivateKey(paths->privateKey);
    config.serverSslConfig.certWithPrivKey = ssl::CertChainWithPrivateKey{
      std::move(certChain),
      std::move(privateKey)
    };
    break;
  }
  case TcpScheme::MutualTls: {
    const auto* paths = boost::get<ServiceDirectoryProxy::MTlsFilepaths>(&filepaths);
    if (paths == nullptr)
    {
      throwError("Missing data: mTLS file paths.");
    }
    // Call functions that might throw out of aggregate initialization of struct to
    // prevent a bug from GCC (see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66139).
    auto certChain = readCertChain(paths->certChain);
    auto privateKey = readPrivateKey(paths->privateKey);
    config.serverSslConfig.certWithPrivKey = ssl::CertChainWithPrivateKey{
      std::move(certChain),
      std::move(privateKey)
    };
    config.serverSslConfig.trustStore = { readCert(paths->trustedCert) };
    config.serverSslConfig.verifyPartialChain = true; // For certificate pinning.
    break;
  }
  default:
    throwError("Unsupported protocol. Scheme = " + listenUrl.protocol());
  }
  return config;
}

Future<ServiceDirectoryProxyPtr> ServiceDirectoryProxy::createFromListeningProtocol(
    Url sdUrl,
    Url listenUrl,
    ServiceDirectoryProxy::FilterService filterService,
    ServiceDirectoryProxy::Filepaths filepaths,
    boost::optional<boost::shared_ptr<AuthProviderFactory>> authProviderFactory
  )
{
  return ServiceDirectoryProxy::create(
    Config::createFromListeningProtocol(
      ka::mv(sdUrl),
      ka::mv(listenUrl),
      ka::mv(filterService),
      ka::mv(filepaths),
      ka::mv(authProviderFactory)
    )
  );
}

Future<ServiceDirectoryProxyPtr> ServiceDirectoryProxy::create(Config config)
{
  // The cancel callback for this promise is set by the proxy constructor.
  Promise<void> readyPromise;
  auto ready = readyPromise.future();
  ServiceDirectoryProxyPtr proxy(
    new ServiceDirectoryProxy(std::move(config), std::move(readyPromise)));
  return ready.andThen([proxy](void*) mutable { return std::move(proxy); });
}

ServiceDirectoryProxy::~ServiceDirectoryProxy() = default;

Property<ServiceDirectoryProxy::Status>& ServiceDirectoryProxy::status()
{
  return _p->status;
}

const Property<ServiceDirectoryProxy::Status>& ServiceDirectoryProxy::status() const
{
  return _p->status;
}

UrlVector ServiceDirectoryProxy::endpoints() const
{
  return _p->endpoints().value();
}

ServiceDirectoryProxy::Impl::Impl(Config config, Promise<void> readyPromise)
  : status{ Status::Initializing, _strand, Property<Status>::Getter{}, util::SetAndNotifyIfChanged{}}
  , _config{ std::move(config) }
{
  if (!_config.serviceDirectoryUrl.isValid())
  {
    readyPromise.setError("service directory proxy: invalid service directory url '" +
                          _config.serviceDirectoryUrl.str() + "'");
    return;
  }

  const auto listenUrlEnd = _config.listenUrls.end();
  const auto invalidListenUrlIt = std::find_if(_config.listenUrls.begin(), listenUrlEnd,
                                               [](const Url& url) { return !url.isValid(); });
  if (invalidListenUrlIt != listenUrlEnd)
  {
    readyPromise.setError("service directory proxy: invalid listen URL '" +
                          invalidListenUrlIt->str() + "'");
    return;
  }

  auto start = restartUnsync();
  start.then(_strand.schedulerFor([readyPromise](Future<void> start) mutable {
      adaptFuture(start, readyPromise);
    }));

  readyPromise.setOnCancel(
    [start](Promise<void>&) mutable
    {
      start.cancel();
      // The promise will be set by the continuation of the start future.
    });
}

ServiceDirectoryProxy::Impl::~Impl()
{
  ka::invoke_catch(
    exceptionLogWarning(category,
                        "Exception caught during destruction of implementation class: "),
    [&] {
      _strand.join();
    });
}

Future<UrlVector> ServiceDirectoryProxy::Impl::endpoints() const
{
  return _strand.async([&] { return _server ? _server->endpoints() : UrlVector{}; });
}

Future<void> ServiceDirectoryProxy::Impl::restartUnsync()
{
  qiLogVerbose() << "Resetting.";
  _sdClient.reset();
  _server.reset();
  qiLogVerbose() << "Clearing the list of known services.";
  _servicesIdMap.clear();
  qiLogVerbose() << "Starting.";

  const auto setReady =
    _strand.unwrappedSchedulerFor([this](void*) {
      // There is a probability that the connection to the service directory
      // is lost before the `listen` future finished. This would normally be racy, but this
      // callback is executed from inside the strand, and so are the property accessors.
      const auto stateError =
        unexpectedState(status, Status::Connected,
                        [](std::ostream& os) { os << "changing state to " << Status::Ready; });
      if (stateError)
        return makeFutureError<void>(*stateError);
      status.set(Status::Ready);
      // Run the mirroring asynchronously, do not return the future.
      // The proxy is considered started and ready before the services are mirrored.
      mirrorAllServicesUnsync();
      return futurize();
    });

  const auto listen =
    _strand.unwrappedSchedulerFor([this, setReady](void*) {
      const auto stateError =
        unexpectedState(status, Status::Connecting,
                        [](std::ostream& os) { os << "changing state to " << Status::Connected; });
      if (stateError)
        return makeFutureError<void>(*stateError);
      status.set(Status::Connected);
      return listenUnsync();
    });

  status.set(Status::Connecting);

  // Connect then listen.
  return connectUnsync()
    .andThen(FutureCallbackType_Sync, listen).unwrap()
    .andThen(FutureCallbackType_Sync, setReady).unwrap();
}

Future<void> ServiceDirectoryProxy::Impl::listenUnsync()
{
  const auto stateError =
    unexpectedState(status, Status::Connected,
                    [](std::ostream& os) { os << "a call to listen"; });
  if (stateError)
  {
    qiLogVerbose() << "Listen failure: " << *stateError;
    return makeFutureError<void>(*stateError);
  }

  constexpr auto errorMsgPrefix =
    "Exception caught while trying to instantiate the server, reason: ";

  // The optional represents a potential error (here, there is none).
  // This is for invocation through `invoke_catch`.
  const auto instantiateServerSession = [&]() -> boost::optional<std::string> {
    qiLogVerbose() << "Instantiating server session.";
    _servicesIdMap.clear();
    _server.reset();
    _server = createServerUnsync();
    return {};
  };
  const auto logError = ka::compose(
      [&](const std::string& msg) -> boost::optional<std::string> {
        const auto error = errorMsgPrefix + msg;
        qiLogVerbose() << error;
        return error;
      }, ka::exception_message_t{});
  const auto maybeError = ka::invoke_catch(logError, instantiateServerSession);
  if (maybeError)
  {
    return makeFutureError<void>(*maybeError);
  }

  qiLogVerbose() << "Starting server session listening on URLs '"
                 << prettified(&_config.listenUrls) << "'.";
  return _server->listenStandalone().async();
}

Future<void> ServiceDirectoryProxy::Impl::connectUnsync()
{
  const auto stateError =
    unexpectedState(status, Status::Connecting,
                    [](std::ostream& os) { os << "a call to connect"; });
  if (stateError)
  {
    qiLogVerbose() << "Connect failure: " << *stateError;
    return makeFutureError<void>(*stateError);
  }

  const auto disconnect = [=] {
    _sdClient.reset();
  };

  const auto connect = [=] {
    return invokeLogProgress(
      "Attaching to service directory at URL '" + _config.serviceDirectoryUrl.str() + "'", [&] {
        disconnect();

        qiLogVerbose() << "Instantiating new service directory client session.";
        _sdClient = makeSession(sdClientConfig(_config));

        return _sdClient->connect().async()
          .then(_strand.unwrappedSchedulerFor([=](Future<void> connectFut) {
            if (connectFut.hasError())
            {
              disconnect();
            }
            else
            {
              bindToServiceDirectoryUnsync();
            }
            return connectFut;
          })).unwrap();
      });
  };

  return repeatWhileError(connect, _strand,
                          "attach to the service directory",
                          retryAttachDelay);
}

Future<void> ServiceDirectoryProxy::Impl::mirrorAllServicesUnsync()
{
  const auto stateError =
    unexpectedState(status, Status::Ready,
                    [](std::ostream& os) { os << "mirroring services"; });
  if (stateError)
    return makeFutureError<void>(*stateError);

  QI_ASSERT_NOT_NULL(_sdClient);

  return invokeLogProgress(
      "Mirroring services: requesting list of services from ServiceDirectory",
      [&]{
        return _sdClient->services().async();
      }).andThen(_strand.unwrappedSchedulerFor([=](const std::vector<ServiceInfo>& services) {
          using namespace boost::adaptors;
          const auto mirroredIds =
            services | transformed(([&](const ServiceInfo& serviceInfo) {
              const auto name = serviceInfo.name();
              return std::make_pair(name,
                                    mirrorServiceFromSDUnsync(serviceInfo.serviceId(), name));
            }));
          logAnyMirroringFailure(FutureMaybeMirroredIdMap{ begin(mirroredIds), end(mirroredIds) });
        })).unwrap();
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
  qiLogVerbose() << "Binding to service directory at url '" << _config.serviceDirectoryUrl.str() << "'";

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
                   restartUnsync();
                 })),
             [&](const SignalSubscriber& sigSub) {
               if (!bindingSucceeded)
                 _sdClient->disconnected.disconnect(sigSub.link());
             });

  bindingSucceeded = true;
}

bool ServiceDirectoryProxy::Impl::shouldMirrorServiceFromSDUnsync(const std::string& name) const
{
  return name != Session::serviceDirectoryServiceName() && !_config.filterService(name);
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

  const auto currentStatus = status.get().value();
  if (currentStatus != Status::Ready)
  {
    std::ostringstream oss;
    oss << "proxy state is not " << Status::Ready << ", current state is " << currentStatus;
    const auto error = oss.str();
    qiLogVerbose() << "Cannot mirror service '" << name << "': " << error;
    return error;
  }
  QI_ASSERT_TRUE(_server);
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

  auto server = makeSession(serverConfig(_config));

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

std::ostream& operator<<(std::ostream& out, ServiceDirectoryProxy::Status status)
{
  using Status = ServiceDirectoryProxy::Status;
  switch (status)
  {
    case Status::Initializing: out << "Initializing"; break;
    case Status::Connecting:   out << "Connecting";   break;
    case Status::Connected:    out << "Connected";    break;
    case Status::Ready:        out << "Ready";        break;
    default: printUnexpectedEnumValue(out, status); break;
  }
  return out;
}

}
