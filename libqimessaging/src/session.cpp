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
#include <qimessaging/service_info.hpp>
#include "src/remoteobject_p.hpp"
#include "src/network_thread.hpp"
#include "src/session_p.hpp"

namespace qi {

  SessionInterface::~SessionInterface() {
  }

  SessionPrivate::SessionPrivate(qi::Session *session)
    : _serviceSocket(new qi::TransportSocket()),
      _networkThread(new qi::NetworkThread()),
      _self(session),
      _callbacks(0)
  {
    _serviceSocket->addCallbacks(this);
  }

  SessionPrivate::~SessionPrivate() {
    _networkThread->stop();
    _networkThread->destroy(true);
  }

  Session::~Session()
  {
    {
      boost::mutex::scoped_lock sl(_p->_mutexCallback);
      _p->_callbacks.clear();
    }
    disconnect();
    waitForDisconnected();
    delete _p;
  }

  void SessionPrivate::onSocketConnected(TransportSocket *client)
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
        msg.setPath(qi::Message::Path_Main);
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

  void SessionPrivate::onSocketConnectionError(TransportSocket *client)
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

  void SessionPrivate::onSocketDisconnected(TransportSocket *client)
  {
    std::vector<SessionInterface *> localCallbacks;
    {
      boost::mutex::scoped_lock l(_mutexCallback);
      localCallbacks = _callbacks;
    }
    std::vector<SessionInterface *>::const_iterator it;
    for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
      (*it)->onSessionDisconnected(_self);
    return;
  }

