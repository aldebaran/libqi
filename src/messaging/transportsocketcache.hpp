#pragma once
/*
**  Copyright (C) 2015 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSOCKETCACHE2_HPP_
#define _SRC_TRANSPORTSOCKETCACHE2_HPP_

#include <string>
#include <queue>

#include <boost/thread/mutex.hpp>
#include <boost/thread/synchronized_value.hpp>

#include <qi/future.hpp>
#include <qi/messaging/serviceinfo.hpp>

#include <qi/trackable.hpp>

#include "messagesocket.hpp"

namespace qi
{

  /**
  * @brief The TransportCache class maintain a cache of TransportSocket
  * @internal
  *
  * `socket` will return a connected endpoint for the associated endpoint.
  *
  * -> if the endpoint is already connected return it.
  * -> if the connection is pending wait for the result
  * -> if the socket do not exist, create it, and try to connect it
  * -> if the socket is disconnected try to reconnect it
  */

  class TransportSocketCache : public Trackable<TransportSocketCache>
  {
  public:
    TransportSocketCache();
    ~TransportSocketCache();

    void init();
    void close();

    /// Get the socket for the given ServiceInfo.
    /// The original url of the service directory is always preferred to the
    /// other endpoints, and other endpoints will not be tried if the services
    /// are running on another machine.
    /// @param servInfo A service info retrieved from a service directory.
    /// @param sdUrl The endpoint of the service directory on which the service info came from.
    Future<MessageSocketPtr> socket(const ServiceInfo& servInfo, const std::string& sdUrl);
    void insert(const std::string& machineId, const Url& url, MessageSocketPtr socket);

    /// The returned future is set when the socket has been disconnected and
    /// effectively removed from the cache.
    FutureSync<void> disconnect(MessageSocketPtr socket);
  private:
    enum State
    {
      State_Pending,
      State_Connected,
      State_Error
    };

    using UrlVectorPtr = boost::shared_ptr<UrlVector>;
    void onSocketConnectionAttempt(Future<void> fut, Promise<MessageSocketPtr> prom, MessageSocketPtr socket, const ServiceInfo& info, uint32_t currentUrlIdx, UrlVectorPtr urls);
    void onSocketParallelConnectionAttempt(Future<void> fut, MessageSocketPtr socket, Url url, const ServiceInfo& info);
    void onSocketDisconnected(Url url, const ServiceInfo& info);


    boost::mutex _socketMutex;
    struct ConnectionAttempt
    {
      ConnectionAttempt() noexcept;
      ~ConnectionAttempt();

      ConnectionAttempt(const ConnectionAttempt&) = delete;
      ConnectionAttempt& operator=(const ConnectionAttempt&) = delete;

      Promise<MessageSocketPtr> promise;
      MessageSocketPtr endpoint;
      UrlVector relatedUrls;
      int attemptCount = 0;
      State state = State_Pending;
      SignalLink disconnectionTracking = SignalBase::invalidSignalLink;
    };
    using ConnectionAttemptPtr = boost::shared_ptr<ConnectionAttempt>;

    void checkClear(ConnectionAttemptPtr, const std::string& machineId);

    /// The promise is set when the `disconnected` signal of `socket` has been received.
    struct DisconnectInfo
    {
      MessageSocketPtr socket;
      Promise<void> promiseSocketRemoved;
    };

    using MachineId = std::string;
    using ConnectionMap = std::map<MachineId, std::map<Url, ConnectionAttemptPtr>>;
    ConnectionMap _connections;
    std::list<MessageSocketPtr> _allPendingConnections;
    boost::synchronized_value<std::vector<DisconnectInfo>> _disconnectInfos;
    bool _dying;
  };
}

#endif
