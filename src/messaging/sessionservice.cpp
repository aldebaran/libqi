/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#include <ka/scoped.hpp>
#include "sessionservice.hpp"
#include "servicedirectoryclient.hpp"
#include "objectregistrar.hpp"
#include "remoteobject_p.hpp"

qiLogCategory("qimessaging.sessionservice");

namespace qi {

  Session_Service::Session_Service(TransportSocketCache* socketCache,
                                   ServiceDirectoryClient* sdClient, ObjectRegistrar* server, bool enforceAuth)
    : _socketCache(socketCache)
    , _sdClient(sdClient)
    , _server(server)
    , _enforceAuth(enforceAuth)
  {
    _sdClient->serviceRemoved.connect(track([this](unsigned int index, const std::string& service) -> void
    {
      qiLogVerbose() << "Remote Service Removed:" << service << " #" << index;
      removeService(service);
    }, this));
  }


  Session_Service::~Session_Service()
  {
    destroy(); // invalidates signal connections
    close();
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

  namespace session_service_private
  {
    static void sendCapabilities(MessageSocketPtr sock)
    {
      Message msg;
      msg.setType(Message::Type_Capability);
      msg.setService(Message::Service_Server);
      msg.setValue(sock->localCapabilities(), typeOf<CapabilityMap>()->signature());
      sock->send(std::move(msg));
    }
  } // session_service_private

  // Bind the 'set in error' of the promise and request removal.
  void Session_Service::setErrorAndRemoveRequest(
    Promise<AnyObject> p, const std::string& message, long requestId)
  {
    try
    {
      p.setError(message);
    }
    catch (...)
    {
      qiLogDebug() << "Exception launched while trying to set the `service()` promise. "
        "Exception ignored because we are in a fallback case. " <<
        "requestId=" << requestId;
    }
    removeRequest(requestId);
  }

  namespace
  {
    void logWarningUnknownServiceRequest(const std::string& caller, long requestId)
    {
      qiLogWarning() << caller << ": Unknown service request. "
        "requestId = " << requestId;
    }
  } // namespace

  void Session_Service::SetPromiseInError::operator()()
  {
    if (promise && mustSetPromise && !(*promise).future().isFinished())
    {
      // It's ok to try removing the request even if it has already been removed.
      session.setErrorAndRemoveRequest(
        *promise,
        "Fallback: error because no value has been provided for service request id "
          + os::to_string(requestId) + ".",
        requestId);
    }
  }

  void Session_Service::onAuthentication(const MessageSocket::SocketEventData& data, long requestId, MessageSocketPtr socket, ClientAuthenticatorPtr auth, SignalSubscriberPtr old)
  {
    static const std::string cmsig = typeOf<CapabilityMap>()->signature().toString();
    boost::recursive_mutex::scoped_lock sl(_requestsMutex);
    ServiceRequest *sr = serviceRequest(requestId);
    if (!sr)
    {
      logWarningUnknownServiceRequest("onAuthentication", requestId);
      return;
    }

    // Ensure the promise is always set.
    boost::optional<Promise<AnyObject>> promise = sr->promise;
    bool mustSetPromise = true;
    auto _ = ka::scoped(SetPromiseInError{*this, promise, mustSetPromise, requestId});

    if (data.which() == MessageSocket::Event_Error)
    {
      if (old)
        socket->socketEvent.disconnect(*old);
      setErrorAndRemoveRequest(sr->promise, boost::get<std::string>(data), requestId);
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
        setErrorAndRemoveRequest(sr->promise, error.str(), requestId);
      }
      else
      {
        session_service_private::sendCapabilities(socket);
        qi::Future<void> metaObjFut;
        sr->remoteObject = new qi::RemoteObject(sr->serviceId, socket);

        // TODO 40203: check if it's possible that the following future is never set.
        metaObjFut = sr->remoteObject->fetchMetaObject();

        qiLogVerbose() << "Fetching metaobject (1) for requestId = " << requestId;
        metaObjFut.connect(track(
          boost::bind(
            &Session_Service::onRemoteObjectComplete, this, _1, requestId),
          this));
        mustSetPromise = false;
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
      const std::string error = "Invalid authentication state token.";
      setErrorAndRemoveRequest(sr->promise, error, requestId);
      qiLogInfo() << error;
      return;
    }
    if (authData[AuthProvider::State_Key].to<unsigned int>() == AuthProvider::State_Done)
    {
      qi::Future<void> metaObjFut;
      if (old)
        socket->socketEvent.disconnectAsync(*old);
      sr->remoteObject = new qi::RemoteObject(sr->serviceId, socket);
      //ask the remoteObject to fetch the metaObject
      metaObjFut = sr->remoteObject->fetchMetaObject();
      qiLogVerbose() << "Fetching metaobject (2) for requestId = " << requestId;
      metaObjFut.connect(track(
        boost::bind(
          &Session_Service::onRemoteObjectComplete, this, _1, requestId),
        this));
      mustSetPromise = false;
      return;
    }

    CapabilityMap nextData = auth->processAuth(authData);
    Message authMsg;
    authMsg.setService(Message::Service_Server);
    authMsg.setType(Message::Type_Call);
    authMsg.setValue(nextData, cmsig);
    authMsg.setFunction(Message::ServerFunction_Authenticate);
    socket->send(std::move(authMsg));
  }

  void Session_Service::onTransportSocketResult(qi::Future<MessageSocketPtr> value, long requestId)
  {
    qiLogVerbose() << "Got transport socket for service. requestId = " << requestId;

    bool mustSetPromise = true;
    boost::optional<Promise<AnyObject>> promise;
    auto _ = ka::scoped(SetPromiseInError{*this, promise, mustSetPromise, requestId});

    {
      boost::recursive_mutex::scoped_lock sl(_requestsMutex);
      ServiceRequest *sr = serviceRequest(requestId);
      if (!sr)
      {
        logWarningUnknownServiceRequest("onTransportSocketResult", requestId);
        return;
      }
      promise = sr->promise;

      if (value.hasError())
      {
        setErrorAndRemoveRequest(sr->promise, value.error(), requestId);
        return;
      }
    }

    MessageSocketPtr socket = value.value();

    // If true, this socket came from the socket cache and has already been identified.
    // This typically happens when two services are behind the same endpoint.
    // We forge a message that just shows we've authenticated successfully.
    if (socket->hasReceivedRemoteCapabilities())
    {
      try
      {
        Message dummy;
        CapabilityMap cm;
        cm[AuthProvider::State_Key] = AnyValue::from<unsigned int>(AuthProvider::State_Done);
        dummy.setType(Message::Type_Reply);
        dummy.setFunction(qi::Message::ServerFunction_Authenticate);
        dummy.setValue(AnyValue::from(cm), typeOf<CapabilityMap>()->signature());
        onAuthentication(MessageSocket::SocketEventData(dummy), requestId, socket,
                         ClientAuthenticatorPtr(new NullClientAuthenticator), SignalSubscriberPtr());
        mustSetPromise = false;
      } catch (const std::exception& e) {
        qiLogWarning() << "SessionService Remote Exception: " << e.what();
        throw;
      } catch (...) {
        qiLogWarning() << "SessionService Remote Exception: Unknown";
        throw;
      }
      return;
    }
    ClientAuthenticatorPtr authenticator = _authFactory->newAuthenticator();
    CapabilityMap authCaps;
    {
      CapabilityMap tmp = authenticator->initialAuthData();
      for (auto it = tmp.begin(); it != tmp.end(); ++it) {
        authCaps[AuthProvider::UserAuthPrefix + it->first] = it->second;
      }
    }
    SignalSubscriberPtr protSubscriber(new SignalSubscriber);
    *protSubscriber = socket->socketEvent.connect(track(
        [=](const MessageSocket::SocketEventData& data) {
          onAuthentication(data, requestId, socket, authenticator, protSubscriber);
        },
        this));
    mustSetPromise = false;
    Message msgCapabilities;
    msgCapabilities.setFunction(Message::ServerFunction_Authenticate);
    msgCapabilities.setService(Message::Service_Server);
    msgCapabilities.setType(Message::Type_Call);

    MessageSocketPtr sdSocket = _sdClient->socket();
    CapabilityMap socketCaps;
    if (sdSocket)
    {
      socketCaps = sdSocket->localCapabilities();
      socket->advertiseCapabilities(socketCaps);
    }
    socketCaps.insert(authCaps.begin(), authCaps.end());
    msgCapabilities.setValue(socketCaps, typeOf<CapabilityMap>()->signature());
    socket->send(std::move(msgCapabilities));
  }

  void Session_Service::onRemoteObjectComplete(qi::Future<void> future, long requestId) {
    qiLogVerbose() << "Got metaobject for request id = " << requestId;
    boost::recursive_mutex::scoped_lock l(_requestsMutex);
    ServiceRequest *sr = serviceRequest(requestId);
    if (!sr)
    {
      logWarningUnknownServiceRequest("onRemoteObjectComplete", requestId);
      return;
    }

    bool mustSetPromise = true;
    boost::optional<Promise<AnyObject>> promise = sr->promise;
    auto _ = ka::scoped(SetPromiseInError{*this, promise, mustSetPromise, requestId});

    if (future.hasError())
    {
      setErrorAndRemoveRequest(sr->promise, future.error(), requestId);
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

        // If this throws, the promise will be set because of the `scoped` object.
        addService(sr->name, o);
        sr->promise.setValue(o);
        sr->remoteObject = 0;
      }
    }

    removeRequest(requestId);
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
      if (it != _remoteObjects.end())
      {
        qiLogVerbose() << "Found service '" << service << "' in the registered remote objects.";
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
          qiLogVerbose() << "Found service '" << service << "' in the pending service requests.";
          return it->second->promise.future();
        }
      }
    }

    // TODO 40203: check if it's possible that the following future is never set.
    qi::Future<qi::ServiceInfo> fut = _sdClient->service(service);
    ServiceRequest *rq = new ServiceRequest(service);
    long requestId = ++_requestsIndex;
    qiLogVerbose() << "Asynchronously asking service '" << service << "' to SD client. "
      "requestId = " << os::to_string(requestId);

    {
      boost::recursive_mutex::scoped_lock l(_requestsMutex);
      _requests[requestId] = rq;
    }
    rq->promise.setOnCancel(track([=](Promise<AnyObject>& p) mutable {
      removeRequest(requestId);
      p.setCanceled();
    }, this));
    qi::Future<qi::AnyObject> result = rq->promise.future();
    //rq is not valid anymore after addCallbacks, because it could have been handled and cleaned
    fut.connect(track([=](Future<ServiceInfo> fut) -> void
    {
      qiLogDebug() << "Got serviceinfo message";

      // Ensure that the promise is always set, even in case of exception.
      bool mustSetPromise = true;
      boost::optional<Promise<AnyObject>> promise;
      auto _ = ka::scoped(SetPromiseInError{*this, promise, mustSetPromise, requestId});

      {
        boost::recursive_mutex::scoped_lock sl(_requestsMutex);
        ServiceRequest *sr = serviceRequest(requestId);
        if (!sr)
        {
          logWarningUnknownServiceRequest("service() ServiceInfo continuation", requestId);
          return;
        }

        qiLogVerbose() << "Received answer from SD client for service '" << sr->name << "'. "
          "requestId = " << requestId;
        promise = sr->promise;

        if (fut.hasError())
        {
          setErrorAndRemoveRequest(*promise, fut.error(), requestId);
          return;
        }
        const qi::ServiceInfo& si = fut.value();
        sr->serviceId = si.serviceId();
        if (_sdClient->isLocal())
        { // Wait! If sd is local, we necessarily have an open socket
          // on which service was registered, whose lifetime is bound
          // to the service
          // TODO 40203: Could this block forever?
          MessageSocketPtr s = _sdClient->_socketOfService(sr->serviceId).value();

          if (!s) // weird
            qiLogVerbose() << "_socketOfService returned 0";
          else
          {
            // check if the socket support that capability
            if (s->remoteCapability(capabilityname::clientServerSocket, false))
            {
              qiLogVerbose() << "sd is local and service is capable, going through socketOfService";
              onTransportSocketResult(qi::Future<MessageSocketPtr>(s), requestId);
              mustSetPromise = false;
              return;
            }
          }
        }
        //empty serviceInfo
        if (!si.endpoints().size()) {
          std::stringstream ss;
          ss << "No endpoints returned for service:" << sr->name << " (id:" << sr->serviceId << ")";
          qiLogVerbose() << ss.str();
          setErrorAndRemoveRequest(*promise, ss.str(), requestId);
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
            setErrorAndRemoveRequest(*promise, ss.str(), requestId);
          }
        }
      }
      qiLogVerbose() << "Requesting socket from cache. service = '" << service << "', "
        "requestId = " << requestId;
      Future<qi::MessageSocketPtr> f = _socketCache->socket(fut.value(), protocol);
      f.connect(track(boost::bind(&Session_Service::onTransportSocketResult, this, _1, requestId), this));
      mustSetPromise = false;
    }, this));
    return result;
  }
}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