  void SessionPrivate::onSocketReadyRead(qi::TransportSocket *client, int id)
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
        onServiceRegistered(_self, serviceName);
      }
      else if (msg.function() == qi::Message::ServiceDirectoryFunction_UnregisterService)
      {
        onServiceUnregistered(_self, serviceName);
      }
      return;
    }


    // request for service method
    std::map<int, boost::shared_ptr<ServiceRequest> >::iterator futureServiceIt;
    {
      boost::mutex::scoped_lock l(_mutexFuture);
      futureServiceIt = _futureService.find(id);
    }
    if (futureServiceIt != _futureService.end())
    {
      if (client == _serviceSocket)
      {
        // Message comes from the ServiceDirectory with the endpoints of the Service
        // the client wants to connect to
        serviceEndpointEnd(id, client, &msg, futureServiceIt->second);
      }
      else
      {
        // The Message comes from the service and contains its MetaObject,
        // which is forwarded to the client.
        serviceMetaobjectEnd(id, client, &msg, futureServiceIt->second);
      }
      return;
    }


    // request from services method
    std::map<int, qi::Promise<std::vector<qi::ServiceInfo> > >::iterator futureServicesIt;
    {
      boost::mutex::scoped_lock l(_mutexFuture);
      futureServicesIt = _futureServices.find(id);
    }
    if (futureServicesIt != _futureServices.end())
    {
      servicesEnd(client, &msg, futureServicesIt->second);
      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureServices.erase(futureServicesIt);
      }
      return;
    }


    // Request for register/unregister methods
    std::map<int, qi::FunctorResult>::iterator futureFunctorIt;
    {
      boost::mutex::scoped_lock l(_mutexFuture);
      futureFunctorIt = _futureFunctor.find(id);
    }
    if (futureFunctorIt != _futureFunctor.end())
    {
      serviceRegisterUnregisterEnd(id, &msg, futureFunctorIt->second);
      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureFunctor.erase(futureFunctorIt);
      }
      return;
    }


    std::vector<unsigned int>::iterator serviceReadyIt;
    {
      boost::mutex::scoped_lock l(_mutexServiceReady);
      serviceReadyIt = std::find(_serviceReady.begin(), _serviceReady.end(), id);
    }
    if (serviceReadyIt != _serviceReady.end())
    {
      boost::mutex::scoped_lock l(_mutexServiceReady);
      _serviceReady.erase(serviceReadyIt);
      return;
    }

    qiLogError("qimessaging") << "Session::Private: onSocketReadyRead: unknown message id " << id;
  }

  void SessionPrivate::serviceRegisterUnregisterEnd(int id,
                                                    qi::Message *msg,
                                                    qi::FunctorResult promise)
  {
    if (!promise.isValid())
    {
      qiLogError("qimessaging.sessionprivate") << "No promise found for req id:" << id;
      return;
    }

    switch (msg->type())
    {
    case qi::Message::Type_Reply:
      promise.setValue(msg->buffer());
      break;
    case qi::Message::Type_Event:
      promise.setValue(msg->buffer());
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
          if (!ts->connect(_self, url))
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
        break;
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
                                   qi::Promise<std::vector<qi::ServiceInfo> > &promise)
  {
    std::vector<qi::ServiceInfo> result;
    qi::IDataStream d(msg->buffer());
    d >> result;
    if (d.status() == qi::IDataStream::Status_Ok)
      promise.setValue(result);
    else
      promise.setError("Serialization error");
  }

  qi::Future<unsigned int> SessionPrivate::registerService(const qi::ServiceInfo &si,
                                                           qi::Future<unsigned int> future)
  {
    qi::Message msg;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_RegisterService);

    qi::Buffer     buf;
    qi::ODataStream d(buf);
    d << si;

    if (d.status() == qi::ODataStream::Status_Ok)
    {
      msg.setBuffer(buf);

      qi::FunctorResult        ret;
      qi::makeFunctorResult<unsigned int>(&ret, &future);
      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureFunctor[msg.id()] = ret;
      }

      if (!_serviceSocket->send(msg))
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
      qi::Promise<unsigned int> prom;
      // FIXME: Maybe there is a better way
      // save all callbacks
      std::vector<std::pair<FutureInterface<unsigned int> *, void *> > callbacks = future.callbacks();

      future = prom.future();

      std::vector<std::pair<FutureInterface<unsigned int> *, void *> >::iterator it;
      for (it = callbacks.begin(); it != callbacks.end(); ++it)
        future.addCallbacks(it->first, it->second);

      prom.setError("serialization error");
    }
    return future;
  }

  qi::Future<void> SessionPrivate::unregisterService(unsigned int idx)
  {
    qi::Message msg;
    qi::Buffer  buf;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_UnregisterService);

    qi::ODataStream d(buf);
    d << idx;

    qi::Future<void> future;
    if (d.status() == qi::ODataStream::Status_Ok)
    {
      msg.setBuffer(buf);

      qi::FunctorResult ret;
      qi::makeFunctorResult<void>(&ret, &future);
      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureFunctor[msg.id()] = ret;
      }

      if (!_serviceSocket->send(msg))
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
    return future;
  }

  void SessionPrivate::onSocketTimeout(TransportSocket *client, int id)
  {
    {
      boost::mutex::scoped_lock l(_mutexFuture);
      std::map<int, qi::FunctorResult>::iterator it = _futureFunctor.find(id);
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
    msg.setPath(qi::Message::Path_Main);
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

      if (!_serviceSocket->send(msg))
      {
        qiLogError("qimessaging.Session") << "Error while ack service directory from service: "
                                          << idx;
      }
    }
  }

  void SessionPrivate::onServiceRegistered(Session *QI_UNUSED(session),
                                           const std::string &serviceName)
  {
    {
      boost::mutex::scoped_lock _sl(_watchedServicesMutex);
      std::map< std::string, std::pair< int, qi::Promise<void> > >::iterator it;
      it = _watchedServices.find(serviceName);
      if (it != _watchedServices.end())
        it->second.second.setValue(0);
    }

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

  void SessionPrivate::onServiceUnregistered(Session *QI_UNUSED(session),
                                             const std::string &serviceName) {
    {
      boost::mutex::scoped_lock _sl(_watchedServicesMutex);
      std::map< std::string, std::pair< int, qi::Promise<void> > >::iterator it;
      it = _watchedServices.find(serviceName);
      if (it != _watchedServices.end())
        it->second.second.setError(0);
    }

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
    return _p->_serviceSocket->connect(_p->_self, serviceDirectoryURL);
  }

  bool Session::disconnect()
  {
    _p->_serviceSocket->disconnect();
    return true;
  }

  bool Session::waitForConnected(int msecs)
  {
    return _p->_serviceSocket->waitForConnected(msecs);
  }

  bool Session::waitForDisconnected(int msecs)
  {
    return _p->_serviceSocket->waitForDisconnected(msecs);
  }

  qi::Future< std::vector<ServiceInfo> > Session::services()
  {
    qi::Promise<std::vector<ServiceInfo> > promise;
    qi::Message msg;

    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_Services);
    {
      boost::mutex::scoped_lock l(_p->_mutexFuture);
      _p->_futureServices[msg.id()] = promise;
    }
    if (!_p->_serviceSocket->send(msg))
    {
      promise.setError("Send failed (socket disconnected?)");
      _p->_futureService.erase(msg.id());
    }
    return promise.future();
  }

  qi::Future< qi::Object * > Session::service(const std::string &service,
                                              const std::string &type)
  {
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
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_Service);
    qi::ODataStream dr(buf);
    dr << service;
    {
      boost::mutex::scoped_lock l(_p->_mutexFuture);
      _p->_futureService[msg.id()] = sr;
    }
    if (!_p->_serviceSocket->send(msg));
    {
      sr->promise.setError("Send failed (socket disconnected?)");
      _p->_futureService.erase(msg.id());
    }
    return sr->promise.future();
  }

  bool Session::join()
  {
    _p->_networkThread->join();
    return true;
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
    return _p->_serviceSocket->isConnected();
  }

  qi::Url Session::url() const {
    return _p->_serviceSocket->url();
  }

  bool Session::waitForServiceReady(const std::string &service, int msecs) {
    qi::Future< std::vector<ServiceInfo> > svs;
    std::vector<ServiceInfo>::iterator     it;
    qi::Promise<void>                      prom;
    qi::Future<void>                       fut;

    //register a watcher for the service
    {
      boost::mutex::scoped_lock _sl(_p->_watchedServicesMutex);
      std::map< std::string, std::pair< int, qi::Promise<void> > >::iterator it;
      it = _p->_watchedServices.find(service);
      if (it == _p->_watchedServices.end()) {
        fut = prom.future();
        it = _p->_watchedServices.insert(std::make_pair(service, std::make_pair(0, prom))).first;
      } else {
        fut = it->second.second.future();
      }
      it->second.first += 1;
    }

    svs = services();
    svs.wait(msecs);
    if (!svs.isReady()) {
      qiLogVerbose("qi.Session") << "waitForServiceReady failed because session.services did not return.";
      return false;
    }
    for (it = svs.value().begin(); it != svs.value().end(); ++it) {
      if (it->name() == service)
        return true;
    }
    fut.wait(msecs);
    //no error mean service found
    bool found = (fut.isReady() && fut.hasError() == 0);

    {
      boost::mutex::scoped_lock _sl(_p->_watchedServicesMutex);
      std::map< std::string, std::pair< int, qi::Promise<void> > >::iterator it;
      it = _p->_watchedServices.find(service);
      if (it != _p->_watchedServices.end()) {
        it->second.first -= 1;
      }
      if (it->second.first == 0)
        _p->_watchedServices.erase(it);
    }
    return found;
  }

}
