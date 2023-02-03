/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/anyobject.hpp>
#include "transportserver.hpp"
#include <qi/messaging/serviceinfo.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include "server.hpp"
#include "objectregistrar.hpp"
#include <qi/os.hpp>
#include <boost/thread/mutex.hpp>
#include "servicedirectoryclient.hpp"
#include "session_p.hpp"
#include <ka/scoped.hpp>

qiLogCategory("qimessaging.objectregistrar");

namespace qi {

  BoundService::~BoundService()
  {
    // if we own the object, destroy it asynchronously because it may hold the
    // last reference to Session which leads to a deadlock (Session can't be
    // destroyed because it is calling us)
    if (object.unique())
      qi::async(boost::bind(&qi::detail::hold<qi::AnyObject>, object));
  }

  ObjectRegistrar::ObjectRegistrar(ServiceDirectoryClient* sdClient,
                                   ssl::ServerConfig sslConfig,
                                   boost::optional<AuthProviderFactoryPtr> authProviderFactory)
    : Server(std::move(sslConfig), std::move(authProviderFactory))
    , _sdClient(sdClient)
    , _id(qi::os::generateUuid())
  {
    _server.endpointsChanged.connect(
      track(boost::bind(&ObjectRegistrar::updateServiceInfo, this), &_tracker));
  }

  ObjectRegistrar::~ObjectRegistrar()
  {
    _tracker.destroy();
    close();
  }

  void ObjectRegistrar::close()
  {
    BoundServiceMap services;
    {
      boost::mutex::scoped_lock sl(_servicesMutex);
      services = _services;
    }
    for (BoundServiceMap::reverse_iterator iter = services.rbegin();
        iter != services.rend();
        ++iter)
      unregisterService(iter->first);
    Server::close().value();
  }

  void serviceReady(qi::Future<void> fut, qi::Promise<unsigned int> result, unsigned int idx) {
    if (fut.hasError()) {
      result.setError(fut.error());
      return;
    }
    result.setValue(idx);
  }

  void ObjectRegistrar::updateServiceInfo()
  {
    qi::ServiceInfo si;
    si.setProcessId(qi::os::getpid());
    si.setMachineId(qi::os::getMachineId());
    si.setEndpoints(Server::endpoints().value());
    si.setSessionId(_id);

    boost::mutex::scoped_lock sl(_servicesMutex);
    std::map<unsigned, BoundService>::iterator it = _services.begin();
    if (it != _services.end()) {
      BoundService& bs = it->second;
      si.setServiceId(bs.id);
      si.setName(bs.name);
      si.setObjectUid(bs.serviceInfo.objectUid());
      _sdClient->updateServiceInfo(si);
    }
  }

  void ObjectRegistrar::onFutureFinished(qi::Future<unsigned int> fut, int id, qi::Promise<unsigned int> result)
  {
    auto eraseRequest = ka::scoped([id, this]{
      boost::mutex::scoped_lock sl(_registerServiceRequestMutex);
      _registerServiceRequest.erase(id);
    });

    if (fut.hasError()) {
      result.setError(fut.error());
      return;
    }
    qi::ServiceInfo              si;
    RegisterServiceMap::iterator it;

    {
      boost::mutex::scoped_lock sl(_registerServiceRequestMutex);
      it = _registerServiceRequest.find(id);
      if (it != _registerServiceRequest.end())
        si = it->second.second;
    }
    unsigned int idx = fut.value();
    si.setServiceId(idx);
    {
      boost::mutex::scoped_lock sl(_servicesMutex);
      BoundService bs;
      bs.id          = idx;
      bs.object      = it->second.first;
      bs.serviceInfo = si;
      bs.name        = si.name();
      BoundServiceMap::iterator it;
      it = _services.find(idx);
      if (it != _services.end()) {
        qiLogError() << "A service is already registered with that id:" << idx;
        result.setError("Service already registered.");
        return;
      }
      _services[idx] = bs;
      //todo register the object on the server (find a better way)
      Server::addObject(idx, bs.object);
    }

    {
      boost::mutex::scoped_lock sl(_serviceNameToIndexMutex);
      _serviceNameToIndex[si.name()] = idx;
    }

    // ack the Service directory to tell that we are ready
    qi::Future<void> fut2 = _sdClient->serviceReady(idx);
    fut2.connect(boost::bind(&serviceReady, _1, result, idx));
  }

