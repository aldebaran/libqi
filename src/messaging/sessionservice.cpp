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

  Session_Service::Session_Service(TransportSocketCache* socketCache,
                                   ServiceDirectoryClient* sdClient, ObjectRegistrar* server, bool enforceAuth)
    : _socketCache(socketCache)
    , _sdClient(sdClient)
    , _server(server)
    , _self(this, sessionServiceWaitBarrier) // create a shared_ptr so that shared_from_this works
    , _enforceAuth(enforceAuth)
  {
    _linkServiceRemoved = _sdClient->serviceRemoved.connect(&Session_Service::onServiceRemoved, this, _1, _2);
  }


  Session_Service::~Session_Service()
  {
    _sdClient->serviceRemoved.disconnect(_linkServiceRemoved);
    _self.reset(); // now existing weak_ptrs cannot from this cannot be locked
    _destructionBarrier.future().wait();
    close();
    destroy();
  }

  void Session_Service::onServiceRemoved(const unsigned int &index, const std::string &service) {
    qiLogVerbose() << "Remote Service Removed:" << service << " #" << index;
    removeService(service);
  }

  void Session_Service::removeService(const std::string &service) {
    {
      boost::recursive_mutex::scoped_lock sl(_remoteObjectsMutex);
      RemoteObjectMap::iterator it = _remoteObjects.find(service);
      if (it != _remoteObjects.end()) {
        qiLogVerbose() << "Session: Removing cached RemoteObject " << service;
        static_cast<RemoteObject*>(it->second.asGenericObject()->value)->close("Service removed");
        _remoteObjects.erase(it);
      }
    }
  }

  void Session_Service::setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr factory)
  {
    _authFactory = factory;
  }

  void Session_Service::close() {
    //cleanup all RemoteObject
    //they are not valid anymore after this function
    boost::recursive_mutex::scoped_lock sl(_remoteObjectsMutex);

    // This function is recursive. Sometimes, calling close on the
    // remoteobject triggers a onSocketDisconnect which calls this function
    // again. We must not allow remoteobjects to be cleaned twice.
    RemoteObjectMap objects;
    std::swap(objects, _remoteObjects);

    for (RemoteObjectMap::iterator it = objects.begin();
        it != objects.end(); ++it)
      static_cast<RemoteObject*>(it->second.asGenericObject()->value)->close("Session closed");
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

  static void deleteLater(qi::RemoteObject *remote, ServiceRequest   *sr) {
    delete remote;
    delete sr;
  }

  void Session_Service::removeRequest(long requestId)
  {
    boost::recursive_mutex::scoped_lock sl(_requestsMutex);
    qi::RemoteObject *remote = nullptr;
    ServiceRequest   *sr     = nullptr;
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
    //At this point the RemoteObject can be either in remote (RemoteObject*)
    //or in sr->promise (promise<AnyObject>), so async them both
    qi::getEventLoop()->post(boost::bind(&deleteLater, remote, sr));
  }

  static void sendCapabilities(TransportSocketPtr sock)
  {
    Message msg;
    msg.setType(Message::Type_Capability);
    msg.setService(Message::Service_Server);
    msg.setValue(sock->localCapabilities(), typeOf<CapabilityMap>()->signature());
    sock->send(msg);
  }

  void Session_Service::onAuthentication(const TransportSocket::SocketEventData& data, long requestId, TransportSocketPtr socket, ClientAuthenticatorPtr auth, SignalSubscriberPtr old)
  {
    static const std::string cmsig = typeOf<CapabilityMap>()->signature().toString();
    boost::recursive_mutex::scoped_lock sl(_requestsMutex);
    ServiceRequest *sr = serviceRequest(requestId);
    if (!sr)
      return;
    if (data.which() == TransportSocket::Event_Error)
    {
      if (old)
        socket->socketEvent.disconnect(*old);
      sr->promise.setError(boost::get<std::string>(data));
      removeRequest(requestId);
      return;
    }

    const Message& msg = boost::get<const Message&>(data);
    int function = msg.function();
    bool failure = msg.type() == Message::Type_Error
        || msg.service() != Message::Service_Server
        || function != Message::ServerFunction_Authenticate;

    if (failure)
    {
      if (old)
        socket->socketEvent.disconnect(*old);
      if (_enforceAuth)
      {
        std::stringstream error;
        if (msg.type() == Message::Type_Error)
          error << "Error while authenticating: " << msg.value("s", socket).to<std::string>();
        else
          error << "Expected a message for function #" << Message::ServerFunction_Authenticate << " (authentication), received a message for function " << function;
        sr->promise.setError(error.str());
        removeRequest(requestId);
      }
      else
      {
        sendCapabilities(socket);
        qi::Future<void> metaObjFut;
        sr->remoteObject = new qi::RemoteObject(sr->serviceId, socket);
        metaObjFut = sr->remoteObject->fetchMetaObject();
        metaObjFut.connect(&Session_Service::onRemoteObjectComplete, this, _1, requestId);
      }
      return;
    }

    AnyReference cmref = msg.value(typeOf<CapabilityMap>()->signature(), socket);
    CapabilityMap authData = cmref.to<CapabilityMap>();
    CapabilityMap::iterator authStateIt = authData.find(AuthProvider::State_Key);
    cmref.destroy();

    if (authStateIt == authData.end() || authStateIt->second.to<unsigned int>() < AuthProvider::State_Error
        || authStateIt->second.to<unsigned int>() > AuthProvider::State_Done)
    {
      if (old)
        socket->socketEvent.disconnect(*old);
      std::string error = "Invalid authentication state token.";
      sr->promise.setError(error);
      removeRequest(requestId);
      qiLogInfo() << error;
      return;
    }
    if (authData[AuthProvider::State_Key].to<unsigned int>() == AuthProvider::State_Done)
    {
      qi::Future<void> metaObjFut;
      if (old)
        socket->socketEvent.disconnect(*old);
      sr->remoteObject = new qi::RemoteObject(sr->serviceId, socket);
      //ask the remoteObject to fetch the metaObject
      metaObjFut = sr->remoteObject->fetchMetaObject();
      metaObjFut.connect(&Session_Service::onRemoteObjectComplete, this, _1, requestId);
      return;
    }

    CapabilityMap nextData = auth->processAuth(authData);
    Message authMsg;
    authMsg.setService(Message::Service_Server);
    authMsg.setType(Message::Type_Call);
    authMsg.setValue(nextData, cmsig);
    authMsg.setFunction(Message::ServerFunction_Authenticate);
    socket->send(authMsg);
  }

  void Session_Service::onTransportSocketResult(qi::Future<TransportSocketPtr> value, long requestId) {
    qiLogDebug() << "Got transport socket for service";
    {
      boost::recursive_mutex::scoped_lock sl(_requestsMutex);
      ServiceRequest *sr = serviceRequest(requestId);
      if (!sr)
        return;

      if (value.hasError()) {
        sr->promise.setError(value.error());
        removeRequest(requestId);
        return;
      }
    }
    TransportSocketPtr socket = value.value();

    // If true, this socket came from the socket cache and has already been identified.
    // This typically happens when two services are behind the same endpoint.
    // We forge a message that just shows we've authenticated successfully.
    if (socket->hasReceivedRemoteCapabilities())
    {
      Message dummy;
      CapabilityMap cm;
      cm[AuthProvider::State_Key] = AnyValue::from(AuthProvider::State_Done);
      dummy.setType(Message::Type_Reply);
      dummy.setFunction(qi::Message::ServerFunction_Authenticate);
      dummy.setValue(AnyValue::from(cm), typeOf<CapabilityMap>()->signature());
      onAuthentication(TransportSocket::SocketEventData(dummy), requestId, socket, ClientAuthenticatorPtr(new NullClientAuthenticator), SignalSubscriberPtr());
      return;
    }
    ClientAuthenticatorPtr authenticator = _authFactory->newAuthenticator();
    CapabilityMap authCaps;
    {
      CapabilityMap tmp = authenticator->initialAuthData();
      for (CapabilityMap::iterator it = tmp.begin(), end = tmp.end(); it != end; ++it)
        authCaps[AuthProvider::UserAuthPrefix + it->first] = it->second;
    }
    SignalSubscriberPtr protSubscriber(new SignalSubscriber);
    *protSubscriber = socket->socketEvent.connect(&Session_Service::onAuthentication, this, _1, requestId, socket, authenticator, protSubscriber);


    Message msgCapabilities;
    msgCapabilities.setFunction(Message::ServerFunction_Authenticate);
    msgCapabilities.setService(Message::Service_Server);
    msgCapabilities.setType(Message::Type_Call);

    TransportSocketPtr sdSocket = _sdClient->socket();
    CapabilityMap socketCaps;
    if (sdSocket)
    {
      socketCaps = sdSocket->localCapabilities();
      socket->advertiseCapabilities(socketCaps);
    }
    socketCaps.insert(authCaps.begin(), authCaps.end());
    msgCapabilities.setValue(socketCaps, typeOf<CapabilityMap>()->signature());
    socket->send(msgCapabilities);
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
      boost::recursive_mutex::scoped_lock sl(_remoteObjectsMutex);
      RemoteObjectMap::iterator it = _remoteObjects.find(sr->name);
      if (it != _remoteObjects.end()) {
        //another object have been registered before us, return it
        //the new socket will be closed when the request is deleted
        qiLogVerbose() << "A request for the service " << sr->name << " have been discarded, "
                                        << "the remoteobject on the service was already available.";
        sr->promise.setValue(it->second);
      } else {

        AnyObject o = makeDynamicAnyObject(sr->remoteObject);
        //register the remote object in the cache
        addService(sr->name, o);
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
    {
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
      if (_sdClient->isLocal())
      { // Wait! If sd is local, we necessarily have an open socket
        // on which service was registered, whose lifetime is bound
        // to the service
        TransportSocketPtr s = _sdClient->_socketOfService(sr->serviceId);
        if (!s) // weird
          qiLogVerbose() << "_socketOfService returned 0";
        else
        {
          // check if the socket support that capability
          if (s->remoteCapability("ClientServerSocket", false))
          {
            qiLogVerbose() << "sd is local and service is capable, going through socketOfService";
            onTransportSocketResult(qi::Future<TransportSocketPtr>(s), requestId);
            return;
          }
        }
      }
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
          qiLogVerbose() << ss.str();
          sr->promise.setError(ss.str());
        }
      }
    }
    qiLogDebug() << "Requesting socket from cache";
    qi::Future<qi::TransportSocketPtr> fut = _socketCache->socket(result.value(), protocol);
    fut.connect(&Session_Service::onTransportSocketResult, this, _1, requestId);
  }

  void Session_Service::addService(const std::string& name, const qi::AnyObject &obj) {
    boost::recursive_mutex::scoped_lock sl(_remoteObjectsMutex);
    RemoteObjectMap::iterator it = _remoteObjects.find(name);
    qiLogDebug() << "Adding remoteobject:" << name << " :" << &obj;
    if (it == _remoteObjects.end())
      _remoteObjects[name] = obj;
    else
      throw std::runtime_error("Service already in cache: " + name);
  }

  qi::Future<qi::AnyObject> Session_Service::service(const std::string &service,
                                                     const std::string &protocol)
  {
    qi::Future<qi::AnyObject> result;
    if (protocol == "" || protocol == "local") {
      //qiLogError() << "service is not implemented for local service, it always return a remote service";
      //look for local object registered in the server
      qi::AnyObject go = _server->registeredServiceObject(service);
      if (go)
        return qi::Future<qi::AnyObject>(go);
      if (protocol == "local") {
        qi::Promise<qi::AnyObject> prom;
        prom.setError(std::string("No local object found for ") + service);
        return prom.future();
      }
    }

    //look for already registered remote objects
    {
      boost::recursive_mutex::scoped_lock sl(_remoteObjectsMutex);
      RemoteObjectMap::iterator it = _remoteObjects.find(service);
      if (it != _remoteObjects.end()) {
        return qi::Future<qi::AnyObject>(it->second);
      }
    }

    {
      boost::recursive_mutex::scoped_lock l(_requestsMutex);
      std::map<int, ServiceRequest*>::const_iterator it;
      for (it = _requests.begin(); it != _requests.end(); ++it)
      {
        if (it->second->name == service)
        {
          return it->second->promise.future();
        }
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
