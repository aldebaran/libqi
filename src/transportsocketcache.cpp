/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "transportsocketcache.hpp"

qiLogCategory("qimessaging.socketcache");

namespace qi {

  TransportSocketCache::~TransportSocketCache() {
    close();
  }


  void TransportSocketCache::init() {
    boost::mutex::scoped_lock sl(_socketsMutex);
    _dying = false;
  }

  void TransportSocketCache::close() {
    {
      _dying = true;
      MachineConnectionMap socketsCopy;
      {
        // Do not hold _socketsMutex while iterating or deadlock may occur
        // between disconnect() that waits for callback handler
        // and callback handler that tries to acquire _socketsMutex
        boost::mutex::scoped_lock sl(_socketsMutex);
        socketsCopy = _sockets;
      }
      MachineConnectionMap::iterator mcmIt;
      for (mcmIt = socketsCopy.begin(); mcmIt != socketsCopy.end(); ++mcmIt) {
        TransportSocketConnectionMap& tscm = mcmIt->second;
        TransportSocketConnectionMap::iterator tscmIt;
        for (tscmIt = tscm.begin(); tscmIt != tscm.end(); ++tscmIt) {
          TransportSocketConnection& tsc = tscmIt->second;
          tsc.socket->disconnected.disconnect(tscmIt->second.disconnectLink);
          tsc.socket->connected.disconnect(tscmIt->second.connectLink);
          //remove callback before calling disconnect. (we dont need them)
          if (tscmIt->second.socket->isConnected())
            tscmIt->second.socket->disconnect();
          tscmIt->second.promise.reset();
          tscmIt->second.promise.setError("session closed");
        }
      }
    }
  }

  qi::Future<qi::TransportSocketPtr> TransportSocketCache::socket(const ServiceInfo& servInfo,
                                                                  const std::string protocol) {
    qi::UrlVector endpoints;
    qi::UrlVector::const_iterator urlIt;

    bool local = servInfo.machineId() == qi::os::getMachineId();
    qiLogDebug() << "local check " << servInfo.machineId() << " " <<  qi::os::getMachineId() << " " << local;
    // RFC 3330 - http://tools.ietf.org/html/rfc3330
    //   -> 127.0.0.0/8 is assigned to loopback address.
    //
    // This filters endpoints. If we are on the same machine, we just try to
    // connect on the loopback address, else we will try on all endpoints we
    // have that are not loopback.
    for (urlIt = servInfo.endpoints().begin(); urlIt != servInfo.endpoints().end(); ++urlIt) {
      qi::Url url = *urlIt;
      qiLogDebug() << "testing url " << url.str();
      if (!url.isValid())
        continue;
      if (url.host().substr(0, 4) == "127." || url.host() == "localhost") {
        if (local && (protocol == "" || url.protocol() == protocol)) {
          endpoints.push_back(url);
          break;
        }
      } else if (!local) {
        endpoints.push_back(url);
      }
    }
    if (endpoints.empty() && local && !servInfo.endpoints().empty())
    { // We are local, but localhost is not listed in endpoints.
      // Just take any entry, it has to be one of our public IP addresses
      endpoints.push_back(servInfo.endpoints().front());
    }
    if (endpoints.empty())
      qiLogWarning() << "No more endpoints available after filtering.";
    {
      boost::mutex::scoped_lock sl(_socketsMutex);
      if (_dying)
        return qi::makeFutureError<qi::TransportSocketPtr>("TransportSocketCache is closed.");

      // From here, we will see if we have a pending/established connection to
      // machineId on one of the endpoints (they all share the same promise
      // anyway). If it is the case, we return its future.
      MachineConnectionMap::iterator mcmIt;
      if ((mcmIt = _sockets.find(servInfo.machineId())) != _sockets.end()) {
        TransportSocketConnectionMap& tscm = mcmIt->second;
        TransportSocketConnectionMap::iterator tscmIt;
        for (urlIt = endpoints.begin(); urlIt != endpoints.end(); ++urlIt) {
          if ((tscmIt = tscm.find((*urlIt).str())) != tscm.end()) {
            TransportSocketConnection& tsc = tscmIt->second;
            try
            {
              if (tsc.promise.future().isFinished() &&
                  tsc.promise.future().hasError(0)) {
                // When we have a socket with an error, we will try to connect to
                // all endpoints in case the old one is completely down.
                continue;
              }
            }
            catch (...)
            {
              continue;
            }

            qiLogVerbose() << "A connection is pending or already"
                                       << " established.";
            return tsc.promise.future();
          }
        }
      }

      // Launching connections to all endpoints at the same time. They all share
      // the same promise.
      qi::Promise<qi::TransportSocketPtr> prom;
      if (endpoints.empty())
      {
        prom.setError("No endpoint available.");
        return prom.future();
      }
      // We will need this to report error (to know if all sockets didn't
      // connect).
      TransportSocketConnectionAttempt& tsca = _attempts[servInfo.machineId()];
      tsca.promise = prom;
      tsca.socket_count = 0;
      tsca.successful = false;

      // This part launches all the socket connections on the same promise. The
      // first socket to connect is the winner.
      TransportSocketConnectionMap& tscm = _sockets[servInfo.machineId()];
      for (urlIt = endpoints.begin(); urlIt != endpoints.end(); ++urlIt) {
        qi::Url url = *urlIt;
        if (protocol != "" && protocol != url.protocol())
        {
          continue;
        }

        qi::TransportSocketPtr socket = makeTransportSocket(url.protocol());
        TransportSocketConnection& tsc = tscm[url.str()];
        qiLogVerbose() << "Attempting connection to " << url.str()
                                   << " of machine id " << servInfo.machineId();
        tsc.socket = socket;
        tsc.promise = prom;
        tsc.url = url;
        tsc.connectLink = socket->connected.connect(boost::bind(&TransportSocketCache::onSocketConnected, this, socket, servInfo, url));
        tsc.disconnectLink = socket->disconnected.connect(boost::bind(&TransportSocketCache::onSocketDisconnected, this, _1, socket, servInfo, url));
        socket->connect(url).async();
        tsca.socket_count++;
      }
      return prom.future();
    } // ! boost::mutex::scoped_lock
  }

