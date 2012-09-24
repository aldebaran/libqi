/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "src/transportsocketcache.hpp"


namespace qi {

  //return a connected TransportSocket for the given endpoint
  qi::Future<qi::TransportSocketPtr> TransportSocketCache::socket(const std::string &endpoint)
  {
    {
      boost::mutex::scoped_lock sl(_socketsMutex);
      TransportSocketConnectionMap::iterator it;
      it = _sockets.find(endpoint);
      if (it != _sockets.end()) {
        TransportSocketConnection &tsc = it->second;
        qi::TransportSocketPtr    &socket = tsc.socket;
        //try to reconnect when it's not connected and not connecting.
        if (tsc.promise.future().hasError()) {
          //reset the promise, we are trying again.
          tsc.promise.reset();
          socket->connect(qi::Url(endpoint));
        }
        return tsc.promise.future();
      }

      qi::Url                             url(endpoint);
      qi::TransportSocketPtr              socket = makeTransportSocket(url.protocol());
      qi::Promise<qi::TransportSocketPtr> prom;
      socket->connected.connect(boost::bind(&TransportSocketCache::onSocketConnected, this, socket, endpoint));
      socket->disconnected.connect(boost::bind(&TransportSocketCache::onSocketDisconnected, this, _1, socket, endpoint));
      TransportSocketConnection &tsc = _sockets[endpoint];
      tsc.socket = socket;
      tsc.promise = prom;
      socket->connect(url).async();
      return prom.future();
    }
  }

  //will return the first connected socket among all provided endpoints
  qi::Future<qi::TransportSocketPtr> TransportSocketCache::socket(const std::vector<std::string> &endpoints) {

    return socket(endpoints.at(0));

    //yes maybe one day we will do some crazy stuff... but tonight...
    std::list< qi::Future<qi::TransportSocketPtr> > futs;

    for (int i = 0; i = endpoints.size(); ++i) {
      futs.push_back(socket(endpoints.at(i)));
    }

    qi::Future<qi::TransportSocketPtr> fut;

    //fut.addCallbacks(this, );
    //put a futureforwarder.
    //
    //add callback to all
  }

  void TransportSocketCache::onSocketDisconnected(int error, TransportSocketPtr client, const std::string &endpoint) {
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
      ss << "Connection to " << endpoint << " failed.";
      prom.setError(ss.str());
    }
  }

  void TransportSocketCache::onSocketConnected(TransportSocketPtr socket, const std::string &endpoint)
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
