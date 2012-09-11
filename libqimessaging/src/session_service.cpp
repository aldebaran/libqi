/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "session_service.hpp"
#include "service_directory_client.hpp"
#include "session_server.hpp"
#include "server_client.hpp"

namespace qi {

  Session_Service::~Session_Service()
  {
    close();
  }

  void Session_Service::close() {
    //cleanup all RemoteObject
    //they are not valid anymore after this function
    {
      boost::mutex::scoped_lock sl(_remoteObjectsMutex);
      RemoteObjectMap::iterator it = _remoteObjects.begin();
      for (; it != _remoteObjects.end(); ++it) {
        it->second.close();
      }
      _remoteObjects.clear();
    }
  }


  ServiceRequest *Session_Service::serviceRequest(void *data)
  {
    {
      boost::mutex::scoped_lock                 sl(_requestsMutex);
      long                                      requestId = reinterpret_cast<long>(data);
      std::map<long, ServiceRequest*>::iterator it;

      it = _requests.find(requestId);
      if (it == _requests.end()) {
        qiLogVerbose("qi.session_service") << "qi.session.service(): No matching request for id(" << requestId << ").";
        return 0;
      }
      return it->second;
    }
  }

  void Session_Service::removeRequest(void *data)
  {
    {
      boost::mutex::scoped_lock                 l(_requestsMutex);
      long                                      requestId = reinterpret_cast<long>(data);
      std::map<long, ServiceRequest*>::iterator it;

      it = _requests.find(requestId);
      if (it == _requests.end()) {
        qiLogVerbose("qi.session_service") << "qi.session.service(): No matching request for id(" << requestId << ").";
        return;
      }
      if (it->second) {
        ServerClient *sc = it->second->sclient;
        delete sc;
        it->second->sclient = 0;
      }
      delete it->second;
      it->second = 0;
      _requests.erase(it);
    }
  }
  void Session_Service::onSocketDisconnected(TransportSocket *client, void *data) {
    delete client;
  }

  void Session_Service::onSocketConnectionError(TransportSocket *client, void *data) {
    delete client;
}

  void Session_Service::onSocketConnected(TransportSocket *client, void *data)
  {
    ServiceRequest *sr = serviceRequest(data);
    if (!sr)
      return;

    if (sr->connected)
    {
      // An other attempt got here first, disconnect and drop
      client->disconnect();
      client->removeCallbacks(this);
      delete client;
      return;
    }
    sr->connected = true;
    sr->socket    = client;
    client->removeCallbacks(this);
    sr->sclient   = new qi::ServerClient(client);

    qi::Future<qi::MetaObject> fut = sr->sclient->metaObject(sr->serviceId, qi::Message::Object_Main);
    fut.addCallbacks(this, data);
  }

  void Session_Service::onFutureFinished(const qi::MetaObject &mo, void *data) {
    ServiceRequest *sr = serviceRequest(data);
    if (!sr)
      return;
    {
      boost::mutex::scoped_lock sl(_remoteObjectsMutex);
      RemoteObjectMap::iterator it = _remoteObjects.find(sr->name);
      if (it != _remoteObjects.end()) {
        //another object have been registered before us, return it
        //the new socket will be closed when the request is deleted
        qiLogVerbose("session_service") << "A request for the service " << sr->name << " have been discarded, "
                                        << "the remoteobject on the service was already available.";
        sr->promise.setValue(it->second);
      } else {
        qi::RemoteObject robj(sr->socket, sr->serviceId, mo);
        //remove the callback of ServerClient before returning the object
        boost::shared_ptr<qi::RemoteObjectPrivate> rop;
        rop = boost::dynamic_pointer_cast<qi::RemoteObjectPrivate>(sr->sclient->_object._p);
        sr->socket->removeCallbacks(rop.get());
        //avoid deleting the socket in removeRequest (RemoteObject will do it)
        sr->socket = 0;
        //register the remote object in the cache
        _remoteObjects[sr->name] = robj;
        sr->promise.setValue(robj);
      }
    }

    removeRequest(data);
  }

  // We received a ServiceInfo, and want to establish a connection
  void Session_Service::onFutureFinished(const qi::ServiceInfo &si, void *data) {
    ServiceRequest *sr = serviceRequest(data);
    if (!sr)
      return;
    sr->attempts = 0;
    sr->serviceId = si.serviceId();
    // Attempt to connect to all endpoints.
    std::vector<std::string>::const_iterator it;
    for (it = si.endpoints().begin(); it != si.endpoints().end(); ++it)
    {
      qi::Url url(*it);
      if (sr->protocol == "any" || sr->protocol == url.protocol())
      {
        qi::TransportSocket *ts = NULL;
        ts = new qi::TransportSocket();
        ts->addCallbacks(this, data);
        sr->socket = ts;
        ts->connect(url).async();
        // The connect may still fail asynchronously.
        ++sr->attempts;
      }
      break;
    }

    // All attempts failed synchronously.
    if (!sr->attempts) {
      sr->promise.setError("No service found");
      removeRequest(data);
    }
  }

  void Session_Service::onFutureFailed(const std::string &error, void *data) {
    ServiceRequest *sr = serviceRequest(data);
    if (!sr)
      return;
    sr->promise.setError(error);
    removeRequest(data);
  }

  qi::Future<qi::Object> Session_Service::service(const std::string &service,
                                                  Session::ServiceLocality locality,
                                                  const std::string &type)
  {
    qi::Future<qi::Object> result;
    if (locality == Session::ServiceLocality_Local) {
      qiLogError("session.service") << "service is not implemented for local service, it always return a remote service";
    }

    //look for already registered remote objects
    {
      boost::mutex::scoped_lock sl(_remoteObjectsMutex);
      RemoteObjectMap::iterator it = _remoteObjects.find(service);
      if (it != _remoteObjects.end())
        return qi::Future<qi::Object>(it->second);
    }

    qi::Future<qi::ServiceInfo> fut = _sdClient->service(service);
    ServiceRequest *rq = new ServiceRequest(service, type);
    long requestId = ++_requestsIndex;

    {
      boost::mutex::scoped_lock l(_requestsMutex);
      _requests[requestId] = rq;
    }
    result = rq->promise.future();
    //rq is not valid anymore after addCallbacks, because it could have been handled and cleaned
    fut.addCallbacks(this, reinterpret_cast<void *>(requestId));
    return result;
  }


}
