/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Herve Cuche <hcuche@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/session.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/object_factory.hpp>
#include <qimessaging/service_info.hpp>
#include "src/remoteobject_p.hpp"
#include "src/session_p.hpp"
#include "src/object_p.hpp"

namespace qi {

  static MetaFunctionResult forwardEvent(const MetaFunctionParameters& params,
    unsigned int service, unsigned int event, TransportSocket* client)
  {
    qi::Message msg;
    msg.setBuffer(params.getBuffer());
    msg.setService(service);
    msg.setFunction(event);
    msg.setType(Message::Type_Event);
    msg.setObject(Message::Object_Main);
    client->send(msg);
    return MetaFunctionResult();
  }


  SessionInterface::~SessionInterface() {
  }

  SessionPrivate::SessionPrivate(qi::Session *session)
    : _serviceSocket(),
      _self(session),
      _callbacks(0)
    , _dying(false)
    , _server(this)
    , _client(this)
    , _watcher(session)
  {
    _serviceSocket.addCallbacks(this);
    _ts.addCallbacks(this);
  }

  SessionServer::SessionServer(SessionPrivate* self)
    : _self(self)
  {
  }

  SessionClient::SessionClient(SessionPrivate* self)
    : _self(self)
  {
  }

  SessionPrivate::~SessionPrivate() {
    _dying = true;
    boost::recursive_mutex::scoped_lock sl(_mutexOthers);
    for (std::set<TransportSocket*>::iterator i = _clients.begin();
      i != _clients.end(); ++i)
    {
      // We do not want onSocketDisconnected called
      (*i)->removeCallbacks(this);
      delete *i;
    }
  }

  Session::~Session()
  {
    {
      boost::mutex::scoped_lock sl(_p->_mutexCallback);
      _p->_callbacks.clear();
    }
    close();
    waitForDisconnected();
    delete _p;
  }

  void SessionPrivate::onSocketConnected(TransportSocket *client, void *data)
  {
    // Come from connection between transport socket and service
    std::map<void *, boost::shared_ptr<ServiceRequest> >::iterator it = _futureConnect.find(client);
    if (it != _futureConnect.end())
    {
      qi::Message msg;
      boost::shared_ptr<ServiceRequest> sr = it->second;
      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureConnect.erase(client);
        if (sr->connected)
        {
          // An other attempt got here first, disconnect and drop
          client->disconnect();
          return;
        }
        sr->connected = true;
        msg.setType(qi::Message::Type_Call);
        msg.setService(sr->serviceId);
        msg.setObject(qi::Message::Object_Main);
        msg.setFunction(qi::Message::Function_MetaObject);
        _futureService[msg.id()] = sr;
      }
      client->send(msg);
      return;
    }

