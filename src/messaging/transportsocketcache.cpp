/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <algorithm>
#include <sstream>

#include <boost/range/algorithm/find_if.hpp>

#include <qi/log.hpp>
#include <qi/numeric.hpp>
#include <qi/uri.hpp>
#include <qi/messaging/tcpscheme.hpp>

#include "messagesocket.hpp"
#include "transportsocketcache.hpp"

#define LOG_CATEGORY "qimessaging.transportsocketcache"

static constexpr auto noReachableEndpointErrorMessage
  = "No reachable endpoint was found for this service.";

qiLogCategory(LOG_CATEGORY);

namespace qi
{
TransportSocketCache::TransportSocketCache(ssl::ClientConfig sslConfig)
  : _sslConfig(std::move(sslConfig))
  , _dying(false)
{
}

TransportSocketCache::~TransportSocketCache()
{
  _dying = true;
  destroy();
  try
  {
    close();
  }
  catch (const std::exception& ex)
  {
    qiLogError() << "Exception caught during destruction: " << ex.what();
  }
  catch (...)
  {
    qiLogError() << "Unknown exception caught during destruction";
  }
}

void TransportSocketCache::init()
{
  _dying = false;
}

/// Container<DisconnectInfo> C
template<typename C>
static void releaseDisconnectInfoPromises(C& disconnectInfos)
{
  for (auto& d: disconnectInfos)
  {
    d.promiseSocketRemoved.setValue(0);
  }
}

void TransportSocketCache::close()
{
  qiLogDebug() << "TransportSocketCache is closing";
  {
    ConnectionMap map;
    std::list<MessageSocketPtr> pending;
    {
      boost::mutex::scoped_lock lock(_socketMutex);
      _dying = true;
      std::swap(map, _connections);
      std::swap(pending, _allPendingConnections);
    }
    for (auto& pairMachineIdConnection: map)
    {
      auto& mapUriConnection = pairMachineIdConnection.second;
      for (auto& pairUriConnection: mapUriConnection)
      {
        auto& connectionAttempt = *pairUriConnection.second;
        auto endpoint = connectionAttempt.endpoint;

        // Disconnect any valid socket we were holding.
        if (endpoint)
        {
          endpoint->disconnect();
          endpoint->disconnected.disconnect(
            exchangeInvalidSignalLink(connectionAttempt.disconnectionTracking));
        }
        else
        {
          connectionAttempt.state = State_Error;
          connectionAttempt.promise.setError("TransportSocketCache is closing.");
        }
      }
    }
    for (auto& socket: pending)
    {
      socket->disconnect();
    }
  }

  /// Release all disconnect promises to avoid deadlocks.
  /// A deadlock could happen if the user calls `disconnect(socket)` and the cache is
  /// destroyed before the disconnected socket signal has arrived.
  auto sync = _disconnectInfos.synchronize();
  releaseDisconnectInfoPromises(*sync);
}


static std::vector<Uri> localhost_only(const std::vector<Uri>& input)
{
  std::vector<Uri> result;
  result.reserve(input.size());
  for (const auto& uri: input)
  {
    const auto auth = uri.authority();
    if (!auth.empty() && isLoopbackAddress((*auth).host()))
      result.push_back(uri);
  }
  return result;
}

Future<MessageSocketPtr> TransportSocketCache::socket(const ServiceInfo& servInfo)
{
  const std::string& machineId = servInfo.machineId();
  auto couple = boost::make_shared<ConnectionAttempt>();
  couple->relatedUris = servInfo.uriEndpoints();
  bool local = machineId == os::getMachineId();
  std::vector<Uri> connectionCandidates;

  // If the connection is local, we're mainly interested in localhost endpoint
  if (local)
    connectionCandidates = localhost_only(servInfo.uriEndpoints());

  // If the connection isn't local or if the service doesn't expose local endpoints,
  // try and connect to whatever is available.
  if (connectionCandidates.size() == 0)
    connectionCandidates = servInfo.uriEndpoints();

  {
    // If we already have a pending connection to one of the uris, we return the future in question
    boost::mutex::scoped_lock lock(_socketMutex);

    if (_dying)
      return makeFutureError<MessageSocketPtr>("TransportSocketCache is closed.");

    ConnectionMap::iterator machineIt = _connections.find(machineId);
    if (machineIt != _connections.end())
    {
      // Check if any connection to the machine matches one of our uris
      auto& vuris = couple->relatedUris;
      for (const auto& uriConnectPair : machineIt->second)
      {
        const auto candidateUri = uriConnectPair.first;
        const auto connection = uriConnectPair.second;
        if (std::any_of(vuris.begin(), vuris.end(),
                        [&](const Uri& uri) { return uri == candidateUri; }))
        {
          // We found a matching machineId and URI : return the connected endpoint.
          qiLogDebug() << "Found pending promise.";
          return connection->promise.future();
        }
      }
    }
    // Otherwise, we keep track of all those URIs and assign them the same promise in our map.
    // They will all track the same connection.
    couple->attemptCount = qi::numericConvert<int>(connectionCandidates.size());
    auto& uriMap = _connections[machineId];
    for (const auto& uri: connectionCandidates)
    {
      // We only support our TCP schemes for message sockets.
      if (!tcpSchemeFromUriScheme(uri.scheme()))
        continue;

      if (!local && isLoopbackAddress((*uri.authority()).host()))
        continue; // Do not try to connect on localhost when it is a remote!

      uriMap[uri] = couple;
      MessageSocketPtr socket = makeMessageSocket(_sslConfig);
      _allPendingConnections.push_back(socket);
      Future<void> sockFuture = socket->connect(toUrl(uri));
      qiLogDebug() << "Inserted [" << machineId << "][" << uri << "]";
      sockFuture.then(std::bind(&TransportSocketCache::onSocketParallelConnectionAttempt, this,
                                std::placeholders::_1, socket, uri, servInfo));
    }
  }
  return couple->promise.future();
}

FutureSync<void> TransportSocketCache::disconnect(MessageSocketPtr socket)
{
  Promise<void> promiseSocketRemoved;
  {
    auto syncDisconnectInfos = _disconnectInfos.synchronize();
    // TODO: Remove Promise<void>{} when get rid of VS2013.
    syncDisconnectInfos->push_back(DisconnectInfo{socket, Promise<void>{}});
    promiseSocketRemoved = syncDisconnectInfos->back().promiseSocketRemoved;
  }
  // We wait that the socket has been disconnected _and_ the `disconnected`
  // signal has been received by the cache.
  FutureBarrier<void> barrier;
  barrier.addFuture(promiseSocketRemoved.future());
  barrier.addFuture(socket->disconnect());
  Promise<void> promise;
  return barrier.future().andThen([=](const std::vector<Future<void>>& v) mutable {
    const auto isInError = [](const Future<void>& f) {
      return f.hasError();
    };
    if (std::any_of(begin(v), end(v), isInError))
    {
      promise.setError("disconnect error");
      return;
    }
    promise.setValue(0);
  });
}

void TransportSocketCache::insert(const std::string& machineId, const Uri& uri, MessageSocketPtr socket)
{
  // If a connection is pending for this machine / uri, terminate the pendage and set the
  // service socket as this one
  boost::mutex::scoped_lock lock(_socketMutex);

  if (_dying)
    return;

  ServiceInfo info;

  info.setMachineId(machineId);
  qi::SignalLink disconnectionTracking = socket->disconnected.connect(
      track([=](const std::string&) { onSocketDisconnected(uri, info); }, this));

  ConnectionMap::iterator mIt = _connections.find(machineId);
  if (mIt != _connections.end())
  {
    const auto uIt = mIt->second.find(uri);
    if (uIt != mIt->second.end())
    {
      auto& connectionAttempt = *uIt->second;
      QI_ASSERT(!connectionAttempt.endpoint);
      // If the attempt is done and the endpoint is null, it means the
      // attempt failed and the promise is set as error.
      // We replace it by a new one.
      // If the attempt is not done we do not replace it, otherwise the future
      // currently in circulation will never finish.
      if (connectionAttempt.state != State_Pending)
        connectionAttempt.promise = Promise<MessageSocketPtr>();
      connectionAttempt.state = State_Connected;
      connectionAttempt.endpoint = socket;
      connectionAttempt.promise.setValue(socket);
      connectionAttempt.disconnectionTracking = disconnectionTracking;
      return;
    }
  }
  ConnectionAttemptPtr couple = boost::make_shared<ConnectionAttempt>();
  couple->promise = Promise<MessageSocketPtr>();
  couple->endpoint = socket;
  couple->state = State_Connected;
  couple->relatedUris.push_back(uri);
  _connections[machineId][uri] = couple;
  couple->promise.setValue(socket);
}

/*
 * Corner case to manage (TODO):
 *
 * You are connecting to machineId foo, you are machineId bar. foo and bar are
 * on different sub-networks with the same netmask. They sadly got the same IP
 * on their subnet: 192.168.1.42. When trying to connect to foo from bar, we
 * will try to connect its endpoints, basically:
 *   - tcp://1.2.3.4:1333 (public IP)
 *   - tcp://192.168.1.42:1333 (subnet public IP)
 * If bar is listening on port 1333, we may connect to it instead of foo (our
 * real target).
 */
void TransportSocketCache::onSocketParallelConnectionAttempt(Future<void> fut,
                                                             MessageSocketPtr socket,
                                                             Uri uri,
                                                             const ServiceInfo& info)
{
  {
    boost::mutex::scoped_lock lock(_socketMutex);

    if (_dying)
    {
      qiLogDebug() << "ConnectionAttempt: TransportSocketCache is closed";
      if (!fut.hasError())
      {
        _allPendingConnections.remove(socket);
        socket->disconnect();
      }
      return;
    }

    ConnectionMap::iterator machineIt = _connections.find(info.machineId());
    std::map<Uri, ConnectionAttemptPtr>::iterator uriIt;
    if (machineIt != _connections.end())
      uriIt = machineIt->second.find(uri);

    if (machineIt == _connections.end() || uriIt == machineIt->second.end())
    {
      // The socket was disconnected at some point, and we removed it from our map:
      // return early.
      _allPendingConnections.remove(socket);
      socket->disconnect();
      return;
    }

    ConnectionAttemptPtr attempt = uriIt->second;
    attempt->attemptCount--;
    if (attempt->state != State_Pending)
    {
      qiLogDebug() << "Already connected: reject socket " << socket.get() << " endpoint " << uri;
      _allPendingConnections.remove(socket);
      socket->disconnect();
      checkClear(attempt, info.machineId());
      return;
    }
    if (fut.hasError())
    {
      // Failing to connect to some of the endpoint is expected.
      qiLogDebug() << "Could not connect to service #" << info.serviceId() << " through uri " << uri;
      _allPendingConnections.remove(socket);
      // It's a critical error if we've exhausted all available endpoints.
      if (attempt->attemptCount == 0)
      {
        std::stringstream err;
        err << "Could not connect to service #" << info.serviceId() << ": no endpoint replied.";
        qiLogError() << err.str();
        attempt->promise.setError(err.str());
        attempt->state = State_Error;
        checkClear(attempt, info.machineId());
      }
      return;
    }
    qi::SignalLink disconnectionTracking = socket->disconnected.connect(
        track([=](const std::string&) { onSocketDisconnected(uri, info); }, this));
    attempt->state = State_Connected;
    attempt->endpoint = socket;
    attempt->promise.setValue(socket);
    attempt->disconnectionTracking = disconnectionTracking;
  }

  // Associate the same socket to the relative URI of the service, so that we may reuse the same
  // socket if another service has this relative URI as one of its endpoints.
  insert(info.machineId(), *qi::uri(std::string(uriQiScheme()) + ":" + info.name()), socket);
  qiLogDebug() << "Connected to service #" << info.serviceId() << " through uri " << uri
               << " and socket " << socket.get();
}

void TransportSocketCache::checkClear(ConnectionAttemptPtr attempt, const std::string& machineId)
{
  if ((attempt->attemptCount <= 0 && attempt->state != State_Connected) || attempt->state == State_Error)
  {
    ConnectionMap::iterator machineIt = _connections.find(machineId);
    if (machineIt == _connections.end())
      return;
    for (auto uit = attempt->relatedUris.begin(), end = attempt->relatedUris.end(); uit != end;
         ++uit)
      machineIt->second.erase(*uit);
    if (machineIt->second.size() == 0)
      _connections.erase(machineIt);
  }
}

/// Remove infos of the given socket and set the associated promise.
///
/// Container<DisconnectInfo> C
template<typename C>
static void updateDisconnectInfos(C& disconnectInfos, const MessageSocketPtr& socket)
{
  // TODO: Replace `using` by `auto` in lambda when C++14 is available.
  using Value = typename C::value_type;
  const auto it = boost::find_if(disconnectInfos, [&](const Value& d) {
    return d.socket == socket;
  });
  if (it == disconnectInfos.end())
  {
    // We should not fall into this if statement, but due to the racy nature of
    // the disconnection, it does indeed occur. Fixing this would be
    // a significant rearchitecture, and we choose for now to lower the log
    // level because there does not seem to be any other side effect besides the
    // warning.
    qiLogVerbose() << "Disconnected socket not found in disconnect infos.";
    return;
  }
  auto promise = (*it).promiseSocketRemoved;
  disconnectInfos.erase(it);
  promise.setValue(0);
}

void TransportSocketCache::onSocketDisconnected(Uri uri, const ServiceInfo& info)
{
  // remove from the available connections
  boost::mutex::scoped_lock lock(_socketMutex);

  const auto machineId = info.machineId();
  const auto machineIt = _connections.find(machineId);
  if (machineIt == _connections.end())
  {
    qiLogDebug() << "onSocketDisconnected: no socket found for this service info machine ID ("
                 << machineId << ", from service " << info.name() << "/" << info.serviceId()
                 << "), ignoring event.";
    return;
  }
  const auto& uriConnectMap = machineIt->second;
  const auto connectionIt = uriConnectMap.find(uri);
  if (connectionIt == uriConnectMap.end())
  {
    qiLogDebug() << "onSocketDisconnected: no socket found for this service info URI (" << uri
                 << machineId << ", from service " << info.name() << "/" << info.serviceId()
                 << "), ignoring event.";
    return;
  }

  qiLogDebug() << "onSocketDisconnected: about to erase socket";
  auto attempt = machineIt->second[uri];
  attempt->state = State_Error;
  checkClear(attempt, info.machineId());
  auto syncDisconnectInfos = _disconnectInfos.synchronize();
  updateDisconnectInfos(*syncDisconnectInfos, attempt->endpoint);
}


TransportSocketCache::ConnectionAttempt::ConnectionAttempt() noexcept
{
}

TransportSocketCache::ConnectionAttempt::~ConnectionAttempt()
{
  // A connection attempt that failed to terminate means no endpoint was reachable.
  try
  {
    if (promise.future().isRunning())
      promise.setError(noReachableEndpointErrorMessage);
  }
  catch (...)
  {
    qiLogDebug(LOG_CATEGORY ".connectionattempt")
      << "Exception occurred but was ignored when trying to set promise in error in destructor";
  }
}

}