  qi::Future<unsigned int> ObjectRegistrar::registerService(const std::string &name, qi::AnyObject obj)
  {
    qi::Promise<unsigned int> prom;
    Server::endpoints().andThen(track([=](const std::vector<qi::Uri>& endpoints) mutable {
      if (endpoints.empty()) {
        const auto error = "Could not register service: " + name + " because the current server has not endpoint";
        prom.setError(error);
        return;
      }
      qi::ServiceInfo si;
      si.setName(name);
      si.setProcessId(qi::os::getpid());
      si.setMachineId(qi::os::getMachineId());
      si.setEndpoints(endpoints);
      si.setSessionId(_id);
      si.setObjectUid(serializeObjectUid<std::string>(obj.uid()));

      int id = ++_registerServiceRequestIndex;
      {
        boost::mutex::scoped_lock sl(_registerServiceRequestMutex);
        _registerServiceRequest[id] = std::make_pair(obj, si);
      }

      auto future = _sdClient->registerService(si);
      future.connect(
        track(boost::bind<void>(&ObjectRegistrar::onFutureFinished, this, _1, id, prom), &_tracker));
    }, &_tracker));
    return prom.future();
  };

  qi::Future<void> ObjectRegistrar::unregisterService(unsigned int idx)
  {
    qi::Future<void> future = _sdClient->unregisterService(idx);

    std::string name;
    {
      // Create a local variable to keep the underlying bound anyobject alive outside the map.
      // It allows us to remove the iterator map without deleting the underlying anyobject
      BoundService serviceToRemove;
      boost::mutex::scoped_lock sl(_servicesMutex);
      BoundServiceMap::iterator it = _services.find(idx);
      if (it != _services.end())
      {
        name = it->second.name;
        if (!it->second.object.unique())
        {
          qiLogVerbose() << "Some references to service #" << idx
                                             << " are still held!";
        }
        serviceToRemove = std::move(it->second);
        _services.erase(it);
      }
      else
      {
        qiLogVerbose() << "Can't find name associated to id:" << idx;
      }
    }

    Server::removeObject(idx);

    if (!name.empty())
    {
      boost::mutex::scoped_lock sl(_serviceNameToIndexMutex);
      ServiceNameToIndexMap::iterator it = _serviceNameToIndex.find(name);
      if (it != _serviceNameToIndex.end())
        _serviceNameToIndex.erase(it);
      else
        qiLogVerbose() << "Can't find idx associated to name :" << name;
    }
    return future;
  }

  std::vector<qi::ServiceInfo> ObjectRegistrar::registeredServices() {
    std::vector<qi::ServiceInfo> ssi;
    {
      boost::mutex::scoped_lock sl(_servicesMutex);
      BoundServiceMap::iterator it = _services.begin();
      for (; it != _services.end(); ++it) {
        //drop 0 => it's the server itself.
        if (it->first == 0)
          continue;
        ssi.push_back(it->second.serviceInfo);
      }
    }
    return ssi;
  }

  //return 0 on error (0 is the server which have no name)
  unsigned int ObjectRegistrar::objectId(const std::string &name)
  {
    {
      boost::mutex::scoped_lock sl(_serviceNameToIndexMutex);
      ServiceNameToIndexMap::iterator it;
      it = _serviceNameToIndex.find(name);
      if (it != _serviceNameToIndex.end())
        return it->second;
    }
    return 0;
  }

  qi::ServiceInfo ObjectRegistrar::registeredService(const std::string &service) {
    unsigned int serviceId = objectId(service);

    if (!serviceId)
      return qi::ServiceInfo();

    {
      boost::mutex::scoped_lock sl(_servicesMutex);
      BoundServiceMap::iterator it = _services.find(serviceId);
      if (it != _services.end())
        return it->second.serviceInfo;
    }
    return qi::ServiceInfo();
  }

  qi::AnyObject ObjectRegistrar::registeredServiceObject(const std::string &service) {
    unsigned int serviceId = objectId(service);
    if (!serviceId)
      return qi::AnyObject();

    {
      boost::mutex::scoped_lock sl(_servicesMutex);
      BoundServiceMap::iterator it = _services.find(serviceId);
      if (it != _services.end())
        return it->second.object;
    }
    return AnyObject();
  }

  void ObjectRegistrar::open()
  {
    Server::open();
  }

}
