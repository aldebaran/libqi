/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "transportsocketcache.hpp"


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
      TransportSocketConnectionMap socketsCopy;
      {
        // Do not hold _socketsMutex while iterating or deadlock may occurr
        // between disconnect() that waits for callback handler
        // and callback handler that tries to acquire _socketsMutex
        boost::mutex::scoped_lock sl(_socketsMutex);
        socketsCopy = _sockets;
      }
      TransportSocketConnectionMap::iterator it;
      for (it = socketsCopy.begin(); it != socketsCopy.end(); ++it) {
        it->second.socket->disconnected.disconnect(it->second.disconnectLink);
        it->second.socket->connected.disconnect(it->second.connectLink);
        //remove callback before calling disconnect. (we dont need them)
        it->second.socket->disconnect();
        it->second.promise.setError("session closed");
      }
    }
  }

  //return a connected TransportSocket for the given endpoint
  qi::Future<qi::TransportSocketPtr> TransportSocketCache::socket(const qi::Url &endpoint)
  {
    {
      boost::mutex::scoped_lock sl(_socketsMutex);
      if (_dying)
        return qi::makeFutureError<qi::TransportSocketPtr>("TransportSocketCache is closed");

      TransportSocketConnectionMap::iterator it;
      it = _sockets.find(endpoint);
      if (it != _sockets.end()) {
        TransportSocketConnection &tsc = it->second;
        qi::TransportSocketPtr    &socket = tsc.socket;
        //try to reconnect when it's not connected and not connecting.
        if (tsc.promise.future().hasError()) {
          //reset the promise, we are trying again.
          tsc.promise.reset();
          socket->connect(endpoint);
        }
        return tsc.promise.future();
      }

      qi::TransportSocketPtr              socket = makeTransportSocket(endpoint.protocol());
      qi::Promise<qi::TransportSocketPtr> prom;
      TransportSocketConnection &tsc = _sockets[endpoint];
      tsc.connectLink    = socket->connected.connect(boost::bind(&TransportSocketCache::onSocketConnected, this, socket, endpoint));
      tsc.disconnectLink = socket->disconnected.connect(boost::bind(&TransportSocketCache::onSocketDisconnected, this, _1, socket, endpoint));
      tsc.socket = socket;
      tsc.promise = prom;
      socket->connect(endpoint).async();
      return prom.future();
    }
  }

  //will return the first connected socket among all provided endpoints
  qi::Future<qi::TransportSocketPtr> TransportSocketCache::socket(const qi::UrlVector &endpoints) {

    //TODO: handle multiples endpoints..
    return socket(endpoints.at(0));

    //yes maybe one day we will do some crazy stuff... but tonight...
    std::list< qi::Future<qi::TransportSocketPtr> > futs;

    for (int i = 0; i == endpoints.size(); ++i) {
      futs.push_back(socket(endpoints.at(i)));
    }

    qi::Future<qi::TransportSocketPtr> fut;

    //fut.addCallbacks(this, );
    //put a futureforwarder.
    //
    //add callback to all
  }

  void TransportSocketCache::onSocketDisconnected(int error, TransportSocketPtr
  client, const qi::Url &endpoint) {
    qi::Promise<TransportSocketPtr> prom;
    int setprom = 0;
    {
      boost::mutex::scoped_lock sl(_socketsMutex);
      TransportSocketConnectionMap::iterator it;
      it = _sockets.find(endpoint);
      if (it != _sockets.end()) {
        prom = it->second.promise;
        setprom = 1;
      }
    }
    if (setprom) {
      std::stringstream ss;
      ss << "Connection to " << endpoint.str() << " failed.";
      prom.setError(ss.str());
    }
  }

  void TransportSocketCache::onSocketConnected(TransportSocketPtr socket, const qi::Url &endpoint)
  {
    qi::Promise<TransportSocketPtr> prom;
    int setprom = 0;
    {
      boost::mutex::scoped_lock sl(_socketsMutex);
      TransportSocketConnectionMap::iterator it;
      it = _sockets.find(endpoint);
      if (it != _sockets.end()) {
        prom = it->second.promise;
        setprom = 1;
      }
    }
    if (setprom) {
      prom.setValue(socket);
    }
  }

}