  void TransportSocketCache::onSocketDisconnected(std::string error, TransportSocketPtr socket, const qi::ServiceInfo& servInfo, const qi::Url& url) {
    {
      boost::mutex::scoped_lock sl(_socketsMutex);

      // First, we get the attempts of the machineId. It is used to know if we
      // have pending connections to other endpoints.
      MachineAttemptsMap::iterator mamIt;
      if ((mamIt = _attempts.find(servInfo.machineId())) == _attempts.end()) {
        // Unknown error. This shouldn't happen...
        return;
      }
      TransportSocketConnectionAttempt& tsca = mamIt->second;

      if (_dying) {
        tsca.promise.setError("TransportSocketCache is closed.");
        return;
      }

      tsca.socket_count--;
      if (tsca.socket_count != 0) {
        // We still have some sockets attempting to connect to the service, so
        // we just ignore this disconnection.
        return;
      }

      // No socket can be created, we just return an error.
      std::stringstream ss;
      ss << "Failed to connect to service " << servInfo.name() << " on "
         << "machine " << servInfo.machineId() << ". All endpoints are "
         << "unavailable.";
      tsca.promise.setError(ss.str());
    } // ! boost::mutex::scoped_lock
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
  void TransportSocketCache::onSocketConnected(TransportSocketPtr socket, const qi::ServiceInfo& servInfo, const qi::Url& url) {
    {
      boost::mutex::scoped_lock sl(_socketsMutex);

      MachineAttemptsMap::iterator mamIt;
      if ((mamIt = _attempts.find(servInfo.machineId())) == _attempts.end()) {
        // Unknown error. This shouldn't happen...
        return;
      }
      TransportSocketConnectionAttempt& tsca = mamIt->second;

      if (_dying) {
        tsca.promise.setError("TransportSocketCache is closed.");
        return;
      }

      if (tsca.successful) {
        // If we are already connected to this service, disconnect this socket.
        socket->disconnect();
        return;
      }

      // Else, we set promise to this socket. We have a winner.
      MachineConnectionMap::iterator mcmIt;
      if ((mcmIt = _sockets.find(servInfo.machineId())) != _sockets.end()) {
        TransportSocketConnectionMap& tscm = mcmIt->second;
        TransportSocketConnectionMap::iterator tscmIt;
        if ((tscmIt = tscm.find(url.str())) != tscm.end()) {
          tscmIt->second.promise.setValue(socket);
          tsca.successful = true;
        }
      }
    } // ! boost::mutex::scoped_lock
  }
}
