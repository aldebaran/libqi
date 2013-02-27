/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#include "sessionservice.hpp"
#include "servicedirectoryclient.hpp"
#include "objectregistrar.hpp"
#include "remoteobject_p.hpp"

qiLogCategory("qimessaging.sessionservice");

namespace qi {

  inline void sessionServiceWaitBarrier(Session_Service* ptr) {
    ptr->_destructionBarrier.setValue(0);
  }
  Session_Service::Session_Service(TransportSocketCache *socketCache, ServiceDirectoryClient *sdClient, ObjectRegistrar *server)
    : _socketCache(socketCache)
    , _sdClient(sdClient)
    , _server(server)
    , _self(this, sessionServiceWaitBarrier) // create a shared_ptr so that shared_from_this works
  {
    _linkServiceRemoved = _sdClient->serviceRemoved.connect(boost::bind<void>(&Session_Service::onServiceRemoved, this, _1, _2));
  }

  Session_Service::~Session_Service()
  {
    _sdClient->serviceRemoved.disconnect(_linkServiceRemoved);
    _self.reset(); // now existing weak_ptrs cannot from this cannot be locked
    _destructionBarrier.future().wait();
    close();
  }

  void Session_Service::onServiceRemoved(const unsigned int &index, const std::string &service) {
    qiLogVerbose() << "Remote Service Removed:" << service << " #" << index;
    removeService(service);
  }

  void Session_Service::removeService(const std::string &service) {
    {
      boost::mutex::scoped_lock sl(_remoteObjectsMutex);
      RemoteObjectMap::iterator it = _remoteObjects.find(service);
      if (it != _remoteObjects.end()) {
        qiLogVerbose() << "Session: Removing cached RemoteObject " << service;
        _remoteObjects.erase(it);
      }
    }
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
    boost::recursive_mutex::scoped_lock sl(_requestsMutex);
    {
      std::map<int, ServiceRequest*>::iterator it;

      it = _requests.find(requestId);
      if (it == _requests.end()) {
        qiLogVerbose() << "qi.session.service(): No matching request for id(" << requestId << ").";
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
    boost::recursive_mutex::scoped_lock sl(_requestsMutex);
    qi::RemoteObject *remote = 0;
    ServiceRequest   *sr     = 0;
    {
      std::map<int, ServiceRequest*>::iterator it;

      it = _requests.find(requestId);
      if (it == _requests.end()) {
        qiLogVerbose() << "qi.session.service(): No matching request for id(" << requestId << ").";
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
    qi::getDefaultNetworkEventLoop()->post(boost::bind(&deleteLater, remote));
    delete sr;
  }

  void Session_Service::onTransportSocketResult(qi::Future<TransportSocketPtr> value, long requestId) {
    qiLogDebug() << "Got transport socket for service";
    boost::recursive_mutex::scoped_lock sl(_requestsMutex);
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
    qiLogDebug() << "Got metaobject";
    boost::recursive_mutex::scoped_lock l(_requestsMutex);
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
        qiLogVerbose() << "A request for the service " << sr->name << " have been discarded, "
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

  inline void onServiceInfoResultIfExists(Session_Service* s, qi::Future<qi::ServiceInfo> f,
    long requestId, std::string protocol, boost::weak_ptr<Session_Service> self)
  {
    boost::shared_ptr<Session_Service> sself = self.lock();
    if (sself)
      sself->onServiceInfoResult(f, requestId, protocol);
  }

  // We received a ServiceInfo, and want to establish a connection
  void Session_Service::onServiceInfoResult(qi::Future<qi::ServiceInfo> result, long requestId, std::string protocol) {
    qiLogDebug() << "Got serviceinfo message";
    boost::recursive_mutex::scoped_lock sl(_requestsMutex);
    ServiceRequest *sr = serviceRequest(requestId);
    if (!sr)
      return;
    if (result.hasError()) {
      sr->promise.setError(result.error());
      removeRequest(requestId);
      return;
    }
    const qi::ServiceInfo &si = result.value();
    sr->serviceId = si.serviceId();
    //empty serviceInfo
    if (!si.endpoints().size()) {
      std::stringstream ss;
      ss << "No endpoints returned for service:" << sr->name << " (id:" << sr->serviceId << ")";
      qiLogVerbose() << ss.str();
      sr->promise.setError(ss.str());
      removeRequest(requestId);
      return;
    }

    if (protocol != "")
    {
      std::vector<qi::Url>::const_iterator it = si.endpoints().begin();

      for (;
           it != si.endpoints().end() && it->protocol() != protocol;
           it++)
      {
        continue;
      }

      if (it == si.endpoints().end())
      {
        std::stringstream ss;
        ss << "No " << protocol << " endpoint available for service:" << sr->name << " (id:" << sr->serviceId << ")";
        qiLogVerbose("session.service") << ss.str();
        sr->promise.setError(ss.str());
      }
    }

    qiLogDebug() << "Requesting socket from cache";
    qi::Future<qi::TransportSocketPtr> fut = _socketCache->socket(si, protocol);
    fut.connect(boost::bind<void>(&Session_Service::onTransportSocketResult, this, _1, requestId));
  }

  qi::Future<qi::ObjectPtr> Session_Service::service(const std::string &service,
                                                     const std::string &protocol)
  {
    qi::Future<qi::ObjectPtr> result;
    if (protocol == "" || protocol == "local") {
      //qiLogError() << "service is not implemented for local service, it always return a remote service";
      //look for local object registered in the server
      qi::ObjectPtr go = _server->registeredServiceObject(service);
      if (go)
        return qi::Future<qi::ObjectPtr>(go);
      if (protocol == "local") {
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
      boost::recursive_mutex::scoped_lock l(_requestsMutex);
      _requests[requestId] = rq;
    }
    result = rq->promise.future();
    //rq is not valid anymore after addCallbacks, because it could have been handled and cleaned
    fut.connect(boost::bind<void>(&onServiceInfoResultIfExists, this, _1, requestId, protocol, boost::weak_ptr<Session_Service>(_self)));
    return result;
  }
}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
