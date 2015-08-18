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

#include <qi/future.hpp>
#include <qi/messaging/serviceinfo.hpp>

#include <qi/trackable.hpp>

#include "transportsocket.hpp"

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

    Future<TransportSocketPtr> socket(const ServiceInfo& servInfo, const std::string& protocol = "");
    void insert(const std::string& machineId, const Url& url, TransportSocketPtr socket);

  private:

    enum State
    {
      State_Pending,
      State_Connected,
      State_Error
    };

    typedef boost::shared_ptr<UrlVector> UrlVectorPtr;
    void onSocketConnectionAttempt(Future<void> fut, Promise<TransportSocketPtr> prom, TransportSocketPtr socket, const ServiceInfo& info, uint32_t currentUrlIdx, UrlVectorPtr urls);
    void onSocketParallelConnectionAttempt(Future<void> fut, TransportSocketPtr socket, Url url, const ServiceInfo& info);
    void onSocketDisconnected(TransportSocketPtr client, Url url, const std::string& reason, const ServiceInfo& info);


    boost::mutex _socketMutex;
    struct ConnectionAttempt {
      Promise<TransportSocketPtr> promise;
      TransportSocketPtr endpoint;
      UrlVector relatedUrls;
      int attemptCount;
      State state;
    };
    typedef boost::shared_ptr<ConnectionAttempt> ConnectionAttemptPtr;

    void checkClear(ConnectionAttemptPtr, const std::string& machineId);

    typedef std::string MachineId;
    typedef std::map<MachineId, std::map<Url, ConnectionAttemptPtr> > ConnectionMap;
    ConnectionMap _connections;
    std::list<TransportSocketPtr> _allPendingConnections;
    bool _dying;
  };
}

#endif
