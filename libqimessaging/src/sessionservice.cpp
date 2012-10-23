/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "sessionservice.hpp"
#include "servicedirectoryclient.hpp"
#include "objectregistrar.hpp"
#include "remoteobject_p.hpp"

namespace qi {

  Session_Service::Session_Service(TransportSocketCache *socketCache, ServiceDirectoryClient *sdClient, ObjectRegistrar *server)
    : _socketCache(socketCache)
    , _sdClient(sdClient)
    , _server(server)
  {
  }

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
        reinterpret_cast<RemoteObject*>(it->second->value)->close();
      }
      _remoteObjects.clear();
    }
  }


  ServiceRequest *Session_Service::serviceRequest(long requestId)
  {
    {
      boost::mutex::scoped_lock                 sl(_requestsMutex);
      std::map<long, ServiceRequest*>::iterator it;

      it = _requests.find(requestId);
      if (it == _requests.end()) {
        qiLogVerbose("qi.session_service") << "qi.session.service(): No matching request for id(" << requestId << ").";
        return 0;
      }
      return it->second;
    }
  }

  static void deleteLater(qi::RemoteObject *remote) {
    delete remote;
  }

  void Session_Service::removeRequest(long requestId)
  {
    qi::RemoteObject *remote = 0;
    ServiceRequest   *sr     = 0;
    {
      boost::mutex::scoped_lock                 l(_requestsMutex);
      std::map<long, ServiceRequest*>::iterator it;

      it = _requests.find(requestId);
      if (it == _requests.end()) {
        qiLogVerbose("qi.session_service") << "qi.session.service(): No matching request for id(" << requestId << ").";
        return;
      }
      if (it->second) {
        remote = it->second->remoteObject;
        it->second->remoteObject = 0;
      }
      sr = it->second;
      it->second = 0;
      _requests.erase(it);
    }
    //we do not call delete on RemoteObject, because remoteObject->close disconnect onMessagePending,
    //which is the signal we are coming from.  (when called from onRemoteObjectComplete)
    //delete later as a workaround.
    qi::getDefaultNetworkEventLoop()->asyncCall(0, boost::bind(&deleteLater, remote));
    delete sr;
  }

  void Session_Service::onTransportSocketResult(qi::Future<TransportSocketPtr> value, long requestId) {
    ServiceRequest *sr = serviceRequest(requestId);
    if (!sr)
      return;

    if (value.hasError()) {
      sr->promise.setError(value.error());
      removeRequest(requestId);
      return;
    }

    sr->remoteObject = new qi::RemoteObject(sr->serviceId, value.value());

    //ask the remoteObject to fetch the metaObject
    qi::Future<void> fut = sr->remoteObject->fetchMetaObject();
    fut.connect(boost::bind<void>(&Session_Service::onRemoteObjectComplete, this, _1, requestId));
  }

  void Session_Service::onRemoteObjectComplete(qi::Future<void> future, long requestId) {
    ServiceRequest *sr = serviceRequest(requestId);
    if (!sr)
      return;

    if (future.hasError()) {
      sr->promise.setError(future.error());
      removeRequest(requestId);
      return;
    }

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

        ObjectPtr o = makeDynamicObjectPtr(sr->remoteObject);
        //register the remote object in the cache
        _remoteObjects[sr->name] = o;
        sr->promise.setValue(o);
        sr->remoteObject = 0;
      }
    }

    removeRequest(requestId);
  }

  // We received a ServiceInfo, and want to establish a connection
  void Session_Service::onServiceInfoResult(qi::Future<qi::ServiceInfo> result, long requestId) {
    ServiceRequest *sr = serviceRequest(requestId);
    if (!sr)
      return;
    if (result.hasError()) {
      sr->promise.setError(result.error());
      removeRequest(requestId);
      return;
    }
    qi::ServiceInfo &si = result.value();
    sr->serviceId = si.serviceId();
    //empty serviceInfo
    if (!si.endpoints().size()) {
      std::stringstream ss;
      ss << "No endpoints returned for service:" << sr->name << " (id:" << sr->serviceId << ")";
      qiLogVerbose("session.service") << ss.str();
      sr->promise.setError(ss.str());
      removeRequest(requestId);
      return;
    }

    qi::Future<qi::TransportSocketPtr> fut = _socketCache->socket(si.endpoints());
    fut.connect(boost::bind<void>(&Session_Service::onTransportSocketResult, this, _1, requestId));
  }

  qi::Future<qi::ObjectPtr> Session_Service::service(const std::string &service,
                                                     Session::ServiceLocality locality)
  {
    qiLogVerbose("session.service") << "Getting service " << service;
    qi::Future<qi::ObjectPtr> result;
    if (locality != Session::ServiceLocality_Remote) {
      //qiLogError("session.service") << "service is not implemented for local service, it always return a remote service";
      //look for local object registered in the server
      qi::ObjectPtr go = _server->registeredServiceObject(service);
      if (go)
        return qi::Future<qi::ObjectPtr>(go);
      if (locality == Session::ServiceLocality_Local) {
        qi::Promise<qi::ObjectPtr> prom;
        prom.setError(std::string("No local object found for ") + service);
        return prom.future();
      }
    }

    //look for already registered remote objects
    {
      boost::mutex::scoped_lock sl(_remoteObjectsMutex);
      RemoteObjectMap::iterator it = _remoteObjects.find(service);
      if (it != _remoteObjects.end()) {
        return qi::Future<qi::ObjectPtr>(it->second);
      }
    }

    qi::Future<qi::ServiceInfo> fut = _sdClient->service(service);
    ServiceRequest *rq = new ServiceRequest(service);
    long requestId = ++_requestsIndex;

    {
      boost::mutex::scoped_lock l(_requestsMutex);
      _requests[requestId] = rq;
    }
    result = rq->promise.future();
    //rq is not valid anymore after addCallbacks, because it could have been handled and cleaned
    fut.connect(boost::bind<void>(&Session_Service::onServiceInfoResult, this, _1, requestId));
    return result;
  }


}
