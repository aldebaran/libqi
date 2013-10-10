#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSOCKETCACHE_HPP_
#define _SRC_TRANSPORTSOCKETCACHE_HPP_

#include <boost/thread/mutex.hpp>
#include <qi/future.hpp>
#include <qimessaging/serviceinfo.hpp>
#include "transportsocket.hpp"
#include <string>

namespace qi {

  struct TransportSocketConnection {
    qi::Url                             url;
    qi::TransportSocketPtr              socket;
    qi::Promise<qi::TransportSocketPtr> promise;
    SignalLink                          connectSignalLink;
    SignalLink                          disconnectSignalLink;
  };

  struct TransportSocketConnectionAttempt {
    qi::Promise<qi::TransportSocketPtr> promise;
    unsigned int socket_count;
    bool successful;
  };

  /**
   * @brief The TransportCache class maintain a cache of TransportSocket
   * @internal
   *
   * getSocket will return a connected endpoint for the associated endpoint.
   *
   * -> if the endpoint is already connected return it.
   * -> if the connection is pending wait for the result
   * -> if the socket do not exist, create it, and try to connect it
   * -> if the socket is disconnected try to reconnect it
   */
  class TransportSocketCache {
  public:
    ~TransportSocketCache();

    void init();
    void close();

    qi::Future<qi::TransportSocketPtr> socket(const ServiceInfo& servInfo, const std::string protocol);
    void insert(const std::string& machineId, const Url& url, TransportSocketPtr socket);
  protected:
    //TransportSocket
    void onSocketConnected(TransportSocketPtr client, const ServiceInfo &servInfo, const Url &url);
    void onSocketDisconnected(std::string error, TransportSocketPtr client, const ServiceInfo &servInfo, const Url& url);

  private:
    //maintain a cache of remote connections
    typedef std::map< std::string, TransportSocketConnection > TransportSocketConnectionMap;
    typedef std::map< std::string, TransportSocketConnectionMap > MachineConnectionMap;
    typedef std::map< std::string, TransportSocketConnectionAttempt > MachineAttemptsMap;

    bool _dying;
    boost::mutex _socketsMutex;

    MachineAttemptsMap _attempts;
    MachineConnectionMap _sockets;
  };
}

#endif  // _SRC_TRANSPORTSOCKETCACHE_HPP_