    {
      // Comes from session connected to ServiceDirectory
      std::vector<SessionInterface *> localCallbacks;
      {
        boost::mutex::scoped_lock l(_mutexCallback);
        localCallbacks = _callbacks;
      }
      std::vector<SessionInterface *>::const_iterator it;
      for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
        (*it)->onSessionConnected(_self);
      return;
    }
  }

  void SessionPrivate::onSocketConnectionError(TransportSocket *client, void *data)
  {
    boost::mutex::scoped_lock l(_mutexFuture);
    std::map<void *, boost::shared_ptr<ServiceRequest> >::iterator it = _futureConnect.find(client);
    if (it != _futureConnect.end())
    {
      boost::shared_ptr<ServiceRequest> sr = it->second;
      --sr->attempts;
      if (!sr->attempts)
      {
        // This was the last connection attempt, and it failed.
        sr->promise.setError("Failed to connect to service");
      }
      _futureConnect.erase(it);
      delete client;
      return;
    }

    std::vector<SessionInterface *> localCallbacks;
    {
      boost::mutex::scoped_lock l(_mutexCallback);
      localCallbacks = _callbacks;
    }
    std::vector<SessionInterface *>::const_iterator it2;
    for (it2 = localCallbacks.begin(); it2 != localCallbacks.end(); ++it2)
      (*it2)->onSessionConnectionError(_self);
    return;
  }

  void SessionServer::onSocketDisconnected(TransportSocket *client)
  {
    // The check below must be done before holding the lock.
    if (_self->_dying)
      return;
    boost::recursive_mutex::scoped_lock sl(_self->_mutexOthers);
    _self->_clients.erase(client);
    // Disconnect event links set for this client.
    SessionPrivate::Links::iterator i = _self->_links.find(client);
    if (i != _self->_links.end())
    {
      // Iterate per service
      for (SessionPrivate::PerServiceLinks::iterator j = i->second.begin();
           j != i->second.end(); ++j)
      {
        std::map<unsigned int, qi::Object*>::iterator iservice = _self->_services.find(j->first);
        // If the service is still there, disconnect one by one.
        if (iservice != _self->_services.end())
          for (SessionPrivate::ServiceLinks::iterator k = j->second.begin();
               k != j->second.end(); ++k)
            iservice->second->disconnect(k->second.localLinkId);
      }
      _self->_links.erase(i);
    }
    delete client;
  }

  void SessionClient::onSocketDisconnected(TransportSocket *client)
  {
    std::vector<SessionInterface *> localCallbacks;
    {
      boost::mutex::scoped_lock l(_self->_mutexCallback);
      localCallbacks = _self->_callbacks;
    }
    std::vector<SessionInterface *>::const_iterator it;
    for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
      (*it)->onSessionDisconnected(_self->_self);
  }

  void SessionPrivate::onSocketDisconnected(TransportSocket *client, void *data)
  {
    if (_clients.find(client) != _clients.end())
    {
      _server.onSocketDisconnected(client);
    }
    else
    {
      _client.onSocketDisconnected(client);
    }
  }

  void SessionServer::onSocketReadyRead(TransportSocket *client, int id)
  {
    qi::Message msg;
    client->read(id, &msg);
    qi::Object *obj;

    {
      boost::mutex::scoped_lock sl(_self->_mutexServices);
      std::map<unsigned int, qi::Object*>::iterator it;
      if (msg.service() == Message::Service_Server)
      {
        // Accept register/unregister event as emit or as call
        if (msg.type() != Message::Type_Event
          && msg.type() != Message::Type_Call)
        {
          qiLogError("qi::Server") << "Server service only handles call/emit";
          qi::Message retval;
          retval.buildReplyFrom(msg);
          Buffer buf;
          ODataStream(buf) << "Server service only handles call/emit";
          retval.setBuffer(buf);
          client->send(retval);
          return;
        }
        // First arg is always a service id, so factor a bi there
        IDataStream ds(msg.buffer());
        unsigned int service;
        ds >> service;
        it = _self->_services.find(service);
        if (it == _self->_services.end())
        {
          if (msg.type() == Message::Type_Call)
          {
            qi::Message retval;
            retval.buildReplyFrom(msg);
            Buffer buf;
            ODataStream(buf) << "Service not found";
            retval.setBuffer(buf);
            client->send(retval);
          }
          return;
        }
        switch(msg.function())
        {

        case Message::ServerFunction_RegisterEvent:
          {
            unsigned int event, remoteLinkId;
            ds >> event >> remoteLinkId;

            // locate object, register locally and bounce to an event message
            unsigned int linkId = it->second->connect(event,
                boost::bind(&forwardEvent, _1, service, event, client));
            _self->_links[client][service][remoteLinkId] = SessionPrivate::RemoteLink(linkId, event);
            if (msg.type() == Message::Type_Call)
            {
              qi::Message retval;
              retval.buildReplyFrom(msg);
              Buffer buf;
              ODataStream ds(buf);
              ds << linkId;
              retval.setBuffer(buf);
              client->send(retval);
            }
          }
          break;
        case Message::ServerFunction_UnregisterEvent:
          {
            unsigned int event, remoteLinkId;
            ds >> event >> remoteLinkId;
            SessionPrivate::ServiceLinks& sl = _self->_links[client][service];
            SessionPrivate::ServiceLinks::iterator i = sl.find(remoteLinkId);
            if (i == sl.end())
            {
              qiLogError("qi::Server") << "Unregister request failed for "
              << remoteLinkId <<" " << service;
            }
            else
            {
              it->second->disconnect(i->second.localLinkId);
            }
            if (msg.type() == Message::Type_Call)
            {
              qi::Message retval;
              retval.buildReplyFrom(msg);
              Buffer buf;
              ODataStream ds(buf);
              ds << (i == sl.end());
              retval.setBuffer(buf);
              client->send(retval);
            }
          }
          break;
        }
        return;
      } // msg.service() == Server
      it = _self->_services.find(msg.service());
      obj = it->second;
      if (it == _self->_services.end() || !obj)
      {
        if (msg.type() == qi::Message::Type_Call) {
          qi::Message retval;
          retval.buildReplyFrom(msg);
          qi::Buffer error;
          qi::ODataStream ds(error);
          std::stringstream ss;
          ss << "can't find service id: " << id;
          ds << ss.str();
          retval.setBuffer(error);
          client->send(retval);

        }
        qiLogError("qi::Server") << "Can't find service: " << msg.service();
        return;
      }
    }
    switch (msg.type())
    {
    case Message::Type_Call:
      {
           qi::Future<MetaFunctionResult> fut = obj->metaCall(msg.function(), MetaFunctionParameters(msg.buffer()), qi::Object::MetaCallType_Queued);
           fut.addCallbacks(new detail::ServerResult(client, msg));
      }
      break;
    case Message::Type_Event:
      {
          obj->metaEmit(msg.function(), MetaFunctionParameters(msg.buffer()));
      }
      break;
    }

  }

  void SessionClient::onSocketReadyRead(TransportSocket *client, int id)
  {
    qi::Message msg;
    client->read(id, &msg);

    if (msg.type() == qi::Message::Type_Event)
    {
      std::string serviceName;
      qi::IDataStream  d(msg.buffer());
      d >> serviceName;

      if (msg.function() == qi::Message::ServiceDirectoryFunction_RegisterService)
      {
        _self->onServiceRegistered(_self->_self, serviceName);
      }
      else if (msg.function() == qi::Message::ServiceDirectoryFunction_UnregisterService)
      {
        _self->onServiceUnregistered(_self->_self, serviceName);
      }
      return;
    }


    // request for service method
    std::map<int, boost::shared_ptr<ServiceRequest> >::iterator futureServiceIt;
    {
      boost::mutex::scoped_lock l(_self->_mutexFuture);
      futureServiceIt = _self->_futureService.find(id);
    }
    if (futureServiceIt != _self->_futureService.end())
    {
      if (client == &_self->_serviceSocket)
      {
        // Message comes from the ServiceDirectory with the endpoints of the Service
        // the client wants to connect to
        _self->serviceEndpointEnd(id, client, &msg, futureServiceIt->second);
      }
      else
      {
        // The Message comes from the service and contains its MetaObject,
        // which is forwarded to the client.
        _self->serviceMetaobjectEnd(id, client, &msg, futureServiceIt->second);
      }
      return;
    }


    // request from services method
    std::map<int, SessionPrivate::ServicesPromiseLocality>::iterator futureServicesIt;
    {
      boost::mutex::scoped_lock l(_self->_mutexFuture);
      futureServicesIt = _self->_futureServices.find(id);
    }
    if (futureServicesIt != _self->_futureServices.end())
    {
      _self->servicesEnd(client, &msg, futureServicesIt->second.first, futureServicesIt->second.second);
      {
        boost::mutex::scoped_lock l(_self->_mutexFuture);
        _self->_futureServices.erase(futureServicesIt);
      }
      return;
    }


    // Request for register/unregister methods
    std::map<int, qi::Promise<MetaFunctionResult> >::iterator futureFunctorIt;
    {
      boost::mutex::scoped_lock l(_self->_mutexFuture);
      futureFunctorIt = _self->_futureFunctor.find(id);
    }
    if (futureFunctorIt != _self->_futureFunctor.end())
    {
      _self->serviceRegisterUnregisterEnd(id, &msg, futureFunctorIt->second);
      {
        boost::mutex::scoped_lock l(_self->_mutexFuture);
        _self->_futureFunctor.erase(futureFunctorIt);
      }
      return;
    }


    std::vector<unsigned int>::iterator serviceReadyIt;
    {
      boost::mutex::scoped_lock l(_self->_mutexServiceReady);
      serviceReadyIt = std::find(_self->_serviceReady.begin(), _self->_serviceReady.end(), id);
    }
    if (serviceReadyIt != _self->_serviceReady.end())
    {
      boost::mutex::scoped_lock l(_self->_mutexServiceReady);
      _self->_serviceReady.erase(serviceReadyIt);
      return;
    }

    qiLogError("qimessaging") << "Session::Private: onSocketReadyRead: unknown message id " << id;
  }

  void SessionPrivate::onSocketReadyRead(qi::TransportSocket *client, int id, void *data)
  {
    if (_clients.find(client) != _clients.end())
    {
      _server.onSocketReadyRead(client, id);
    }
    else
    {
      _client.onSocketReadyRead(client, id);
    }
  }

  void SessionPrivate::serviceRegisterUnregisterEnd(int id,
                                                    qi::Message *msg,
                                                    qi::Promise<MetaFunctionResult> promise)
  {
    switch (msg->type())
    {
    case qi::Message::Type_Reply:
    case qi::Message::Type_Event:
      {
        MetaFunctionResult res(msg->buffer());
        promise.setValue(res);
      }
      break;
    default:
      {
        promise.setError("Bad message type");
        qiLogError("qimessaging.sessionprivate") << "Message (#" << id << ") type not handled: "
                                                 << msg->type();
        return;
      }
    }
  }

  void SessionPrivate::serviceEndpointEnd(int id,
                                          qi::TransportSocket *QI_UNUSED(client),
                                          qi::Message *msg,
                                          boost::shared_ptr<ServiceRequest> sr)
  {
    sr->attempts = 0;
    // Decode ServiceInfo in the message
    qi::ServiceInfo si;
    qi::IDataStream  d(msg->buffer());
    d >> si;
    if (d.status() == qi::IDataStream::Status_Ok)
    {
      sr->serviceId = si.serviceId();
      // Attempt to connect to all endpoints.
      for (std::vector<std::string>::const_iterator it = si.endpoints().begin();
           it != si.endpoints().end();
           ++it)
      {
        qi::Url url(*it);
        if (sr->protocol == "any" || sr->protocol == url.protocol())
        {
          qi::TransportSocket *ts = NULL;
          ts = new qi::TransportSocket();
          ts->addCallbacks(this);
          {
            boost::mutex::scoped_lock l(_mutexFuture);
            _futureConnect[ts] = sr;
            _futureService.erase(id);
          }
          if (!ts->connect(url))
          {
            // Synchronous failure, do nothing, try next
            delete ts;
            continue;
          }
          else
          {
            // The connect may still fail asynchronously.
            boost::mutex::scoped_lock l(_mutexFuture);
            ++sr->attempts;
          }
        }
      }
    }
    if (!sr->attempts)
    {
      // All attempts failed synchronously.
      boost::mutex::scoped_lock l(_mutexFuture);
      sr->promise.setError("No service found");
      _futureService.erase(id);
    }
    return;
  }

  void SessionPrivate::serviceMetaobjectEnd(int QI_UNUSED(id),
                                            qi::TransportSocket *client,
                                            qi::Message *msg,
                                            boost::shared_ptr<ServiceRequest> sr)
  {
    qi::MetaObject *mo = new qi::MetaObject();
    qi::IDataStream  d(msg->buffer());
    d >> *mo;
    {
      boost::mutex::scoped_lock l(_mutexFuture);
      _futureService.erase(msg->id());
    }
    if (d.status() == qi::IDataStream::Status_Ok)
    {
      client->removeCallbacks(this);
      qi::RemoteObject *robj = new qi::RemoteObject(client, sr->serviceId, mo);
      qi::Object *obj;
      obj = robj;
      sr->promise.setValue(obj);
    } else {
      sr->promise.setError("Serialization error");
    }
  }

  void SessionPrivate::servicesEnd(qi::TransportSocket *QI_UNUSED(client),
                                   qi::Message *msg,
                                   qi::Session::ServiceLocality locality,
                                   qi::Promise<std::vector<qi::ServiceInfo> > &promise)
  {
    std::vector<qi::ServiceInfo> result;
    qi::IDataStream d(msg->buffer());
    d >> result;
    if (locality == qi::Session::ServiceLocality_All)
    {
      std::map<std::string, qi::ServiceInfo>::iterator it;
      {
        boost::recursive_mutex::scoped_lock sl(_mutexOthers);
        for (it = _servicesInfo.begin(); it != _servicesInfo.end(); ++it) {
          result.push_back(it->second);
        }
      }
    }
    if (d.status() == qi::IDataStream::Status_Ok)
      promise.setValue(result);
    else
      promise.setError("Serialization error");
  }

  qi::Future<unsigned int> SessionPrivate::registerService(const qi::ServiceInfo &si)
  {
    qi::Promise<unsigned int> ret;
    qi::Message msg;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setObject(qi::Message::Object_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_RegisterService);

    qi::Buffer     buf;
    qi::ODataStream d(buf);
    d << si;

    if (d.status() == qi::ODataStream::Status_Ok)
    {
      msg.setBuffer(buf);
      Promise<MetaFunctionResult> promise;
      promise.future().addCallbacks(new detail::FutureAdapter<unsigned int>(ret));
      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureFunctor[msg.id()] = promise;
      }

      if (!_serviceSocket.send(msg))
      {
        qi::IDataStream dout(msg.buffer());
        qi::ServiceInfo si;
        dout >> si;
        qiLogError("qimessaging.Session") << "Error while register service: "
                                          << si.name() << " request";
        std::stringstream ss;
        ss << "Error while register service: "
           << si.name() << " request";
        ret.setError(ss.str());
      }
    } else {

      ret.setError("serialization error");
    }
    return ret.future();
  }

  qi::Future<void> SessionPrivate::unregisterService(unsigned int idx)
  {
    qi::Promise<void> ret;
    qi::Message msg;
    qi::Buffer  buf;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setObject(qi::Message::Object_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_UnregisterService);

    qi::ODataStream d(buf);
    d << idx;

    qi::Future<void> future;
    if (d.status() == qi::ODataStream::Status_Ok)
    {
      msg.setBuffer(buf);
      qi::Promise<MetaFunctionResult> promise;
      promise.future().addCallbacks(new detail::FutureAdapter<void>(ret));

      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureFunctor[msg.id()] = promise;
      }

      if (!_serviceSocket.send(msg))
      {
        qi::IDataStream dout(msg.buffer());
        unsigned int id;
        dout >> id;
        qiLogError("qimessaging.Session") << "Error while unregister serviceId: "
                                          << id << " request";

        std::stringstream ss;
        ss << "Error while unregister serviceId: "
           << id << " request";
        ret.setError(ss.str());
      }
    }
    return ret.future();
  }

  void SessionPrivate::onSocketTimeout(TransportSocket *client, int id, void *data)
  {
    {
      boost::mutex::scoped_lock l(_mutexFuture);
      std::map<int, Promise<MetaFunctionResult> >::iterator it = _futureFunctor.find(id);
      if (it != _futureFunctor.end())
      {
        it->second.setError("network timeout");
        _futureFunctor.erase(it);
      }
    }
  }

  void SessionPrivate::serviceReady(unsigned int idx)
  {
    if (idx == 0)
    {
      qiLogError("qimessaging.Session") << "called serviceReady with id #0";
      return;
    }

    qi::Message msg;
    qi::Buffer  buf;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setObject(qi::Message::Object_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_ServiceReady);

    qi::ODataStream d(buf);
    d << idx;

    if (d.status() == qi::ODataStream::Status_Ok)
    {
      msg.setBuffer(buf);
      {
        boost::mutex::scoped_lock l(_mutexServiceReady);
        _serviceReady.push_back(msg.id());
      }

      if (!_serviceSocket.send(msg))
      {
        qiLogError("qimessaging.Session") << "Error while ack service directory from service: "
                                          << idx;
      }
    }
  }

  void SessionPrivate::onServiceRegistered(Session *session, const std::string &serviceName)
  {
    _watcher.onServiceRegistered(session, serviceName);

    std::vector<SessionInterface *> localCallbacks;
    {
      boost::mutex::scoped_lock l(_mutexCallback);
      localCallbacks = _callbacks;
    }
    std::vector<SessionInterface *>::const_iterator localCallbacksIt;
    for (localCallbacksIt = localCallbacks.begin(); localCallbacksIt != localCallbacks.end(); ++localCallbacksIt)
    {
      (*localCallbacksIt)->onServiceRegistered(_self, serviceName);
    }
  }

  void SessionPrivate::onServiceUnregistered(Session *session, const std::string &serviceName)
  {
    _watcher.onServiceUnregistered(session, serviceName);

    std::vector<SessionInterface *> localCallbacks;
    {
      boost::mutex::scoped_lock l(_mutexCallback);
      localCallbacks = _callbacks;
    }
    std::vector<SessionInterface *>::const_iterator localCallbacksIt;
    for (localCallbacksIt = localCallbacks.begin(); localCallbacksIt != localCallbacks.end(); ++localCallbacksIt)
    {
      (*localCallbacksIt)->onServiceUnregistered(_self, serviceName);
    }
  }


  // ###### Session
  Session::Session()
    : _p(new SessionPrivate(this))
  {
  }


  bool Session::connect(const qi::Url &serviceDirectoryURL)
  {
    return _p->_serviceSocket.connect(serviceDirectoryURL);
  }

  bool Session::waitForConnected(int msecs)
  {
    return _p->_serviceSocket.waitForConnected(msecs);
  }

  bool Session::waitForDisconnected(int msecs)
  {
    return _p->_serviceSocket.waitForDisconnected(msecs);
  }

  //3 cases:
  //  - local service => just return the vector
  //  - remote => ask the sd return the result
  //  - all => ask the sd, append local services, return the result
  qi::Future< std::vector<ServiceInfo> > Session::services(ServiceLocality locality)
  {
    qi::Promise<std::vector<ServiceInfo> > promise;
    if (locality == ServiceLocality_Local) {
      std::vector<qi::ServiceInfo> ssi;
      std::map<std::string, qi::ServiceInfo>::iterator it;

      {
        boost::recursive_mutex::scoped_lock sl(_p->_mutexOthers);
        for (it = _p->_servicesInfo.begin(); it != _p->_servicesInfo.end(); ++it) {
          ssi.push_back(it->second);
        }
      }
      promise.setValue(ssi);
      return promise.future();
    }

    qi::Message msg;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setObject(qi::Message::Object_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_Services);
    {
      boost::mutex::scoped_lock l(_p->_mutexFuture);
      SessionPrivate::ServicesPromiseLocality spl = std::make_pair(locality, promise);
      _p->_futureServices[msg.id()] = spl;
    }
    if (!_p->_serviceSocket.send(msg))
    {
      promise.setError("Send failed (socket disconnected?)");
      _p->_futureService.erase(msg.id());
    }
    return promise.future();
  }

  qi::Future< qi::Object * > Session::service(const std::string &service,
                                              ServiceLocality locality,
                                              const std::string &type)
  {
    if (locality == ServiceLocality_Local)
      qiLogError("session.service") << "service is not implemented for local service, it always return a remote service";

    boost::shared_ptr<ServiceRequest> sr(new ServiceRequest);
    qi::Message    msg;
    qi::Buffer     buf;

    msg.setBuffer(buf);
    sr->name      = service;
    sr->protocol  = type;
    sr->connected  = false;
    sr->attempts = 0;

    // Ask a ServiceInfo to the ServiceDirectory.
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setObject(qi::Message::Object_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_Service);
    qi::ODataStream dr(buf);
    dr << service;
    {
      boost::mutex::scoped_lock l(_p->_mutexFuture);
      _p->_futureService[msg.id()] = sr;
    }
    if (!_p->_serviceSocket.send(msg))
    {
      sr->promise.setError("Send failed (socket disconnected?)");
      _p->_futureService.erase(msg.id());
    }
    return sr->promise.future();
  }

  void Session::addCallbacks(SessionInterface *delegate)
  {
    if (delegate)
    {
      boost::mutex::scoped_lock l(_p->_mutexCallback);
      _p->_callbacks.push_back(delegate);
    }
    else
      qiLogError("qimessaging.Session") << "Trying to set invalid callback on the session.";
  }

  void Session::removeCallbacks(SessionInterface *delegate)
  {
    if (delegate)
    {
      boost::mutex::scoped_lock l(_p->_mutexCallback);
      std::vector<SessionInterface *>::iterator it;
      for (it = _p->_callbacks.begin(); it != _p->_callbacks.end(); ++it)
      {
        if (*it == delegate)
        {
          _p->_callbacks.erase(it);
          break;
        }
      }
    }
    else
      qiLogError("qimessaging.Session") << "Trying to erase invalid callback on the session.";
  }

  bool Session::isConnected() const {
    return _p->_serviceSocket.isConnected();
  }

  qi::Url Session::url() const {
    return _p->_serviceSocket.url();
  }

  bool Session::waitForServiceReady(const std::string &service, int msecs) {
    return _p->_watcher.waitForServiceReady(service, msecs);
  }

  bool Session::listen(const std::string &address)
  {
    qi::Url url(address);

    if (url.protocol() != "tcp") {
      qiLogError("qi::Server") << "Protocol " << url.protocol() << " not supported.";
      return false;
    }
    if (!_p->_ts.listen(url))
      return false;
    qiLogVerbose("qimessaging.Server") << "Started Server at " << _p->_ts.listenUrl().str();
    return true;
  }

  void Session::close()
  {
    _p->_serviceSocket.disconnect();
    return _p->_ts.close();
  }

  qi::Future<unsigned int> Session::registerService(const std::string &name,
                                                    qi::Object *obj)
  {
    if (_p->_ts.endpoints().empty()) {
      qiLogError("qimessaging.Server") << "Could not register service: " << name << " because the current server has not endpoint";
      return qi::Future<unsigned int>();
    }
    qi::ServiceInfo si;
    si.setName(name);
    si.setProcessId(qi::os::getpid());
    si.setMachineId(qi::os::getMachineId());

    {
      std::vector<qi::Url> epsUrl = _p->_ts.endpoints();
      std::vector<std::string> epsStr;
      for (std::vector<qi::Url>::const_iterator epsUrlIt = epsUrl.begin();
           epsUrlIt != epsUrl.end();
           epsUrlIt++)
      {
        epsStr.push_back((*epsUrlIt).str());
      }
      si.setEndpoints(epsStr);
    }

    {
      boost::recursive_mutex::scoped_lock sl(_p->_mutexOthers);
      _p->_servicesObject[obj] = si;
    }

    qi::Future<unsigned int> future =  _p->registerService(si);
    future.addCallbacks(_p, obj);
    return future;
  }

  qi::Future<void> Session::unregisterService(unsigned int idx)
  {
    qi::Future<void> future = _p->unregisterService(idx);

    {
      boost::mutex::scoped_lock sl(_p->_mutexServices);
      _p->_services.erase(idx);
    }
    {
      boost::recursive_mutex::scoped_lock sl(_p->_mutexOthers);
      std::map<unsigned int, std::string>::iterator it;
      it = _p->_servicesIndex.find(idx);
      if (it == _p->_servicesIndex.end()) {
        qiLogError("qimessaging.Server") << "Can't find name associated to id:" << idx;
      }
      else {
        _p->_servicesByName.erase(it->second);
        _p->_servicesInfo.erase(it->second);
      }
      _p->_servicesIndex.erase(idx);
    }
    return future;
  }

  qi::Url Session::listenUrl() const
  {
    return _p->_ts.listenUrl();
  }

  void SessionPrivate::newConnection(TransportServer* server, TransportSocket *socket)
  {
    boost::recursive_mutex::scoped_lock sl(_mutexOthers);
    if (!socket)
      return;
    _clients.insert(socket);
    socket->addCallbacks(this);
  }

  void SessionPrivate::onFutureFailed(const std::string &error, void *data)
  {
    qi::ServiceInfo si;
    qi::Object     *obj = static_cast<qi::Object *>(data);

    std::map<qi::Object *, qi::ServiceInfo>::iterator it = _servicesObject.find(obj);
    if (it != _servicesObject.end())
      _servicesObject.erase(it);
  }

  void SessionPrivate::onFutureFinished(const unsigned int &idx,
                                       void               *data)
  {
    qi::Object     *obj = static_cast<qi::Object *>(data);
    qi::ServiceInfo si;
    std::map<qi::Object *, qi::ServiceInfo>::iterator it;

    {
      boost::recursive_mutex::scoped_lock sl(_mutexOthers);
      it = _servicesObject.find(obj);
      if (it != _servicesObject.end())
        si = _servicesObject[obj];
    }
    si.setServiceId(idx);

    {
      boost::mutex::scoped_lock sl(_mutexServices);
      _services[idx] = obj;
    }
    // ack the Service directory to tell that we are ready
    _self->_p->serviceReady(idx);
    {
      boost::recursive_mutex::scoped_lock sl(_mutexOthers);
      _servicesInfo[si.name()] = si;
      _servicesByName[si.name()] = obj;
      _servicesIndex[idx] = si.name();
      _servicesObject.erase(it);
    }
  }

  std::vector<std::string> Session::loadService(const std::string& name, int flags)
  {
    std::vector<std::string> names = ::qi::loadObject(name, flags);
    for (unsigned int i=0; i<names.size(); ++i)
      registerService(names[i], createObject(names[i]));
    return names;
  }
}
